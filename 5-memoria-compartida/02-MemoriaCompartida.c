
#include <stdio.h>    // Para funciones de entrada/salida como printf y fgets
#include <string.h>   // Para funciones de manejo de cadenas como strcpy y strcmp
#include <stdlib.h>   // Para EXIT_SUCCESS y EXIT_FAILURE
#include <unistd.h>   // Para fork(), getpid(), shmat(), shmdt()
#include <sys/wait.h> // Para wait()
#include <sys/shm.h>  // Para shmget(), shmctl(), shmat(), shmdt()
#include <sys/stat.h> // Para constantes de modo de acceso

int main() {
    char *ptr;
    int shm_id;
    int shm_size = 256; // Tamaño del segmento de memoria compartida

    // Crea un segmento de memoria compartida
    shm_id = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | S_IRUSR | S_IWUSR);

    // Crea un proceso hijo
    if (!fork()) {
        // Proceso hijo
        printf("[%d]child process started\n", getpid());
        // Adjunta el segmento de memoria compartida al espacio de direcciones del proceso hijo
        ptr = (char *)shmat(shm_id, 0, 0);
        char old[256];
        strcpy(old, ptr); // Inicializa 'old' con el contenido actual de la memoria compartida

        // Bucle que se ejecuta hasta que el usuario ingresa "quit"
        do {
            // Si el contenido de la memoria compartida cambia, imprime el nuevo mensaje
            if (strcmp(old, ptr) != 0) {
                printf("<--%s\n", ptr);
                strcpy(old, ptr); // Actualiza 'old' con el nuevo contenido
            }
        } while (strcmp(ptr, "quit") != 0);

        printf("[%d]shm_value->%s\n", getpid(), ptr);
        // Desadjunta el segmento de memoria compartida
        shmdt(ptr);
    } else {
        // Proceso padre
        printf("[%d]parent process started\n", getpid());
        // Adjunta el segmento de memoria compartida al espacio de direcciones del proceso padre
        ptr = (char *)shmat(shm_id, 0, 0);

        char msg[256];
        // Bucle que se ejecuta hasta que el usuario ingresa "quit"
        do {
            // Obtiene el mensaje del usuario
            fgets(msg, 256, stdin);
            msg[strlen(msg) - 1] = '\0'; // Elimina el salto de línea al final del mensaje
            // Escribe el mensaje en la memoria compartida
            strcpy(ptr, msg);
        } while (strcmp(ptr, "quit") != 0);

        // Espera a que el proceso hijo termine
        wait(NULL);

        // Desadjunta y elimina el segmento de memoria compartida
        shmdt(ptr);
        shmctl(shm_id, IPC_RMID, 0);
    }
    return EXIT_SUCCESS;
}