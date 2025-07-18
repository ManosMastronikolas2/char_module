#include "charmod.h"

int majorNum = MAJORNUM;
int minorNum = MINORNUM;
struct mod_dev* devices;

struct file_operations fops = {
  .owner = THIS_MODULE,
  .open = mod_open,
  .release = mod_release,
  .read = mod_read,
  .write = mod_write

};

static int mod_init(void){

    int err;
    dev_t dev;

    if(majorNum){ //static major number creation
        dev = MKDEV(majorNum, minorNum);
        err = register_chrdev_region(dev, NR_DEVS, "char_mod");
    }else{ //dynamic major number creation
        alloc_chrdev_region(&dev, minorNum, NR_DEVS, "char_mod");
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

    for(int i=0;i<NR_DEVS;i++) {
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

    if(devices != NULL) {

        for(int i=0;i<NR_DEVS;i++){
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
    printk("Closing module!\n");
    return 0;

}

ssize_t mod_read(struct file* fp, char __user* buff, size_t count, loff_t* f_pos) {

    struct mod_dev* dev = fp->private_data;
    struct mod_qset* ptr;
    int quantum = dev->quantum, qset = dev->qset;
    int item_sz = quantum*qset;
    int node, set_pos, quantum_pos, node_pos;
    ssize_t ret=0;

    
    if(*f_pos >= dev->size) return ret; //if offset exceeds device size, abort read 
    if(*f_pos + count > dev->size) count = dev->size - *f_pos; //if size of data exceeds device size, read from f_pos to end of device

    node = *f_pos / item_sz; //which node (qset) of the list to access
    node_pos = *f_pos % item_sz; //position in that node
    set_pos = node_pos / quantum; //position inside the quantum set
    quantum_pos = node_pos % quantum; //offset inside quantum

    ptr = dev->data;

    if(!ptr) return -1; //if device has no data, return error
    for(int i=0;i<node && ptr!=NULL;i++) ptr = ptr->next; //follow the list to correct qset

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
    int node, set_pos, quantum_pos, node_pos;

    node = *f_pos / item_sz; //which node (qset) of the list to access
    node_pos = *f_pos % item_sz; //position in that node
    set_pos = node_pos / quantum; //position inside the quantum set
    quantum_pos = node_pos % quantum; //offset inside quantum
 

    if(dev->data==NULL){ //if device has no data, allocate first qset
        dev->data = kmalloc(sizeof(struct mod_qset), GFP_KERNEL);
        dev->data->next = NULL;
    }

    ptr = dev->data;
    for(int i=0;i<node;i++){

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




module_init(mod_init);
module_exit(mod_cleanup);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Manos Mastronikolas");
