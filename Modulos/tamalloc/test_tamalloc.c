#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/random.h>
#include <linux/slab.h>

extern long luis_tamalloc(size_t size, unsigned long *addr); // Declaración de la syscall personalizada

static int __init tamalloc_test_init(void)
{
    printk(KERN_INFO "Tamalloc Test Module Loaded\n");

    size_t total_size = 10 * 1024 * 1024; // 10 MB
    unsigned long addr = 0;
    long result;
    char *buffer;
    size_t i;

    // Invocar la syscall luis_tamalloc
    result = luis_tamalloc(total_size, &addr);
    if (result != 0) {
        printk(KERN_ERR "luis_tamalloc syscall failed, error code: %ld\n", result);
        return -EFAULT;
    }

    printk(KERN_INFO "Allocated 10MB of memory using tamalloc at address: 0x%lx\n", addr);

    // Acceder a la memoria asignada
    buffer = (char *)addr;

    // Verificar la memoria byte por byte
    for (i = 0; i < total_size; i++) {
        char t = buffer[i]; // triggers lazy allocation (must be zeroed)
        if (t != 0) {
            printk(KERN_ERR "ERROR: Memory at byte %zu was not initialized to 0\n", i);
            return -EFAULT;
        }

        // Escribir un carácter aleatorio de A-Z
        char random_letter = 'A' + (get_random_u32() % 26);
        buffer[i] = random_letter;

        if (i % (1024 * 1024) == 0 && i > 0) { // Cada 1 MB
            printk(KERN_INFO "Checked %zu MB...\n", i / (1024 * 1024));
        }
    }

    printk(KERN_INFO "All memory verified to be zero-initialized.\n");
    return 0;
}

static void __exit tamalloc_test_exit(void)
{
    printk(KERN_INFO "Tamalloc Test Module Unloaded\n");
}

module_init(tamalloc_test_init);
module_exit(tamalloc_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Luis Castillo");
MODULE_DESCRIPTION("Tamalloc Test Module");

