#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/string.h>

#define PROC_NAME "safe_lkm"
#define MSG_SIZE 256
#define HIGH_PRIO_THRESHOLD 5

// -------------------------------
// IPC Structures
// -------------------------------

struct demo_msg {
    int pid;
    int type;
    char text[MSG_SIZE];
    struct list_head list;
};

struct demo_msg_queue {
    struct list_head high;
    struct list_head normal;
    int count;
};

static struct demo_msg_queue msg_queue;
static spinlock_t msg_lock;

// -------------------------------
// IPC Functions
// -------------------------------

// Send message with priority
int demo_send_msg(int pid, int type, const char *text)
{
    struct demo_msg *m;
    unsigned long flags;

    if (!text || pid <= 0) {
        printk(KERN_WARNING "[safe_lkm] Invalid PID or text\n");
        return -EINVAL;
    }

    m = kmalloc(sizeof(*m), GFP_KERNEL);
    if (!m) {
        printk(KERN_WARNING "[safe_lkm] Failed to allocate message\n");
        return -ENOMEM;
    }

    m->pid = pid;
    m->type = type;
    strncpy(m->text, text, MSG_SIZE - 1);
    m->text[MSG_SIZE - 1] = '\0';
    INIT_LIST_HEAD(&m->list);

    spin_lock_irqsave(&msg_lock, flags);
    if (type >= HIGH_PRIO_THRESHOLD) {
        list_add_tail(&m->list, &msg_queue.high);
        printk(KERN_INFO "[safe_lkm] High-priority msg from PID %d: %s\n", pid, text);
    } else {
        list_add_tail(&m->list, &msg_queue.normal);
        printk(KERN_INFO "[safe_lkm] Normal-priority msg from PID %d: %s\n", pid, text);
    }
    msg_queue.count++;
    spin_unlock_irqrestore(&msg_lock, flags);

    return 0;
}

// Receive message (high-priority first)
int demo_receive_msg(struct demo_msg *out)
{
    struct demo_msg *m = NULL;
    unsigned long flags;
    int ret = -ENOMSG;

    if (!out) return -EINVAL;

    spin_lock_irqsave(&msg_lock, flags);
    if (!list_empty(&msg_queue.high)) {
        m = list_first_entry(&msg_queue.high, struct demo_msg, list);
        list_del(&m->list);
        printk(KERN_INFO "[safe_lkm] Received high-priority message: %s\n", m->text);
    } else if (!list_empty(&msg_queue.normal)) {
        m = list_first_entry(&msg_queue.normal, struct demo_msg, list);
        list_del(&m->list);
        printk(KERN_INFO "[safe_lkm] Received normal-priority message: %s\n", m->text);
    }

    if (m) {
        memcpy(out, m, sizeof(*out));
        kfree(m);
        msg_queue.count--;
        ret = 0;
    } else {
        printk(KERN_INFO "[safe_lkm] No messages available\n");
    }

    spin_unlock_irqrestore(&msg_lock, flags);
    return ret;
}

// -------------------------------
// Cleanup
// -------------------------------

static void cleanup_messages(void)
{
    struct demo_msg *m, *tmp;
    unsigned long flags;
    int count = 0;

    spin_lock_irqsave(&msg_lock, flags);
    list_for_each_entry_safe(m, tmp, &msg_queue.high, list) {
        list_del(&m->list);
        kfree(m);
        count++;
    }
    list_for_each_entry_safe(m, tmp, &msg_queue.normal, list) {
        list_del(&m->list);
        kfree(m);
        count++;
    }
    msg_queue.count = 0;
    spin_unlock_irqrestore(&msg_lock, flags);

    if (count > 0) {
        printk(KERN_INFO "[safe_lkm] Cleaned up %d messages\n", count);
    }
}

// -------------------------------
// Proc Interface for IPC Testing
// -------------------------------

static ssize_t proc_read(struct file *file, char __user *buf,
                         size_t count, loff_t *ppos)
{
    char kbuff[512];
    int len;

    if (*ppos > 0) return 0;

    len = snprintf(kbuff, sizeof(kbuff),
                   "=== Safe LKM IPC Demo ===\n"
                   "Messages in queue: %d\n"
                   "Commands:\n"
                   "  B <pid> <type> <msg> - Send message\n"
                   "  R                     - Receive message\n"
                   "Examples:\n"
                   "  echo \"B 1001 6 Hello\" > /proc/safe_lkm\n"
                   "  echo \"R\" > /proc/safe_lkm\n\n",
                   msg_queue.count);

    if (copy_to_user(buf, kbuff, len)) return -EFAULT;
    *ppos = len;
    return len;
}

static ssize_t proc_write(struct file *file, const char __user *buf,
                          size_t count, loff_t *ppos)
{
    char kbuf[128];
    int pid, type;
    char text[64];
    struct demo_msg received_msg;

    if (count > sizeof(kbuf)-1) return -EINVAL;
    if (copy_from_user(kbuf, buf, count)) return -EFAULT;
    kbuf[count] = '\0';

    if (sscanf(kbuf, "B %d %d %63[^\n]", &pid, &type, text) == 3) {
        demo_send_msg(pid, type, text);
    } else if (strncmp(kbuf, "R", 1) == 0) {
        if (demo_receive_msg(&received_msg) == 0) {
            printk(KERN_INFO "[safe_lkm] User received: PID=%d, Type=%d, Text=%s\n",
                   received_msg.pid, received_msg.type, received_msg.text);
        }
    } else {
        printk(KERN_WARNING "[safe_lkm] Unknown command: %s\n", kbuf);
    }

    return count;
}

static const struct proc_ops proc_fops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

// -------------------------------
// Module Init/Exit
// -------------------------------

static int __init safe_lkm_init(void)
{
    spin_lock_init(&msg_lock);
    INIT_LIST_HEAD(&msg_queue.high);
    INIT_LIST_HEAD(&msg_queue.normal);
    msg_queue.count = 0;

    if (!proc_create(PROC_NAME, 0666, NULL, &proc_fops)) {
        printk(KERN_ERR "[safe_lkm] Failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }

    printk(KERN_INFO "[safe_lkm] IPC Module loaded\n"
           "[safe_lkm] Use: echo '<command>' > /proc/%s\n", PROC_NAME);
    return 0;
}

static void __exit safe_lkm_exit(void)
{
    cleanup_messages();
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "[safe_lkm] IPC Module unloaded\n");
}

module_init(safe_lkm_init);
module_exit(safe_lkm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OS Fall 2025 - IPC Demo");
MODULE_DESCRIPTION("Safe kernel module demo for Advanced IPC (Priority Message Queue)");
MODULE_VERSION("1.0");
