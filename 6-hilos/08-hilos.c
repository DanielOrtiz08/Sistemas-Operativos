#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

void* funcion(void*);

typedef struct {
    int idx;
    pthread_barrier_t *miBarrera;
} Data;

int main(int argc, char **argv) {
    
    int nHilos = strtol(argv[1], NULL, 10);

    pthread_barrier_t miBarrera;
    pthread_t tids[nHilos];
    Data datos[nHilos];

    srand(time(NULL));
    pthread_barrier_init(&miBarrera, NULL, nHilos + 1); // Incluye el hilo principal

    for (int i = 0; i < nHilos; i++) {
        datos[i].idx = i;
        datos[i].miBarrera = &miBarrera;
        pthread_create(&tids[i], NULL, funcion, &datos[i]);
    }

    for (int i = 0; i < 3; i++) {
        pthread_barrier_wait(&miBarrera);
        int pausa = 1 + rand() % 3;
        printf("Hilo padre bloqueado por %d segundos\n", pausa);
        sleep(pausa);
        printf("Hilo padre terminó su pausa de %d segundos\n", pausa);
    }

    for (int i = 0; i < nHilos; i++) {
        pthread_join(tids[i], NULL);
    }

    pthread_barrier_destroy(&miBarrera);

    return EXIT_SUCCESS;
}

void* funcion(void* arg) {
    Data *data = (Data*)arg;
    srand(time(NULL) + data->idx);

    for (int i = 0; i < 3; i++) {
        int pausa = 1 + rand() % 3;
        printf("Hilo %d bloqueado por %d segundos\n", data->idx, pausa);
        sleep(pausa);
        printf("Hilo %d terminó su pausa de %d segundos\n", data->idx, pausa);
        
        pthread_barrier_wait(data->miBarrera);
    }

    printf("Hilo %d finalizado\n", data->idx);
    pthread_exit(NULL);
}
