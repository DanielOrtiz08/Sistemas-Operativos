#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

int turno = 0;
void* funcionHilo(void*);

int main(void) {

    srand(time(NULL));

    int nHilos = 100;
    pthread_t *hilos = malloc(nHilos * sizeof(pthread_t));

    int *ids = malloc(nHilos * sizeof(int));
    for (int i = 0; i < nHilos; i++) {
        ids[i] = i;
        pthread_create(&hilos[i], NULL, funcionHilo, &ids[i]);
    }

    /*int *id;
    for (int i = 0; i < nHilos; i++) {
        id = malloc(sizeof(int));
        *id = i;
        pthread_create(&hilos[i], NULL, funcionHilo, (void*) id);
    }*/

    clock_t t_ini = clock();
    
    for (int i = 0; i < nHilos; i++) {
        pthread_join(hilos[i], NULL);
    }

    clock_t t_fin = clock();

    double secs = (double)(t_fin - t_ini) / CLOCKS_PER_SEC;
    printf("%.16g milisegundos\n", secs * 1000.0);

    free(hilos);

    return EXIT_SUCCESS;
}

void* funcionHilo(void *arg) {
    int miTurno = *(int*)arg;
    
    while (miTurno != turno);

    printf("Ejecutando hilo %ld con turno %d\n", pthread_self(), miTurno);
    turno++;
    pthread_exit(EXIT_SUCCESS);
}