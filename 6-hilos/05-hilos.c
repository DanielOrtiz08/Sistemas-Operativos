#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

void* funcion(void*);
int suma = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv) {

    int maxItem = atoi(argv[1]);

    pthread_t tid;

    pthread_create(&tid, NULL, funcion, (void*) &maxItem);

    printf("Hilo principal %lu\n", (unsigned long int) pthread_self());

    for (int i = 0; i < maxItem; i++) {
        pthread_mutex_lock(&mutex);
        suma++;
        pthread_mutex_unlock(&mutex);
    }

    pthread_join(tid, NULL);

    printf("suma = %d\n", suma);

    return EXIT_SUCCESS;
}

void* funcion(void *max_Item) {
    int maxItem = *(int*) max_Item;
    printf("Hilo %ld\n", (long int) pthread_self());
    for (int i = 0; i < maxItem; i++) {
        pthread_mutex_lock(&mutex);
        suma++;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0);
}