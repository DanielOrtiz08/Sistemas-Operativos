#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h> // para S_IRUSR y S_IWUSR
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

//#include <stdbool.h>
#include <ctype.h>

void handler();

int main(int argc, char **argv) {

    void *oldHandler = signal(SIGUSR1, handler);
    if(oldHandler == SIG_ERR) {
        perror("Error en signal\n");
        exit(EXIT_FAILURE);
    }

    pid_t children[2], padre = getpid();
    int shmSize = sizeof(int);

    int *vector = (int*)malloc(3*sizeof(int));
    int *referencia = vector


    int shmKey = shmget(IPC_PRIVATE, shmSize, IPC_CREAT | S_IRUSR | S_IWUSR);

    int *turno = (int*) shmat(shmKey, 0, 0);
    *turno = 0;
    
    int i;
    for ( i = 0; i < 2; i++ ) {
        children[i] = fork();
        if(!children[i]) break;
    }
    
    if(padre == getpid()) {
        usleep(100);
        printf("Padre [%d]\n", getpid());
        kill(children[0], SIGUSR1);
        if(*turno == 0) {
            *turno = 1;
            kill(children[0], SIGUSR1);
        } else if(*turno == 1) {
            *turno = 0;
            kill(children[1], SIGUSR1);
        }
        wait(NULL);
        shmdt(turno);
        shmctl(shmKey, IPC_RMID, 0);
    } else {
        pause();
        printf("Hijo [%d]\n", i+1);
        shmdt(turno);
    }

    if(signal(SIGUSR1, oldHandler) == SIG_ERR) {
        perror("Error en signal\n");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

void handler() {}