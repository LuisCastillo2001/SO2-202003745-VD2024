#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/statfs.h>

#define PROC_FILENAME "disk_stats" // Nombre del archivo en /proc
#define AUTHOR "Luis Antonio Castillo Javier"
#define DESCRIPTION "Módulo para monitorear estadísticas de disco"
#define MODULE_NAME "disk_stats"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_VERSION("1.0");

// Función para obtener información de almacenamiento
static int fetch_storage_info(const char *path, unsigned long *total, unsigned long *free) {
    struct kstatfs stats;
    struct file *file;
    int ret;

    file = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(file)) {
        pr_err("No se pudo abrir %s\n", path);
        return -1;
    }

    ret = vfs_statfs(&file->f_path, &stats);
    filp_close(file, NULL);

    if (ret == 0) {
        *total = (stats.f_blocks * stats.f_bsize) >> 10; // Total en KB
        *free = (stats.f_bfree * stats.f_bsize) >> 10;   // Libre en KB
    } else {
        pr_err("Error al obtener información de almacenamiento\n");
    }

    return ret;
}

// Función para mostrar la información del disco en /proc
static int disk_stats_display(struct seq_file *m, void *v) {
    unsigned long total_storage, free_storage;

    if (fetch_storage_info("/", &total_storage, &free_storage) == 0) {
        seq_printf(m, "=== Estadísticas de Disco ===\n");
        seq_printf(m, "Almacenamiento Total: %lu KB\n", total_storage);
        seq_printf(m, "Almacenamiento Libre: %lu KB\n", free_storage);
    } else {
        seq_printf(m, "Información de Almacenamiento: Error\n");
    }

    return 0;
}

static int disk_stats_open(struct inode *inode, struct file *file) {
    return single_open(file, disk_stats_display, NULL);
}

static const struct proc_ops disk_stats_ops = {
    .proc_open = disk_stats_open,
    .proc_read = seq_read,
    .proc_release = single_release,
};

// Inicialización del módulo
static int __init disk_stats_init(void) {
    if (!proc_create(PROC_FILENAME, 0, NULL, &disk_stats_ops)) {
        pr_err("No se pudo crear /proc/%s\n", PROC_FILENAME);
        return -ENOMEM;
    }

    pr_info("Módulo %s cargado correctamente\n", PROC_FILENAME);
    return 0;
}

// Limpieza del módulo
static void __exit disk_stats_exit(void) {
    remove_proc_entry(PROC_FILENAME, NULL);
    pr_info("Módulo %s desinstalado\n", PROC_FILENAME);
}

module_init(disk_stats_init);
module_exit(disk_stats_exit);
