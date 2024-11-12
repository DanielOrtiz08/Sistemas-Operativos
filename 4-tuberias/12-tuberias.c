#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define NUM_HIJOS 5

int main() {
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("Error al crear la tubería");
        return 1;
    }

    pid_t children[NUM_HIJOS];
    int idx;
    for (idx = 0; idx < NUM_HIJOS; idx++) {
        children[idx] = fork();
        if (children[idx] == -1) {
            perror("Error al crear el hijo");
            return 1;
        }
        if (children[idx] == 0) break; // Proceso hijo
    }

    if (idx == NUM_HIJOS) { // padre
        close(pipe_fd[1]);

        for (int i = 0; i < NUM_HIJOS; i++) {
            kill(children[i], SIGCONT);
            pid_t pid;
            int vector_size;

            // Leer el PID y el tamaño del vector
            read(pipe_fd[0], &pid, sizeof(pid));
            read(pipe_fd[0], &vector_size, sizeof(vector_size));

            // Reservar memoria para el vector
            int* vector = malloc(vector_size * sizeof(int));
            if (vector == NULL) {
                perror("Error al asignar memoria");
                exit(1);
            }

            // Leer el vector
            read(pipe_fd[0], vector, vector_size * sizeof(int));

            printf("Mensaje recibido del hijo con PID %d:\n", pid);
            printf("Tamaño del vector: %d\n", vector_size);
            printf("Vector: ");
            for (int j = 0; j < vector_size; j++) {
                printf("%d ", vector[j]);
            }
            printf("\n\n");

            free(vector); // Liberar memoria
        }

        close(pipe_fd[0]);

        for (int i = 0; i < NUM_HIJOS; i++)
            wait(NULL);

    } else { // Proceso hijo
        srand(time(NULL) + getpid());

        close(pipe_fd[0]);

        pid_t pid = getpid();
        int vector_size = rand() % 10 + 1; // Tamaño aleatorio entre 1 y 10

        // Reservar memoria para el vector
        int* vector = malloc(vector_size * sizeof(int));
        if (vector == NULL) {
            perror("Error al asignar memoria");
            exit(1);
        }

        for (int j = 0; j < vector_size; j++) {
            vector[j] = rand() % 100; // Llenar el vector con números aleatorios
        }

        raise(SIGSTOP);

        // Escribir datos en la tubería
        write(pipe_fd[1], &pid, sizeof(pid)); // Enviar PID
        write(pipe_fd[1], &vector_size, sizeof(vector_size)); // Enviar tamaño del vector
        write(pipe_fd[1], vector, vector_size * sizeof(int)); // Enviar contenido del vector

        // Liberar memoria antes de cerrar la escritura
        free(vector);
        close(pipe_fd[1]);
    }

    return 0;
}
