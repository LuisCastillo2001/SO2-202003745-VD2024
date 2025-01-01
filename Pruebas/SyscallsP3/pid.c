#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    pid_t pids[3]; // Arreglo para almacenar los PIDs
    int i;

    printf("Creando 3 procesos...\n");

    // Crear 3 procesos
    for (i = 0; i < 3; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            // Error al crear el proceso
            perror("Error al crear proceso");
            exit(1);
        } else if (pids[i] == 0) {
            // Código del hijo
            printf("Proceso hijo creado con PID: %d\n", getpid());
            pause(); // El hijo espera indefinidamente
            exit(0);
        }
        // El código del padre continúa aquí
    }

    // Código del padre: Imprimir los PIDs de los hijos
    printf("\nPIDs de los procesos hijos creados:\n");
    for (i = 0; i < 3; i++) {
        printf("PID del proceso hijo %d: %d\n", i + 1, pids[i]);
    }

    // Esperar que se presione Enter para finalizar
    printf("\nPresiona Enter para finalizar...\n");
    getchar();

    return 0;
}

