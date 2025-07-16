#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

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

    // Write to the device
    ssize_t bytes_written = write(fd, write_buf, strlen(write_buf));
    if (bytes_written < 0) {
        perror("Write failed");
        close(fd);
        return 1;
    }
    printf("Wrote %zd bytes to device.\n", bytes_written);

    // Reset file position to start (optional, depending on your driver)
    lseek(fd, 0, SEEK_SET);

    // Read back from the device
    ssize_t bytes_read = read(fd, read_buf, BUF_SIZE - 1);
    if (bytes_read < 0) {
        perror("Read failed");
        close(fd);
        return 1;
    }

    read_buf[bytes_read] = '\0'; // Ensure null termination
    printf("Read from device: %s\n", read_buf);

    // Optionally: perform an ioctl if implemented
    // int result = ioctl(fd, MY_IOCTL_CMD, &some_struct);
    // printf("ioctl result = %d\n", result);

    // Close the device
    close(fd);
    return 0;
}
