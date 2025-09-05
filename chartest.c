#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <cuda.h>
#include "ioctl_commands.h"


#define DEVICE_PATH "/dev/charmod0"
#define BUF_SIZE 128

int main() {
    int fd;

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
    int flag =1;
    CUresult status = cuPointerSetAttribute(&flag,CU_POINTER_ATTRIBUTE_SYNC_MEMOPS,device_array);
    if(status!=CUDA_SUCCESS) return -ENOTTY;

    fprintf(stdout, "device_array: %llu\n", device_array);

    long result = ioctl(fd, SET_ADDR, &device_array);
    result = ioctl(fd, SET_SIZE, &num_elements);
    if(result<0) printf("set size failed\n");

    printf("Got %lu entries\n", result);

    result= ioctl(fd, UNPIN_MEM, 0);

    // Close the device
    close(fd);
    return 0;
}
