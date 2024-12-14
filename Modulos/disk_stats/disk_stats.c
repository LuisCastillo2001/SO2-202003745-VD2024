#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/statfs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define PROC_FILENAME "disk_stats" 
#define AUTHOR "Luis Antonio Castillo Javier"
#define DESCRIPTION "Módulo para monitorear estadísticas de disco"
#define MODULE_NAME "disk_stats"

static struct proc_dir_entry *proc_entry;


static int proc_show(struct seq_file *m, void *v)
{
    struct path path;
    struct kstatfs stat;
    int ret;
    const char *mount_point = "/";

 
    ret = kern_path(mount_point, LOOKUP_FOLLOW, &path);
    if (ret) {
        seq_printf(m, "Error al obtener el path de %s: %d\n", mount_point, ret);
        return ret;
    }

   
    ret = vfs_statfs(&path, &stat);
    if (ret) {
        seq_printf(m, "Error al obtener estadísticas de %s: %d\n", mount_point, ret);
        return ret;
    }

 
    unsigned long long free_space = stat.f_bsize * stat.f_bfree;
    unsigned long long total_space = stat.f_bsize * stat.f_blocks;


    seq_printf(m, "Punto de montaje: %s\n", mount_point);
    seq_printf(m, "Espacio total: %llu bytes\n", total_space);
    seq_printf(m, "Espacio libre: %llu bytes\n", free_space);
    return 0;
}


static int proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_show, NULL);
}


static const struct proc_ops proc_fops = {
    .proc_open = proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};


static int __init disk_stats_init(void)
{

    proc_entry = proc_create(PROC_FILENAME, 0, NULL, &proc_fops);
    if (!proc_entry) {
        pr_err("[%s] No se pudo crear /proc/%s\n", MODULE_NAME, PROC_FILENAME);
        return -ENOMEM;
    }

    pr_info("[%s] Módulo cargado correctamente.\n", MODULE_NAME);
    return 0;
}


static void __exit disk_stats_exit(void)
{
  
    proc_remove(proc_entry);
    pr_info("[%s] Módulo descargado correctamente.\n", MODULE_NAME);
}

module_init(disk_stats_init);
module_exit(disk_stats_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_VERSION("1.0");

