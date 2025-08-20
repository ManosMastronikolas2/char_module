#include "charmod.h"
#include "/usr/src/nvidia-565.57.01/nvidia/nv-p2p.h"

int majorNum = MAJORNUM;
int minorNum = MINORNUM;
struct mod_dev* devices;

struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = mod_open,
    .release = mod_release,
    .read = mod_read,
    .write = mod_write,
    .unlocked_ioctl = mod_ioctl

};

static struct nvidia_p2p_page_table* pg_table = NULL;

static int mod_init(void){

    int err,i;
    dev_t dev;

    if(majorNum){ //static major number creation
        dev = MKDEV(majorNum, minorNum);
        err = register_chrdev_region(dev, NR_DEVS, "charmod");
    }else{ //dynamic major number creation
        alloc_chrdev_region(&dev, minorNum, NR_DEVS, "charmod");
        majorNum = MAJOR(dev);
    }
    if(err<0) { //error
        printk("Can't get major\n");
        return -1;
    }

    printk("Device created with major number: %d\n", majorNum);

    devices = kmalloc(NR_DEVS * sizeof(struct mod_dev), GFP_KERNEL);
    if(devices==NULL) {
        printk("Could not create devices!\n");
        mod_cleanup();
        return -1;
    }

    for(i=0;i<NR_DEVS;i++) {
        devices[i].quantum = QUANTUM_SZ;
        devices[i].qset = QUANTUM_SET_SZ;

        dev = MKDEV(majorNum, minorNum+i);
        cdev_init(&(devices[i].chardev), &fops);
        devices[i].chardev.owner = THIS_MODULE;
        devices[i].chardev.ops = &fops;
        err = cdev_add(&(devices[i].chardev), dev, 1);
        if(err) {
            printk("Error registering devices!\n");
            return -1;
        }
    }
    return 0;
}

static void mod_cleanup(void){

    dev_t dev = MKDEV(majorNum, minorNum);
    int i;

    if(devices != NULL) {

        for(i=0;i<NR_DEVS;i++){
            cdev_del(&devices[i].chardev);
        }

        kfree(devices);
    }

    unregister_chrdev_region(dev, NR_DEVS);

   
}

int mod_open(struct inode* inode, struct file *fp){

    struct mod_dev* dev = container_of(inode->i_cdev, struct mod_dev, chardev);
    fp->private_data = dev;

    printk("Opened module!\n");
    return 0;
}

int mod_release(struct inode *inode, struct file* fp){
   struct mod_dev* dev = fp->private_data;
    printk("Closing module!\n");
     if(pg_table != NULL) {
        nvidia_p2p_put_pages(0,0,dev->user_addr,pg_table);
        printk("Released GPU pages!\n");
        pg_table = NULL;
    }
    return 0;

}

ssize_t mod_read(struct file* fp, char __user* buff, size_t count, loff_t* f_pos) {

    printk("Read!\n");
    struct mod_dev* dev = fp->private_data;
    struct mod_qset* ptr;
    int quantum = dev->quantum, qset = dev->qset;
    int item_sz = quantum*qset;
    int i,node, set_pos, quantum_pos, node_pos;
    ssize_t ret=0;

    
    if(*f_pos >= dev->size) return ret; //if offset exceeds device size, abort read 
    if(*f_pos + count > dev->size) count = dev->size - *f_pos; //if size of data exceeds device size, read from f_pos to end of device

    node = *f_pos / item_sz; //which node (qset) of the list to access
    node_pos = *f_pos % item_sz; //position in that node
    set_pos = node_pos / quantum; //position inside the quantum set
    quantum_pos = node_pos % quantum; //offset inside quantum

    ptr = dev->data;

    if(!ptr) return -1; //if device has no data, return error
    for(i=0;i<node && ptr!=NULL;i++) ptr = ptr->next; //follow the list to correct qset

    if(ptr==NULL || ptr->data==NULL || ptr->data[set_pos]==NULL){ //if qset is not found, or qset has no data, or quantum in qset has no data, abort
        printk("Data not found!\n");
        return 0;
    }

    if(count > quantum-quantum_pos) count = quantum-quantum_pos; //if data size to be read exceeds quantum, just read till the end of the quantum

    if(copy_to_user(buff, ptr->data[set_pos] + quantum_pos, count)){ //if read failed return -1
        printk("Data could not be read!\n");
        return -1;
    }

    *f_pos += count;

    return count;

}

