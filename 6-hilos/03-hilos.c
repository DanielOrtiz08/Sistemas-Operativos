
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void* funcionHilo(void*);

typedef struct {
    int a;
}Nodo;

int main(int argc, char **argv) {

    int maxHilos = atoi(argv[1]);
    pthread_t tids[maxHilos];

    Nodo *datos[maxHilos];

    for (int i = 0; i < maxHilos; i++) {
        datos[i] = (Nodo*) malloc(sizeof(Nodo));
        datos[i]->a = i;
        pthread_create(&tids[i], NULL, funcionHilo, (void*) datos[i]);
    }

    printf("Hilo principal [%lu]\n", (long unsigned int) pthread_self());
    

    for (int i = 0; i < maxHilos; i++) {
        pthread_join(tids[i], NULL);
        free(datos[i]);
    }
    
    return EXIT_SUCCESS;
}

void *funcionHilo(void* arg) {
    Nodo *p = (Nodo*) arg;
    printf("Hilo [%lu] dato [%d]\n", pthread_self(), p->a);
    pthread_exit(NULL);
}