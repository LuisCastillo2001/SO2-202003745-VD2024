#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ALLOC_SIZE (50 * 1024 * 1024)

int main() {
    char *buffer;

    printf("Program for malloc PID: %d\n", getpid());

    printf("Press ENTER to allocate memory...\n");
    getchar();

    printf("Allocating memory...\n");

    buffer = malloc(ALLOC_SIZE);
    if (buffer == NULL) {
        perror("Failed to allocate memory");
        return 1;
    }

    printf("Successfully allocated 50 MB of memory at %p\n", buffer);

    printf("Press ENTER to fill with random data.\n");
    getchar();

    for (size_t i = 0; i < ALLOC_SIZE; i++) {
        char random_letter = 'A' + (i % 26);
        buffer[i] = random_letter;
    }


    printf("Press ENTER to exit...\n");
    getchar();

    free(buffer);
    printf("Memory freed. Exiting.\n");

    return 0;
}

