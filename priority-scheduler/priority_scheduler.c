#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define PROC_NAME "priority_sched"

static struct proc_dir_entry *proc_entry;

static ssize_t proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos) {
    char kbuf[128];
    if(count > sizeof(kbuf)-1)
        return -EINVAL;
    if(copy_from_user(kbuf, buffer, count))
        return -EFAULT;
    kbuf[count] = '\0';
    printk(KERN_INFO "priority_sched: Received input: %s\n", kbuf);
    return count;
}

static const struct proc_ops fops = {
    .proc_write = proc_write,
};

static int __init priority_init(void) {
    proc_entry = proc_create(PROC_NAME, 0666, NULL, &fops);
    if(!proc_entry) {
        printk(KERN_ERR "priority_sched: Failed to create /proc entry\n");
        return -ENOMEM;
    }
    printk(KERN_INFO "priority_sched: Module loaded\n");
    return 0;
}

static void __exit priority_exit(void) {
    proc_remove(proc_entry);
    printk(KERN_INFO "priority_sched: Module removed\n");
}

module_init(priority_init);
module_exit(priority_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple Priority Scheduler Kernel Module");

