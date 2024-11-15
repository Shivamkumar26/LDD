#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
 
#define DEVICE_NAME "dev2"
#define BUFFER_SIZE 1024
 
static dev_t dev_num;
static struct cdev my_cdev;
static struct class *my_class;
static char kernel_buffer[BUFFER_SIZE];
static int open_count = 0;
 
static int dev_open(struct inode *inode, struct file *file) {
    open_count++;
    
    int* ptr = NULL;
    *ptr = 10;
    
    printk(KERN_INFO "simple_char_dev: Device opened %d times\n", open_count);
    return 0;
}
 
static int dev_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "simple_char_dev: Device closed\n");
    return 0;
}
 
static ssize_t dev_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset) {
    size_t to_copy, not_copied;
    to_copy = min(size, BUFFER_SIZE - (size_t)(*offset));
    not_copied = copy_to_user(user_buffer, kernel_buffer + *offset, to_copy);
    *offset += to_copy - not_copied;
    printk(KERN_INFO "simple_char_dev: Read %zu bytes\n", to_copy - not_copied);
    printk(KERN_INFO "/n not copied %zu", not_copied); 
    return to_copy - not_copied;
}
 
static ssize_t dev_write(struct file *file, const char __user *user_buffer, size_t size, loff_t *offset) {
    size_t to_copy, not_copied;
    to_copy = min(size, BUFFER_SIZE - (size_t)(*offset));
    not_copied = copy_from_user(kernel_buffer + *offset, user_buffer, to_copy);
    *offset += to_copy - not_copied;
    printk(KERN_INFO "simple_char_dev: Written %zu bytes\n", to_copy - not_copied);
    return to_copy - not_copied;
}

loff_t driver_lseek(struct file *filp, loff_t offset, int whence)
{
    loff_t DEV_MEM_SIZE = 1024;
	loff_t temp;

	pr_info("lseek requested \n");
	pr_info("Current value of the file position = %lld\n",filp->f_pos);

	switch(whence)
	{
		case SEEK_SET:
			if((offset > DEV_MEM_SIZE) || (offset < 0))
				return -EINVAL;
			filp->f_pos = offset;
			break;
		case SEEK_CUR:
			temp = filp->f_pos + offset;
			if((temp > DEV_MEM_SIZE) || (temp < 0))
				return -EINVAL;
			filp->f_pos = temp;
			break;
		case SEEK_END:
			temp = DEV_MEM_SIZE + offset;
			if((temp > DEV_MEM_SIZE) || (temp < 0))
				return -EINVAL;
			filp->f_pos = temp;
			break;
		default:
			return -EINVAL;
	}
	
	pr_info("New value of the file position = %lld\n",filp->f_pos);
    return temp;
}
	
 
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
    .llseek = driver_lseek,
};
 
static int __init simple_char_init(void) {
    int ret;
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ALERT "simple_char_dev: Failed to allocate a major number\n");
        return ret;
    }
    cdev_init(&my_cdev, &fops);
    ret = cdev_add(&my_cdev, dev_num, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ALERT "simple_char_dev: Failed to add cdev\n");
        return ret;
    }
    my_class = class_create("class");
    if (IS_ERR(my_class)) {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ALERT "simple_char_dev: Failed to create class\n");
        return PTR_ERR(my_class);
    }
    device_create(my_class, NULL, dev_num, NULL, DEVICE_NAME);
    printk(KERN_INFO "simple_char_dev: Device initialized with major %d, minor %d\n", MAJOR(dev_num), MINOR(dev_num));
    return 0;
}
 
static void __exit simple_char_exit(void) {
    device_destroy(my_class, dev_num);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "simple_char_dev: Device unregistered\n");
}
 
module_init(simple_char_init);
module_exit(simple_char_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shivam Kumar");
MODULE_DESCRIPTION("A simple character device driver for Raspberry Pi 4");
MODULE_VERSION("1.0");
