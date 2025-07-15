#include "charmod.h"

int majorNum = 0;
int minorNum = 0;


int mod_init(struct cdev* cdev, struct file_operations* fops){



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

    if(down_interruptible(&dev->sem)) return -ERESTARTSYS; //used for MUTEX reasons, to wait for semaphore acquiration while enabling interrupts
    if(*f_pos >= dev->size) { //if offset exceeds device size, abort read
        up(&dev->sem);
        return retval;
    }
    if(*f_pos + count > dev->size) count = dev->size - *f_pos; //if size of data exceeds device size, read from f_pos to end of device

    node = *f_pos / item_sz; //which node (mod_dev) of the list to access
    node_pos = *f_pos % item_sz; //position in that node
    set_pos = node_pos / quantum; //position inside the quantum set
    quantum_pos = node_pos % quantum; //offset inside quantum

    ptr = dev->data;

    if(!ptr) return -1; //if device has no data, return error

}




module_init(mod_init);
module_exit(mod_cleanup);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Manos Mastronikolas");
