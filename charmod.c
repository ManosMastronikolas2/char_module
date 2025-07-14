#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>

int mod_open(struct inode* inode, struct file *fp);
int mod_release(struct inode *inode, struct file* fp);
ssize_t mod_read(struct file* fp, char __user *buff, size_t count, loff_t *offp);
ssize_t mod_write(struct file* fp, char __user *buff, size_t count, loff_t *offp);
int mod_init(struct cdev* cdev, struct file_operations* fops);

int majorNum = 0;

struct mod_dev{

    
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
    printk("Opened module!\n");
    return 0;
}

int mod_release(struct inode *inode, struct file* fp){
  printk("Closing module!\n");
  return 0;

}






MODULE_LICENSE("GPL");
MODULE_AUTHOR("Manos Mastronikolas");
