#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

typedef struct {
    int maxItem;
    int suma;
} Data;

void* funcion(void*);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv) {

    int maxItem = atoi(argv[1]);
    Data *data = &(Data){maxItem, 0};

    int nIteraciones = atoi(argv[2]);
    pthread_t tids[nIteraciones];

    for (int i = 0; i < nIteraciones; i++)
        pthread_create(&tids[i], NULL, funcion, (void*) data);

    printf("Hilo principal %lu\n", (unsigned long int) pthread_self());

    for (int i = 0; i < maxItem; i++) {
        pthread_mutex_lock(&mutex);
        data->suma++;
        pthread_mutex_unlock(&mutex);
    }

    for (int i = 0; i < nIteraciones; i++) {
        pthread_join(tids[i], NULL);   
    }

    printf("suma = %d\n", data->suma);

    return EXIT_SUCCESS;
}

void* funcion(void *arg) {
    Data *data = (Data*) arg;
    printf("Hilo %ld\n", (long int) pthread_self());
    for (int i = 0; i < data->maxItem; i++) {
        pthread_mutex_lock(&mutex);
        data->suma++;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0);
}