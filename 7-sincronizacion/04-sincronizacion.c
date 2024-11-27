#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

void* funcionHilo(void*);

pthread_barrier_t miBarrera1;
pthread_barrier_t miBarrera2;
int *data;
int ptrData = 0; // posicion donde se almacenara el siguiente numero en el vector
const int delta = 26;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
bool terminado = false;

int main(int argc, char** argv) {

    if(argc != 2) {
        perror("Se requieren 1 argumento, nombre del archivo\n");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(*(argv+1), "r");

    // lectura de 26 en 26
    const int nHilos = 2;

    pthread_barrier_init(&miBarrera1, NULL, nHilos);
    pthread_barrier_init(&miBarrera2, NULL, nHilos);

    pthread_t *hilos = malloc( sizeof(pthread_t) * nHilos ); 
    for (int i = 0; i < nHilos; i++)
        pthread_create(&hilos[i], NULL, funcionHilo, NULL);

    data = malloc( sizeof(int) * delta);

    while (fscanf(file, "%d", &data[ptrData]) == 1) {
        ptrData++;
        if(ptrData == delta) {
            pthread_barrier_wait(&miBarrera1);
            pthread_barrier_wait(&miBarrera2);
            ptrData = 0;
        }
    }
    fclose(file);
    if(ptrData > 0) {
        pthread_barrier_wait(&miBarrera1);
        pthread_barrier_wait(&miBarrera2);
    }

    terminado = true;
    
    for (int i = 0; i < nHilos; i++)
        pthread_join(hilos[i], NULL);
    
    free(hilos);
    free(data);
    pthread_barrier_destroy(&miBarrera1);
    pthread_barrier_destroy(&miBarrera2);
    pthread_mutex_destroy(&mutex);

    printf("Hilo principal mostrando total\n");
    file = fopen(argv[1], "r");
    int *dataTotal = malloc(sizeof(int));
    int ptrDataTotal = 0;
    while (fscanf(file, "%d", &dataTotal[ptrDataTotal++]) == 1) {
        dataTotal = realloc(dataTotal, sizeof(int)*(ptrDataTotal+1));
    }

    fclose(file);
    free(dataTotal);

    return EXIT_SUCCESS;
}

void* funcionHilo(void* arg) {

    while (!terminado) {
        pthread_barrier_wait(&miBarrera1);
        pthread_mutex_lock(&mutex);
        printf("Hilo %ld mostrando el vector\n", pthread_self());
        for (int j = 0; j < ptrData; j++)
            printf("%d ", data[j]);
        printf("\n");
        pthread_mutex_unlock(&mutex);
        pthread_barrier_wait(&miBarrera2);
    }
    
    pthread_exit(EXIT_SUCCESS);
}