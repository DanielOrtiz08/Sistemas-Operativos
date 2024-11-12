#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wait.h>

typedef struct {
    int x;
    int y;
    pid_t pid;
} Data;

typedef struct {
    int tam;       // Número de elementos agregados
    int lock;      // Variable de bloqueo (0 = libre, 1 = ocupado)
    Data nidos[];  // Array dinámico de estructuras Data
} SharedData;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Función para obtener el lock con espera activa
void acquire_lock(int *lock) {
    while (*lock == 1) {
        // Espera activa (busy-wait) hasta que lock esté libre (lock == 0)
    }
    *lock = 1; // Establece lock como ocupado
}

// Función para liberar el lock
void release_lock(int *lock) {
    *lock = 0; // Libera el lock
}

int main() {
    int maxNumMinas = 10;  // Número máximo de elementos en el array nidos

    // Crear el segmento de memoria compartida
    int shmIdRes = shmget(IPC_PRIVATE, sizeof(SharedData) + maxNumMinas * sizeof(Data), IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmIdRes == -1) {
        error("Error al crear el segmento de memoria compartida");
    }

    // Acoplar el segmento de memoria
    SharedData *sharedData = (SharedData*) shmat(shmIdRes, NULL, 0);
    if (sharedData == (void*)-1) {
        error("Error al acoplar el segmento de memoria compartida");
    }

    // Inicializar tam y lock en la memoria compartida
    sharedData->tam = 0;
    sharedData->lock = 0;

    // Crear varios procesos hijos
    for (int i = 0; i < maxNumMinas; i++) {
        if (fork() == 0) {  // Proceso hijo
            // Espera activa hasta poder adquirir el lock
            acquire_lock(&sharedData->lock);

            // Sección crítica: modificar tam y agregar Data
            int idx = sharedData->tam;
            sharedData->nidos[idx].x = i;
            sharedData->nidos[idx].y = i * 10;
            sharedData->nidos[idx].pid = getpid();
            sharedData->tam += 1;

            // Liberar el lock
            release_lock(&sharedData->lock);

            // Salir del proceso hijo
            exit(0);
        }
    }

    // Esperar a que todos los procesos hijos terminen
    for (int i = 0; i < maxNumMinas; i++) {
        wait(NULL);
    }

    // Imprimir el contenido de nidos y tam
    printf("Número total de elementos agregados: %d\n", sharedData->tam);
    for (int i = 0; i < sharedData->tam; i++) {
        printf("nidos[%d]: x = %d, y = %d, pid = %d\n", i, sharedData->nidos[i].x, sharedData->nidos[i].y, sharedData->nidos[i].pid);
    }

    // Desvincular el segmento de memoria compartida
    shmdt(sharedData);

    // Eliminar el segmento de memoria compartida
    shmctl(shmIdRes, IPC_RMID, NULL);

    return 0;
}
