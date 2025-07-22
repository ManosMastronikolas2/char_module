#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <cuda.h>


enum ioctl_commands {SET_ADDR, SET_SIZE};


#define DEVICE_PATH "/dev/charmod0"
#define BUF_SIZE 128

int main() {
    int fd;
    char write_buf[] = "Hello from user space!\n";
    char read_buf[BUF_SIZE] = {0};

    // Open the device
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open /dev/charmod");
        return 1;
    }

    CUdevice dev;
    CUcontext ctx;

    size_t num_elements = 1024*1024;
    CUdeviceptr device_array;

    cuInit(0);
    cuDeviceGet(&dev, 0);
    cuCtxCreate(&ctx, 0, dev);

    cuMemAlloc(&device_array, num_elements * sizeof(char));

    fprintf(stdout, "device_array: %llu\n", device_array);

    int result = ioctl(fd, SET_ADDR, device_array);
    result = ioctl(fd, SET_SIZE, num_elements);

    // Close the device
    close(fd);
    return 0;
}
