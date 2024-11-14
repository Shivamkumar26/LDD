#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h> 

/* Define device name and buffer size */
#define DEVICE_NAME "dev1"
#define BUFFER_SIZE 1024

/* Define global variables for device number, character device, device class, and buffer */
static dev_t dev_num;
static struct cdev my_cdev;
static struct class *my_class;
static char kernel_buffer[BUFFER_SIZE];
static int open_count = 0;

/* Function to handle opening of the device */
static int simple_open(struct inode *inode, struct file *file) {
    open_count++;
    printk(KERN_INFO "Device opened %d times\n", open_count);
    return 0;
}

/* Function to handle closing of the device */
static int simple_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device closed\n");
    return 0;
}

/* Function to handle reading from the device */
static ssize_t simple_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset) {
    size_t to_copy, not_copied;

    /* Calculate the amount of data to copy based on the buffer size and requested size */
    to_copy = min(size, BUFFER_SIZE - (size_t)(*offset));

    /* Copy data from kernel space to user space and update the offset */
    not_copied = copy_to_user(user_buffer, kernel_buffer + *offset, to_copy);
    *offset += to_copy - not_copied;

    printk(KERN_INFO "simple_char_dev: Read %zu bytes\n", to_copy - not_copied);
    printk(KERN_INFO "simple_char_dev: Read %zu bytes: %.*s\n", to_copy - not_copied, (int)(to_copy - not_copied), kernel_buffer + *offset - (to_copy - not_copied));
    /* Return the number of bytes actually read */
    return to_copy - not_copied; 
}

/* Function to handle writing to the device */
static ssize_t simple_write(struct file *file, const char __user *user_buffer, size_t size, loff_t *offset) {
    size_t to_copy, not_copied;

    /* Calculate the amount of data to copy based on the buffer size and requested size */
    to_copy = min(size, BUFFER_SIZE - (size_t)(*offset));

    /* Copy data from user space to kernel space and update the offset */
    not_copied = copy_from_user(kernel_buffer + *offset, user_buffer, to_copy);
    *offset += to_copy - not_copied;

    printk(KERN_INFO "simple_char_dev: Written %zu bytes\n", to_copy - not_copied);
    printk(KERN_INFO "simple_char_dev: Written %zu bytes: %.*s\n", to_copy - not_copied, (int)(to_copy - not_copied), kernel_buffer + *offset - (to_copy - not_copied));
    
    /* Return the number of bytes actually written */
    return to_copy - not_copied; 
}

/* File operations structure linking function pointers for open, read, write, and release operations */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = simple_open,
    .release = simple_release,
    .read = simple_read,
    .write = simple_write,
};

/* Module initialization function, called when the module is loaded */
static int __init simple_char_init(void) {
    int ret;

    /* Allocate a major and minor number dynamically for the device */
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ALERT "simple_char_dev: Failed to allocate a major number\n");
        return ret;
    }

    /* Initialize the character device and add it to the system */
    cdev_init(&my_cdev, &fops);
    ret = cdev_add(&my_cdev, dev_num, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ALERT "simple_char_dev: Failed to add cdev\n");
        return ret;
    }

    /* Create a device class for easier access to the device in /dev */
    my_class = class_create(THIS_MODULE, "simple_char_class");
    if (IS_ERR(my_class)) {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ALERT "simple_char_dev: Failed to create device class\n");
        return PTR_ERR(my_class);
    }

    /* Create a device node in /dev directory */
    if (IS_ERR(device_create(my_class, NULL, dev_num, NULL, DEVICE_NAME))) {
        class_destroy(my_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ALERT "simple_char_dev: Failed to create device\n");
        return -1;
    }

    printk(KERN_INFO "simple_char_dev: Device initialized with major %d, minor %d\n", MAJOR(dev_num), MINOR(dev_num));
    return 0;
}

/* Module cleanup function, called when the module is removed */
static void __exit simple_char_exit(void) {
    /* Remove device from /dev */
    device_destroy(my_class, dev_num);

    /* Destroy the device class */
    class_destroy(my_class);

    /* Remove the character device */
    cdev_del(&my_cdev);

    /* Unregister the device numbers */
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO "simple_char_dev: Device unregistered\n");
}

/* Register module initialization and cleanup functions */
module_init(simple_char_init);
module_exit(simple_char_exit);

/* Module metadata */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shivam Kumar");
MODULE_DESCRIPTION("A simple character device driver for Raspberry Pi 4");
MODULE_VERSION("1.0");
