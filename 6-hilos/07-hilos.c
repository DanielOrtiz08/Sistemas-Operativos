#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void* funcion(void*);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int cont = 0;
int main(int argc, char **argv) {

    int turno = 0;
    int nHilos = (int) strtol(argv[1], NULL, 10);
    printf("nHilos %d\n", nHilos);
    pthread_t tids[nHilos];
    for (int i = 0; i < nHilos; i++) {
        pthread_create(&tids[i], NULL, funcion, (void*)&turno);
    }

    pthread_mutex_lock(&mutex);
    printf("Hilo princiapal %d con turno %d ", (int) pthread_self(), turno);
    turno++;
    pthread_mutex_unlock(&mutex);
    
    for (int i = 0; i < nHilos; i++)
        pthread_join(tids[i], NULL);
    
    printf("Valor final de turno %d\n", turno);

    return EXIT_SUCCESS;
}

void* funcion(void *arg) {
    int *turno = (int*) arg;
    pthread_mutex_lock(&mutex);
    printf("Hilo %d con turno %d\n", (int) pthread_self(), *turno);
    (*turno)++;
    pthread_mutex_unlock(&mutex);
}