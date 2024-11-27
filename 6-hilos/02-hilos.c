#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void* funcionHilo(void*);

struct Nodo {
    int a;
    float b;
};

int main(void) {

    pthread_t tid;
    struct Nodo *dato = (struct Nodo *)malloc(sizeof(struct Nodo));

    dato->a = 10;
    dato->b = 25.5;

    pthread_create(&tid, NULL, funcionHilo, (void*)dato);
    usleep(100);
    printf("Hilo principal [%ld] dato[%d | %.3f]\n", (long int) pthread_self(), dato->a, dato->b);
    pthread_join(tid, NULL);

    free(dato);

    return EXIT_SUCCESS;
}

void* funcionHilo(void* arg) {
    struct Nodo* p = (struct Nodo*)arg;
    p->a = 30;
    p->b = 40;
    printf("Hilo [%ld] dato[%d | %.3f]\n", (long int) pthread_self(), p->a, p->b);
    pthread_exit(0);
}