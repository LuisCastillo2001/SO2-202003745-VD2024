#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Luis Castillo");
MODULE_DESCRIPTION("Modulo para mostrar estadisticas de memoria de procesos");
MODULE_VERSION("1.0");

#define PROC_NAME "luis_mem_process"

static int target_pid = 0;

static unsigned long calculate_percentage(unsigned long part, unsigned long total) {
    return total ? (part * 10000 / total) : 0;
}

static int meminfo_show(struct seq_file *m, void *v) {
    struct sysinfo si;
    struct task_struct *task;

    si_meminfo(&si);
    unsigned long total_reserved = si.totalram * 4; 

    seq_printf(m, "Total Reserved Memory (KB): %lu\n", total_reserved);
    seq_printf(m, "+-------+-----------------+---------------------+-------------------+--------------+\n");
    seq_printf(m, "|  PID |       Name      | Reserved Memory(KB) | Committed Memory(KB) | Mem Usage (%) |\n");
    seq_printf(m, "+-------+-----------------+---------------------+-------------------+--------------+\n");

    for_each_process(task) {
        if (target_pid != 0 && task->pid != target_pid) {
            continue;
        }

        unsigned long vsz = 0;
        unsigned long rss = 0;
        unsigned long mem_usage = 0;
        long oom_score_adj = 0;

        if (task->mm) {
            vsz = task->mm->total_vm << (PAGE_SHIFT - 10);
            rss = get_mm_rss(task->mm) << (PAGE_SHIFT - 10);
            mem_usage = calculate_percentage(rss, vsz);
        }

        oom_score_adj = task->signal->oom_score_adj;

        seq_printf(m, "| %5d | %-15s | %19lu | %19lu | %12lu.%02lu |\n",
                   task->pid, task->comm, vsz, rss, mem_usage / 100, mem_usage % 100);
    }

    seq_printf(m, "+-------+-----------------+---------------------+-------------------+--------------+\n");

    return 0;
}

static int meminfo_open(struct inode *inode, struct file *file) {
    return single_open(file, meminfo_show, NULL);
}

static ssize_t meminfo_write(struct file *file, const char __user *buf, size_t count, loff_t *pos) {
    char input[10];

    if (count > 9) {
        return -EINVAL;
    }

    if (copy_from_user(input, buf, count)) {
        return -EFAULT;
    }

    input[count] = '\0';

    if (input[0] == '0' && input[1] == '\0') {
        target_pid = 0;
    } else {
        sscanf(input, "%d", &target_pid);
    }

    return count;
}

static const struct proc_ops meminfo_ops = {
    .proc_open = meminfo_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = meminfo_write,
};

static int __init meminfo_init(void) {
    proc_create(PROC_NAME, 0666, NULL, &meminfo_ops);
    printk(KERN_INFO "Modulo %s cargado\n", PROC_NAME);
    return 0;
}

static void __exit meminfo_exit(void) {
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "Modulo %s descargado\n", PROC_NAME);
}

module_init(meminfo_init);
module_exit(meminfo_exit);

