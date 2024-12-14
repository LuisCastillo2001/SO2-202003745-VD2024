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
        printf("Snapshot Date: %s\n", snapshot.snapshot_date);
        printf("Total Memory: %lu KB\n", snapshot.total_mem_kb);
        printf("Free Memory: %lu KB\n", snapshot.free_mem_kb);
        printf("Active Pages: %lu\n", snapshot.active_page_count);
        printf("Inactive Pages: %lu\n", snapshot.inactive_page_count);
        printf("Total Swap: %lu KB\n", snapshot.total_swap_kb);
        printf("Slab Memory (Unreclaimable): %lu KB\n", snapshot.slab_mem_kb);
        printf("Reclaimable Slab Memory: %lu KB\n", snapshot.reclaimable_slab_mem_kb);
    } else {
        perror("luis_capture_memory_snapshot failed");
    }

    
    return 0;
}

