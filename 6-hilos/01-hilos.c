
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void* funcionHilo(void*);

int main(void) {

    pthread_t tid;

    int *dato = malloc(sizeof(int));
    *dato = 10;

    pthread_create(&tid, NULL, funcionHilo, (void*) dato);
    printf("Hilo principal [%lu]\n",(long unsigned int) pthread_self());
    pthread_join(tid, NULL);

    //usleep(100);
    return EXIT_SUCCESS;
}

void* funcionHilo(void* arg) {
    int* parametro = (int*) arg;
    printf("Hilo1 [%lu] dato [%d]\n", (long unsigned int) pthread_self(), *parametro);
    pthread_exit(0);
}