#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <asm/current.h>
#include <linux/file.h>
#include <linux/syscalls.h>
#include <linux/fdtable.h>


static int nullspy_major;
#define NULLSPY_NAME "nullspy"
static char *email = "ewust@umich.edu";
module_param(email, charp, S_IRUGO);

static ssize_t
nullspy_read(struct file *filp, char __user *buf,
             size_t len, loff_t *off)
{
    return 0;
}


/*
int call_usermodehelper_stdinpipe(struct subprocess_info *sub_info,
                  struct file **filp)
{
    struct file *f;

    f = create_write_pipe(0);
    if (IS_ERR(f))
        return PTR_ERR(f);
    *filp = f;

    f = create_read_pipe(f, 0);
    if (IS_ERR(f)) {
        free_write_pipe(*filp);
        return PTR_ERR(f);
    }
    sub_info->stdin = f;

    return 0;
}
*/

int
nullspy_init(struct subprocess_info *info, struct cred *new)
{
    // Setup stdin to be our filp (this maybe a bad idea...)
    struct files_struct *f = current->files;
    struct fdtable *fdt;
    /* no races because files should be private here */
    printk(KERN_INFO "In nullspy_init, brace for panic");

    sys_close(0);
    fd_install(0, (struct file*)info->data);

    spin_lock(&f->file_lock);
    fdt = files_fdtable(f);
    FD_SET(0, fdt->open_fds);
    FD_CLR(0, fdt->close_on_exec);
    spin_unlock(&f->file_lock);

    /* and disallow core files too */
    current->signal->rlim[RLIMIT_CORE] = (struct rlimit){0, 0};

    return 0;
}

static ssize_t
nullspy_write(struct file *filp, const char __user *buf,
              size_t len, loff_t *off)
{
    char *argv[] = {"/usr/bin/mail", "-s", "Test Subject", email, NULL };
    char *envp[] = {"HOME=/root/", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };

    // Upcalls have changed since 2.6.35. We now have to create our own subproc,
    // and capture the init function to insert a pipe into the proc's stdin
    //info = call_usermodehelper_setup("/usr/bin/mail", argv, envp, GFP_KERNEL);
    call_usermodehelper_fns("/usr/bin/mail", argv, envp, UMH_WAIT_EXEC, nullspy_init, NULL, filp);
    
    return len;
}

struct file_operations nullspy_fops = {
    .read = nullspy_read,
    .write = nullspy_write,
    .open = NULL,
    .release = NULL
};

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
