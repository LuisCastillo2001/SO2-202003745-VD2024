#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MB (1024 * 1024)

void allocate_memory(size_t size, int step) {
    printf("\nPaso %d: Intentando asignar %zu MB de memoria...\n", step, size / MB);
    printf("Presiona ENTER para continuar...");
    getchar();

    char *buffer = malloc(size);
    if (buffer == NULL) {
        printf("Error: No se pudo asignar %zu MB de memoria.\n", size / MB);
        return;
    }

    printf("Exito: Se asignaron %zu MB de memoria en %p.\n", size / MB, buffer);

    printf("Llenando memoria con datos aleatorios...\n");
    for (size_t i = 0; i < size; i++) {
        buffer[i] = 'A' + (i % 26);
    }

    if (step == 4) {
        printf("Presiona ENTER para liberar esta memoria...\n");
        getchar();
        free(buffer);
        printf("Memoria de %zu MB liberada.\n", size / MB);
    }
}

int main() {
    printf("Programa de pruebas de asignación de memoria PID: %d\n", getpid());

    // Paso 1: Pedir 20 MB -> ESPERADO: FALLAR
    allocate_memory(20 * MB, 1);

    // Paso 2: Pedir 20 MB -> ESPERADO: OK
    allocate_memory(20 * MB, 2);

    // Paso 3: Pedir otros 20 MB -> ESPERADO: FALLAR
    allocate_memory(20 * MB, 3);

    // Paso 4: Pedir 35 MB -> ESPERADO: OK
    allocate_memory(35 * MB, 4);

    printf("\nLiberando 55 MB de memoria simulada. Presiona ENTER...\n");
    getchar();
    printf("Memoria liberada.\n");

    // Paso 5: Pedir 90 MB -> ESPERADO: OK
    allocate_memory(90 * MB, 5);

    // Paso 6: Pedir 30 MB -> ESPERADO: FALLAR
    allocate_memory(30 * MB, 6);

    printf("\nPrueba finalizada. Presiona ENTER para salir...\n");
    getchar();

    return 0;
}

