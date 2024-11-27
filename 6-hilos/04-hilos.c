#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void* funcionHilo(void*);

int valor = 0;
int maxIter = 10000;

pthread_mutex_t mt = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv) {

    int maxHilos = atoi(argv[1]);
    pthread_t tids[maxHilos];

    for (int i = 0; i < maxHilos; i++) {
        pthread_create(&tids[i], NULL, funcionHilo, NULL);
    }

    printf("Hilo principal %lu\n", pthread_self());

    for (int i = 0; i < maxHilos; i++) {
        pthread_join(tids[i], NULL);
    }
    
    printf("valor = %d\n", valor);

    pthread_mutex_destroy(&mt);

    return EXIT_SUCCESS;
}

void* funcionHilo(void *arg) {
    printf("Hilo %lu\n", pthread_self());
    for (int i = 0; i < maxIter; i++) {
        pthread_mutex_lock(&mt);
        valor++;
        pthread_mutex_unlock(&mt);
    }
    pthread_exit(0);
}