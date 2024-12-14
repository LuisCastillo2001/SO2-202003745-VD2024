#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/ioctl.h>

#define __NR_luis_get_io_throttle 550
#define MAX_PROCS 20

// Definiciones actualizadas según las estructuras modificadas
struct io_stats {
    pid_t pid;                      // ID del proceso
    char comm[18];                  // Nombre del proceso
    unsigned long long bytes_read;  // Bytes leídos
    unsigned long long bytes_written; // Bytes escritos
    unsigned long long cancelled_write_bytes; // Bytes cancelados
    unsigned long long io_wait_time;          // Tiempo de espera (placeholder)
};

struct all_io_stats {
    struct io_stats stats[MAX_PROCS];
    int num_procs; // Número de procesos en la lista
};

int main() {
    struct all_io_stats user_stats;

    // Llamada a la syscall
    if (syscall(__NR_luis_get_io_throttle, &user_stats) == -1) {
        perror("syscall");
        return -1;
    }

    // Imprimir los datos obtenidos
    printf("Número de procesos: %d\n", user_stats.num_procs);
    for (int i = 0; i < user_stats.num_procs; i++) {
        printf("Proceso %d:\n", i + 1);
        printf("  PID: %d\n", user_stats.stats[i].pid);
        printf("  Nombre: %s\n", user_stats.stats[i].comm);
        printf("  Bytes leídos: %llu\n", user_stats.stats[i].bytes_read);
        printf("  Bytes escritos: %llu\n", user_stats.stats[i].bytes_written);
        printf("  Bytes cancelados: %llu\n", user_stats.stats[i].cancelled_write_bytes);
        printf("  Tiempo de espera de I/O: %llu\n", user_stats.stats[i].io_wait_time);
    }

    return 0;
}

