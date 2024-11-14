#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/* Path to the device file in /dev */
#define DEVICE_PATH "/dev/dev1" 
/* Buffer size for read/write operations */
#define BUFFER_SIZE 1024        

int main() {
    int fd;
    char write_data[BUFFER_SIZE] = "Hello, Character Device!";
    char read_data[BUFFER_SIZE];
    ssize_t bytes_written, bytes_read;

    /* Open the device file */
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return errno;
    }
    printf("Device opened successfully.\n");

    /* Write data to the device */
    bytes_written = write(fd, write_data, strlen(write_data));
    if (bytes_written < 0) {
        perror("Failed to write to the device");
        close(fd);
        return errno;
    }
    printf("Data written to the device: %s\n", write_data);

    /* Reset read_data buffer and file offset */
    memset(read_data, 0, BUFFER_SIZE);
    /* Reset offset to the beginning of the device */
    lseek(fd, 0, SEEK_SET);  

    /* Read data from the device */
    bytes_read = read(fd, read_data, BUFFER_SIZE);
    if (bytes_read < 0) {
        perror("Failed to read from the device");
        close(fd);
        return errno;
    }
    printf("Data read from the device: %s\n", read_data);

    /* Close the device file */
    close(fd);
    printf("Device closed successfully.\n");

    return 0;
}
