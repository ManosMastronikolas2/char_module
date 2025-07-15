#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>

#ifndef CHARMOD_H
#define CHARMOD_H

#define QUANTUM_SZ 4000
#define QUANTUM_SET_SZ 1000
#define NR_DEVS 1
#define MAJORNUM 0
#define MINORNUM 0

int mod_open(struct inode* inode, struct file *fp);
int mod_release(struct inode *inode, struct file* fp);
ssize_t mod_read(struct file* fp, char __user *buff, size_t count, loff_t *f_pos);
ssize_t mod_write(struct file* fp, char __user *buff, size_t count, loff_t *f_pos);
static int mod_init(void);
static int mod_cleanup(void);


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



#endif
