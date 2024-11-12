#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int shmid;
    int *shared_memory;

    // Crear un segmento de memoria compartida
    shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    // Adjuntar el segmento de memoria compartida
    shared_memory = (int *)shmat(shmid, NULL, 0);
    if (shared_memory == (int *)-1) {
        perror("shmat");
        exit(1);
    }

    *shared_memory = 10; // Inicializar la memoria compartida

    pid_t pid = fork();

    if (pid == 0) {
        // Proceso hijo
        printf("Hijo (antes): valor = %d, dirección = %p\n", *shared_memory, (void *)shared_memory);
        
        // Cambiar la dirección a la que apunta el puntero en el hijo
        int *new_address = malloc(sizeof(int)); // Crear una nueva dirección
        *new_address = 30; // Inicializar el nuevo valor
        shared_memory = new_address; // Cambiar el puntero en el hijo

        printf("Hijo (después): valor = %d, dirección = %p\n", *shared_memory, (void *)shared_memory);
    } else {
        // Proceso padre
        wait(NULL); // Espera a que el hijo termine
        printf("Padre: valor = %d, dirección = %p\n", *shared_memory, (void *)shared_memory);
    }

    // Desadjuntar el segmento de memoria compartida
    shmdt(shared_memory);

    // Eliminar el segmento de memoria compartida
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
