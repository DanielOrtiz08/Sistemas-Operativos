#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define NUM_HIJOS 5

typedef struct {
    pid_t pid;        
    int vector_size;  
    int vector[20];   
} message_t;

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
        if (children[idx] == 0) break;
    }

    if(idx == NUM_HIJOS) { // padre

        close(pipe_fd[1]);
    
        for (int i = 0; i < NUM_HIJOS; i++) {
            kill(children[i], SIGCONT);
            message_t msg;
            read(pipe_fd[0], &msg, sizeof(message_t));

            printf("Mensaje recibido del hijo con PID %d:\n", msg.pid);
            printf("Tamaño del vector: %d\n", msg.vector_size);
            printf("Vector: ");
            for (int j = 0; j < msg.vector_size; j++) {
                printf("%d ", msg.vector[j]);
            }
            printf("\n\n");
        }

        close(pipe_fd[0]); 

        for (int i = 0; i < NUM_HIJOS; i++)
            wait(NULL);

    } else {
        srand(time(NULL) + getpid());
        
        close(pipe_fd[0]);

        message_t msg;
        msg.pid = getpid();
        msg.vector_size = rand() % 10 + 1; 

        
        for (int j = 0; j < msg.vector_size; j++) {
            msg.vector[j] = rand() % 100; 
        }

        raise(SIGSTOP);
        write(pipe_fd[1], &msg, sizeof(message_t));

        close(pipe_fd[1]);
    }

    return 0;
}
