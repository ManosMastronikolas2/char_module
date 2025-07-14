#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>

#define QUANTUM_SZ 4000
#define QUANTUM_SET_SZ 1000
#define NR_DEVS 1

int mod_open(struct inode* inode, struct file *fp);
int mod_release(struct inode *inode, struct file* fp);
ssize_t mod_read(struct file* fp, char __user *buff, size_t count, loff_t *f_pos);
ssize_t mod_write(struct file* fp, char __user *buff, size_t count, loff_t *f_pos);
int mod_init(struct cdev* cdev, struct file_operations* fops);

int majorNum = 0;
int minorNum = 0;

/*Representation of one quantum set of data*/
struct mod_qset {
    void** data;
    struct mod_qset* next;
};

/*Representation of one module device*/
struct mod_dev{
    size_t quantum; /*Quantum block size*/
    size_t qset; /*Maximum number of quantum blocks of device*/
    size_t size; /*Total size of data on device*/
    struct mod_qset* data; /*Pointer to first quantum set*/
    struct cdev chardev; /*Character device structure*/
};

struct file_operations fops = {
  .owner = THIS_MODULE,
  .open = mod_open,
  .release = mod_release,
  .read = mod_read,
  .write = mod_write

};

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
    int item, set_pos, quantum_pos, rest;
    ssize_t ret=0;

    item = *f_pos / item_sz;
    rest = *f_pos % item_sz;
}





MODULE_LICENSE("GPL");
MODULE_AUTHOR("Manos Mastronikolas");
