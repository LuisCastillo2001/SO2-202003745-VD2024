#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <time.h>

#define SYS_luis_tamalloc 551 

int main() {
    printf("Program for tamalloc PID: %d\n", getpid());

    printf("Program to Allocate Memory using tamalloc. Press ENTER to continue...\n");
    getchar();

    size_t total_size = 10 * 1024 * 1024; // 10 MB
    unsigned long addr;

   
    long result = syscall(SYS_luis_tamalloc, total_size, &addr);
    if (result != 0) {
        perror("syscall luis_tamalloc failed");
        return 1;
    }

    char *buffer = (char *)addr;
    printf("Allocated 10MB of memory using tamalloc at address: %p\n", buffer);

    printf("Press ENTER to start reading memory byte by byte...\n");
    getchar();

    srand(time(NULL));

    
    for (size_t i = 0; i < total_size; i++) {
        char t = buffer[i]; // triggers lazy allocation (with zeroing :D )
        if (t != 0) {
            printf("ERROR FATAL: Memory at byte %zu was not initialized to 0\n", i);
            return 10;
        }

        
        char random_letter = 'A' + (rand() % 26);
        buffer[i] = random_letter;

        if (i % (1024 * 1024) == 0 && i > 0) { // Cada 1 MB
            printf("Checked %zu MB...\n", i / (1024 * 1024));
            sleep(1);
        }
    }

    printf("All memory verified to be zero-initialized. Press ENTER to exit.\n");
    getchar();
    return 0;
}
