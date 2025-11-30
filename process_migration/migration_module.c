#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>    // for kstrdup
#include <linux/string.h>  // for strsep

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rameen Arshad");
MODULE_DESCRIPTION("Process Migration Simulation Module (Multiple PIDs Support, Handles Invalids)");

// Accept comma-separated list of PIDs
static char *target_pids = "";
module_param(target_pids, charp, 0644);
MODULE_PARM_DESC(target_pids, "Comma-separated list of PIDs to migrate");

static int __init migration_init(void)
{
    char *pid_str, *cur, *p;
    pid_t pid;
    struct task_struct *task;

    if (strlen(target_pids) == 0) {
        pr_info("migration_module: No PIDs provided.\n");
        return 0;
    }

    // Duplicate the string to safely modify it
    cur = kstrdup(target_pids, GFP_KERNEL);
    if (!cur)
        return -ENOMEM;

    p = cur;
    while ((pid_str = strsep(&p, ",")) != NULL) {
        if (kstrtoint(pid_str, 10, &pid) == 0) {
            int found = 0;
            // Find the process with this PID
            for_each_process(task) {
                if (task->pid == pid) {
                    pr_info("migration_module: Found process %d (%s)\n", task->pid, task->comm);
                    pr_info("migration_module: Simulating migration...\n");
                    pr_info("migration_module: Migration complete.\n");
                    found = 1;
                    break;
                }
            }
            if (!found) {
                pr_info("migration_module: No process found with PID %d\n", pid);
            }
        } else {
            pr_info("migration_module: Invalid PID: %s\n", pid_str);
        }
    }

    kfree(cur);
    return 0;
}

static void __exit migration_exit(void)
{
    pr_info("migration_module: Module unloaded.\n");
}

module_init(migration_init);
module_exit(migration_exit);

