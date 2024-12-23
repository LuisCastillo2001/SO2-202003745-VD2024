#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mm.h>
#include <linux/swap.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Luis Castillo");
MODULE_DESCRIPTION("Modulo para mostrar resumen de memoria de todos los procesos");
MODULE_VERSION("1.0");

#define PROC_NAME "luis_mem_info"

static unsigned long calculate_percentage(unsigned long part, unsigned long total) {
    return total ? (part * 10000 / total) : 0;
}

static int meminfo_show(struct seq_file *m, void *v) {
    struct sysinfo si;
    struct task_struct *task;

    si_meminfo(&si);
    unsigned long total_reserved_memory = 0; // KB
    unsigned long total_committed_memory = 0; // KB

    for_each_process(task) {
        struct mm_struct *mm = task->mm;

        if (mm) {
            total_reserved_memory += mm->total_vm << (PAGE_SHIFT - 10); 
            total_committed_memory += get_mm_rss(mm) << (PAGE_SHIFT - 10); 
        }
    }

    unsigned long reserved_mb = total_reserved_memory / 1024; 
    unsigned long committed_mb = total_committed_memory / 1024; 

    seq_printf(m, "\033[1;36m+-----------------------+-----------------------+\033[0m\n");
    seq_printf(m, "\033[1;36m| Reserved Memory (MB) | Committed Memory (MB) |\033[0m\n");
    seq_printf(m, "\033[1;36m+-----------------------+-----------------------+\033[0m\n");
    seq_printf(m, "| %21lu | %21lu |\n", reserved_mb, committed_mb);
    seq_printf(m, "+-----------------------+-----------------------+\n");

    return 0;
}

static int meminfo_open(struct inode *inode, struct file *file) {
    return single_open(file, meminfo_show, NULL);
}

static const struct proc_ops meminfo_ops = {
    .proc_open = meminfo_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int __init meminfo_init(void) {
    proc_create(PROC_NAME, 0444, NULL, &meminfo_ops);
    printk(KERN_INFO "Modulo %s cargado\n", PROC_NAME);
    return 0;
}

static void __exit meminfo_exit(void) {
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "Modulo %s descargado\n", PROC_NAME);
}

module_init(meminfo_init);
module_exit(meminfo_exit);
