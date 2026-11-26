#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define BUFFER_SIZE 1024

static dev_t dev_num;
static struct cdev my_cdev;
static struct class *my_class;

// 1 KB memory buffer in kernel space
static char buffer[BUFFER_SIZE];


// Read data from kernel memory
static ssize_t my_read(struct file *filp,
                       char __user *user_buf,
                       size_t count,
                       loff_t *pos)
{
    if (*pos >= BUFFER_SIZE)
        return 0;

    if (count > BUFFER_SIZE - *pos)
        count = BUFFER_SIZE - *pos;

    if (copy_to_user(user_buf, buffer + *pos, count))
        return -EFAULT;

    *pos += count;

    return count;
}


// Write data to kernel memory
static ssize_t my_write(struct file *filp,
                        const char __user *user_buf,
                        size_t count,
                        loff_t *pos)
{
    if (*pos >= BUFFER_SIZE)
        return -ENOSPC;

    if (count > BUFFER_SIZE - *pos)
        count = BUFFER_SIZE - *pos;

    if (copy_from_user(buffer + *pos, user_buf, count))
        return -EFAULT;

    *pos += count;

    return count;
}


// File operations
static const struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .read  = my_read,
    .write = my_write,
};


// Initialize driver
static int __init my_init(void)
{

    // 1. Create device number
    if (alloc_chrdev_region(&dev_num, 0, 1, "my_device") < 0)
    {
        pr_err("Failed to allocate device number\n");
    }
    pr_info("my_device: allocated with major %d and minor %d\n",
            MAJOR(dev_num), MINOR(dev_num));
    // 2. Register character device
    cdev_init(&my_cdev, &my_fops);
    if (cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        pr_err("Failed to add character device\n");
        goto err_num;
    }
    pr_info("my_device: registered with major %d and minor %d\n",
            MAJOR(dev_num), MINOR(dev_num));
    // 3. Create class and device
    my_class = class_create("my_class");
    if (my_class == NULL) {
        pr_err("Failed to create class\n");
        goto err_cdev;
    }

    pr_info("my_device: class created\n");

    if (device_create(my_class, NULL, dev_num,
                             NULL, "my_device") == NULL) {
        pr_err("Failed to create device\n");
        goto err_class;
    }

    pr_info("my_device: loaded\n");
    return 0;

err_class:
    class_destroy(my_class);
err_cdev:
    cdev_del(&my_cdev);
err_num:
    unregister_chrdev_region(dev_num, 1);
    return -1;
}


// Remove driver
static void __exit my_exit(void)
{
    device_destroy(my_class, dev_num);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);

    pr_info("my_device: unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");