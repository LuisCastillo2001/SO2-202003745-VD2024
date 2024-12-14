#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>

#define SYS_LUIS_TRACK_SYSCALL_USAGE 549

struct track_counters {
    long counter_open;
    long counter_read;
    long counter_write;
    long counter_fork;
};

int main() {
    struct track_counters trackers;

 
    if (syscall(SYS_LUIS_TRACK_SYSCALL_USAGE, &trackers) == 0) {
        printf("Lllamadas a open: %ld\n", trackers.counter_open);
        printf("Llamadas a read: %ld\n", trackers.counter_read);
        printf("Lllamadas a write: %ld\n", trackers.counter_write);
        printf("Llamadas a fork: %ld\n", trackers.counter_fork);  // Falta el paréntesis de cierre
    } else {
        perror("luis_track_syscall_usage failed");
    }

    return 0;
}

