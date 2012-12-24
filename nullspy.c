#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

static ssize_t
nullspy_read(struct file *filp, char __user *buf,
             size_t len, loff_t *off)
{
    return 0;
}

static ssize_t
nullspy_write(struct file *filp, const char __user *buf,
              size_t len, loff_t *off)
{
    return len;
}

struct file_operations nullspy_fops = {
    .read = nullspy_read,
    .write = nullspy_write,
    .open = NULL,
    .release = NULL
};


static int nullspy_major;
#define NULLSPY_NAME "nullspy"


static int __init nullspy_start(void)
{

    nullspy_major = register_chrdev(0, NULLSPY_NAME, &nullspy_fops);
    if (nullspy_major < 0) {
        printk(KERN_ALERT "Registering char device failed with %d\n",
               nullspy_major);
        return nullspy_major;
    }
    printk(KERN_INFO "Registered nullspy major %d\n", nullspy_major);


    return 0;
}

static void __exit nullspy_end(void)
{
    unregister_chrdev(nullspy_major, NULLSPY_NAME);
    printk(KERN_INFO "Unloading\n");
}

module_init(nullspy_start);
module_exit(nullspy_end);

MODULE_AUTHOR("Eric Wustrow");
MODULE_DESCRIPTION("Emails things written to /dev/null");
MODULE_LICENSE("GPL");
