#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/statfs.h>

#define AUTHOR "Luis Antonio Castillo Javier"
#define DESCRIPTION "Módulo para monitorear estadísticas de CPU"
#define MODULE_NAME "cpu_stats"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_VERSION("1.0");

static unsigned long long prev_idle_time = 0, prev_total_time = 0;

static unsigned long long get_cpu_idle_time(void) {
    struct file *file;
    char buffer[128] = {0};
    ssize_t bytes_read;
    unsigned long long idle, iowait;

    file = filp_open("/proc/stat", O_RDONLY, 0);
    if (IS_ERR(file)) {
        pr_err("No se pudo abrir /proc/stat\n");
        return -1;
    }

    bytes_read = kernel_read(file, buffer, sizeof(buffer) - 1, &file->f_pos);
    filp_close(file, NULL);

    if (bytes_read <= 0) {
        pr_err("No se pudo leer /proc/stat\n");
        return -1;
    }

    buffer[bytes_read] = '\0'; // Aseguramos terminación de la cadena

    if (sscanf(buffer, "cpu  %*llu %*llu %*llu %llu %llu %*llu %*llu %*llu",
               &idle, &iowait) != 2) {
        pr_err("Error al parsear /proc/stat\n");
        return -1;
    }

    return idle + iowait;
}

static unsigned long long get_cpu_total_time(void) {
    struct file *file;
    char buffer[128] = {0};
    ssize_t bytes_read;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;

    file = filp_open("/proc/stat", O_RDONLY, 0);
    if (IS_ERR(file)) {
        pr_err("No se pudo abrir /proc/stat\n");
        return -1;
    }

    bytes_read = kernel_read(file, buffer, sizeof(buffer) - 1, &file->f_pos);
    filp_close(file, NULL);

    if (bytes_read <= 0) {
        pr_err("No se pudo leer /proc/stat\n");
        return -1;
    }

    buffer[bytes_read] = '\0'; // Aseguramos terminación de la cadena

    if (sscanf(buffer, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) != 8) {
        pr_err("Error al parsear /proc/stat\n");
        return -1;
    }

    return user + nice + system + idle + iowait + irq + softirq + steal;
}

static int calculate_cpu_usage(void) {
    unsigned long long idle_time, total_time, delta_idle, delta_total;
    int cpu_usage = 0;

    idle_time = get_cpu_idle_time();
    total_time = get_cpu_total_time();

    if (idle_time == -1 || total_time == -1) {
        return -1;
    }

    delta_idle = idle_time - prev_idle_time;
    delta_total = total_time - prev_total_time;

    prev_idle_time = idle_time;
    prev_total_time = total_time;

    if (delta_total > 0) {
        cpu_usage = (100 * (delta_total - delta_idle)) / delta_total;
    }

    return cpu_usage;
}

static int show_cpu_stats(struct seq_file *m, void *v) {
    int cpu_usage = calculate_cpu_usage();
    if (cpu_usage >= 0) {
        seq_printf(m, "Uso de CPU: %d%%\n", cpu_usage);
    } else {
        seq_printf(m, "Uso de CPU: Error\n");
    }
    return 0;
}

static int open_cpu_stats(struct inode *inode, struct file *file) {
    return single_open(file, show_cpu_stats, NULL);
}

static const struct proc_ops cpu_stats_ops = {
    .proc_open = open_cpu_stats,
    .proc_read = seq_read,
    .proc_release = single_release,
};

static int __init cpu_stats_init(void) {
    if (!proc_create("cpu_stats", 0, NULL, &cpu_stats_ops)) {
        pr_err("No se pudo crear /proc/cpu_stats\n");
        return -ENOMEM;
    }

    pr_info("Módulo %s cargado correctamente\n", MODULE_NAME);
    return 0;
}

static void __exit cpu_stats_exit(void) {
    remove_proc_entry("cpu_stats", NULL);
    pr_info("Módulo %s desinstalado\n", MODULE_NAME);
}

module_init(cpu_stats_init);
module_exit(cpu_stats_exit);

