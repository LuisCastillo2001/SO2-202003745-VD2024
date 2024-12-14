#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/statfs.h>

#define AUTHOR "Luis Antonio Castillo Javier"
#define DESCRIPTION "Módulo para monitorear estadísticas de memoria"
#define MODULE_NAME "mem_stats"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_VERSION("1.0");

static int show_mem_stats(struct seq_file *m, void *v) {
    struct sysinfo si;

    si_meminfo(&si);

    seq_printf(m, "Memoria Total: %lu KB\n", si.totalram * si.mem_unit / 1024);
    seq_printf(m, "Memoria Libre: %lu KB\n", si.freeram * si.mem_unit / 1024);
    seq_printf(m, "Memoria en Buffers: %lu KB\n", si.bufferram * si.mem_unit / 1024);
    seq_printf(m, "Swap Total: %lu KB\n", si.totalswap * si.mem_unit / 1024);
    seq_printf(m, "Swap Libre: %lu KB\n", si.freeswap * si.mem_unit / 1024);

    return 0;
}

static int open_mem_stats(struct inode *inode, struct file *file) {
    return single_open(file, show_mem_stats, NULL);
}

static const struct proc_ops mem_stats_ops = {
    .proc_open = open_mem_stats,
    .proc_read = seq_read,
    .proc_release = single_release,
};

static int __init mem_stats_init(void) {
    if (!proc_create("mem_stats", 0, NULL, &mem_stats_ops)) {
        pr_err("No se pudo crear /proc/mem_stats\n");
        return -ENOMEM;
    }

    pr_info("Módulo %s cargado correctamente\n", MODULE_NAME);
    return 0;
}

static void __exit mem_stats_exit(void) {
    remove_proc_entry("mem_stats", NULL);
    pr_info("Módulo %s desinstalado\n", MODULE_NAME);
}

module_init(mem_stats_init);
module_exit(mem_stats_exit);

