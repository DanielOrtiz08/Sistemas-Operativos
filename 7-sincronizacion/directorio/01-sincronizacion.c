#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

pthread_barrier_t miBarrera;

int cont = 0;
void* funcionHilo(void*);

int main(void) {

    srand(time(NULL));

    int nHilos = 6;
    pthread_t hilos[nHilos];

    pthread_barrier_init(&miBarrera, NULL, nHilos);

    int ids[nHilos];
    for (int i = 0; i < nHilos; i++) {
        ids[i] = i;
        pthread_create(&hilos[i], NULL, funcionHilo, &ids[i]);
    }

    for (int i = 0; i < nHilos; i++) {
        pthread_join(hilos[i], NULL);
    }
    
    pthread_barrier_destroy(&miBarrera);

    return EXIT_SUCCESS;
}

void* funcionHilo(void* arg) {
    int idx = *(int*) arg;
    
    int pausa = 1 + rand() % 6;

    printf("Hilo %ld bloqueado por %d segundos\n", pthread_self(), pausa);
    sleep(pausa);
    printf("Hilo %ld termino su pausa de %d segundos\n", pthread_self(), pausa);

    pthread_barrier_wait(&miBarrera);

    printf("Hilos %ld finalizado\n", pthread_self());

    pthread_exit(0);
}