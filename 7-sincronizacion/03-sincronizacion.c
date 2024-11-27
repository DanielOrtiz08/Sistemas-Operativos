#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

int turno = 0;
pthread_mutex_init mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* funcionHilo(void*);

int main(void) {

    srand(time(NULL));

    int nHilos = 100;
    int *ids = malloc(nHilos * sizeof(int));
    pthread_t *hilos = malloc(nHilos * sizeof(pthread_t));

    for (int i = 0; i < nHilos; i++) {
        ids[i] = i;
        pthread_create(&hilos[i], NULL, funcionHilo, &ids[i]);
    }

    clock_t t_ini = clock();
    
    for (int i = 0; i < nHilos; i++) {
        pthread_join(hilos[i], NULL);
    }

    clock_t t_fin = clock();

    double secs = (double)(t_fin - t_ini) / CLOCKS_PER_SEC;
    printf("%.16g milisegundos\n", secs * 1000.0);

    free(hilos);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return EXIT_SUCCESS;
}

void* funcionHilo(void *arg) {
    int miTurno = *(int*)arg;

    pthread_mutex_lock(&mutex);

    while (miTurno != turno)
        pthread_cond_wait(&cond, &mutex);

    printf("Ejecutando hilo %ld con turno %d\n", pthread_self(), miTurno);
    turno++;

    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
    
    pthread_exit(EXIT_SUCCESS);
}