#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int __init nullspy_start(void)
{
    printk(KERN_INFO "Hello world\n");
    return 0;
}

static void __exit nullspy_end(void)
{
    printk(KERN_INFO "Unloading\n");
}

module_init(nullspy_start);
module_exit(nullspy_end);

MODULE_AUTHOR("Eric Wustrow");
MODULE_DESCRIPTION("Emails things written to /dev/null");
MODULE_LICENSE("GPL");
