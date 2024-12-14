#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>

#define SYS_LUIS_CAPTURE_MEMORY_SNAPSHOT 548


struct memory_snapshot {
    unsigned long total_mem_kb;
    unsigned long free_mem_kb;
    unsigned long active_page_count;
    unsigned long inactive_page_count;
    unsigned long total_swap_kb;
    unsigned long slab_mem_kb;
    unsigned long reclaimable_slab_mem_kb;
    char snapshot_date[36];
};

struct track_counters {
    long counter_open;
    long counter_read;
    long counter_write;
};

int main() {
    struct memory_snapshot snapshot = {0}; // Inicializa la estructura
    struct track_counters trackers = {0}; // Inicializa la estructura

    // Llamar a la syscall luis_capture_memory_snapshot
    if (syscall(SYS_LUIS_CAPTURE_MEMORY_SNAPSHOT, &snapshot) == 0) {
        printf("Fecha del Snapshot: %s\n", snapshot.snapshot_date);
        printf("Memoria Total: %lu KB\n", snapshot.total_mem_kb);
        printf("Memoria Libre: %lu KB\n", snapshot.free_mem_kb);
        printf("Páginas Activas: %lu\n", snapshot.active_page_count);
        printf("Páginas Inactivas: %lu\n", snapshot.inactive_page_count);
        printf("Swap Total: %lu KB\n", snapshot.total_swap_kb);
        printf("Memoria Slab (No Reclamable): %lu KB\n", snapshot.slab_mem_kb);
        printf("Memoria Slab Reclamable: %lu KB\n", snapshot.reclaimable_slab_mem_kb);
    } else {
        perror("Error al llamar a luis_capture_memory_snapshot");
    }

    return 0;
}

