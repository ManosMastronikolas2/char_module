#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>

int mod_open(struct inode* inode, struct file *fp){
    printk("Opened module!\n");
    return 0;
}

struct file_operations fops = {
        .open = *mod_open,
        .release = NULL,
        .read = NULL,
        .write = NULL

};



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Manos Mastronikolas");
