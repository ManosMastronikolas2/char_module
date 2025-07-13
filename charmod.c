#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>

static int majorNum;

struct file_operations fops = {
        .open = *mod_open,
        .release = NULL,
        .read = NULL,
        .write = NULL

};

int mod_open(struct inode* inode, struct file *fp){
    printk("Opened module!\n");
    return 0;
}

int mod_release(){
  printk("Closing module!\n");
  return 0;

}

int mod_read(){

}

int mod_write(){
}





MODULE_LICENSE("GPL");
MODULE_AUTHOR("Manos Mastronikolas");
