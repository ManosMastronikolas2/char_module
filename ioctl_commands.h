#include <linux/ioctl.h>

#ifndef IOCTL_COMMANDS_H
#define IOCTL_COMMANDS_H

#define CHARDEV_IOC_MAGIC 'M'

#define SET_ADDR _IOW(CHARDEV_IOC_MAGIC, 0, unsigned long)
#define SET_SIZE   _IOW(CHARDEV_IOC_MAGIC, 2, size_t)
#define UNPIN_MEM  _IO(CHARDEV_IOC_MAGIC, 3)

#endif