ssize_t mod_write(struct file* fp, const char __user *buff, size_t count, loff_t *f_pos){

    printk("Write!\n");
    struct mod_dev* dev = fp->private_data;
    struct mod_qset* ptr;
    int quantum = dev->quantum, qset = dev->qset;
    int item_sz = quantum*qset;
    int i,node, set_pos, quantum_pos, node_pos;

    node = *f_pos / item_sz; //which node (qset) of the list to access
    node_pos = *f_pos % item_sz; //position in that node
    set_pos = node_pos / quantum; //position inside the quantum set
    quantum_pos = node_pos % quantum; //offset inside quantum
 

    if(dev->data==NULL){ //if device has no data, allocate first qset
        dev->data = kmalloc(sizeof(struct mod_qset), GFP_KERNEL);
        dev->data->next = NULL;
    }

    ptr = dev->data;
    for(i=0;i<node;i++){

        if(ptr->next==NULL) { 
            ptr->next = kmalloc(sizeof(struct mod_qset), GFP_KERNEL);
            if(ptr->next == NULL) return -ENOMEM;
            ptr->next->next = NULL;
        }
    }
    


    if(ptr==NULL){ //if qset is not found, abort
        printk("Set not found!\n");
        return 0;
    }

    if(ptr->data ==  NULL){ //if qset has no data, allocate data chunk
        ptr->data = kmalloc(qset*sizeof(char*), GFP_KERNEL);
        if(ptr->data==NULL) return -ENOMEM; //no memory could be allocated
    }

    if(ptr->data[set_pos]==NULL) { //if qset[set_pos] has no data, allocate a quantum
        ptr->data[set_pos] = kmalloc(quantum, GFP_KERNEL);
        if(ptr->data[set_pos]==NULL) return -ENOMEM; //no memory could be allocated
    }

    if(count > quantum-quantum_pos)  count = quantum-quantum_pos; //if data size to be written exceeds quantum, just write till the end of the quantum

    if(copy_from_user(ptr->data[set_pos] + quantum_pos, buff, count)){
        printk("Data could not be written!\n");
        return -EFAULT;
    }

    *f_pos += count;

    if(dev->size < *f_pos) dev->size = *f_pos; //update device size

    return count;

}

void free_callback(void* data) {

	nvidia_p2p_free_page_table(pg_table);
}


long mod_ioctl(struct file* fp, unsigned int cmd, unsigned long arg){

    //assume that user first gives address and then size, change in the future
    struct mod_dev* dev = fp->private_data;
    unsigned long addr=0;
    size_t size=0;
    int i;

    switch(cmd){

        case SET_ADDR:
            if(copy_from_user(&dev->user_addr, (unsigned long __user*)arg, sizeof(dev->user_addr))){
                                printk("1FAiled\n");

                return -EFAULT;
            }
            printk("size: %zu\n", dev->user_addr);
            break;
        case SET_SIZE:
            if(copy_from_user(&dev->user_size, (size_t __user*)arg, sizeof(dev->user_size))){
                printk("2FAiled\n");
                return -EFAULT;
            }
            printk("size: %zu\n", dev->user_size);

            if(dev->user_size==0 || dev->user_size==0) return -EFAULT;
            size = dev->user_size;
            addr = dev->user_addr;

            int ret = nvidia_p2p_get_pages(0,0,addr,size,&pg_table,free_callback,NULL);
            printk("Got %u GPU pages\n", pg_table->entries);

            if(ret || pg_table == NULL) return -EIO;

            printk("Got %u GPU pages\n", pg_table->entries);
            
            for(i=0;i<pg_table->entries;i++){
                /*print page physical addresses*/
                printk("addr %d: 0x%llx\n", i,pg_table->pages[i]->physical_address);
            }
            return (long)(pg_table->entries);
    	    break;
        default:
            return -ENOTTY;
            break;
    }

    return 0;
}


module_init(mod_init);
module_exit(mod_cleanup);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Manos Mastronikolas");
