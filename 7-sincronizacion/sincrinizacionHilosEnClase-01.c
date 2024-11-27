#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdarg.h>

typedef struct {
    int N;
    int M;
    int anios;
    int **matrizDeforestacion;
    int **matrizAxiliar; // almacenará valores booleanos
    pthread_barrier_t *miBarrera;
    pthread_barrier_t *miBarrera2;
} Data;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void error(const char *, ...);
void* deBosqueADeforestado(void*);
void* deDeforestadoARegenerado(void*);

int main(int argc, char **argv) {

    if(argc != 2) error("Se requieren un argumento\n");

    FILE *file = fopen(argv[1], "r");

    Data data;
    fscanf(file, "%d %d %d", &data.anios, &data.N, &data.M);

    // para las matrices si reservamos memoria :/
    data.matrizDeforestacion = (int**) malloc(data.N * sizeof(int*));
    for (int i = 0; i < data.N; i++)
        data.matrizDeforestacion[i] = (int*) malloc(data.M * sizeof(int));

    data.matrizAxiliar = (int**) calloc(data.N, sizeof(int*));
    for (int i = 0; i < data.N; i++)
        data.matrizAxiliar[i] = (int*) calloc(data.M, sizeof(int));
    
    for (int i = 0; i < data.N; i++) {
        for (int j = 0; j < data.M; j++) {
            fscanf(file, "%d", &data.matrizDeforestacion[i][j]);
        }
    }

    int nHilos = 2;

    pthread_barrier_t barrera; // el ptr MiBarrera del struct apuntara a este para no tener que reservar memoria :)
    data.miBarrera = &barrera;

    pthread_barrier_t barrera2;
    data.miBarrera2 = &barrera2;

    pthread_barrier_init(data.miBarrera, NULL, nHilos+1); // incluyendo el hilo principal
    pthread_barrier_init(data.miBarrera2, NULL, nHilos+1);

    pthread_t tids[nHilos];
    pthread_create(&tids[0], NULL, deBosqueADeforestado, (void*) &data);
    pthread_create(&tids[1], NULL, deDeforestadoARegenerado, (void*) &data);

    for (int i = 0; i < data.anios; i++) {
        pthread_barrier_wait(data.miBarrera); // una vez que los dos hilos terminen el primer ciclo mostrar la matriz
        printf("principal ejecutando año %d despues que los hijos lo terminan\n", i);        
        pthread_barrier_wait(data.miBarrera2); // ahora si los hijos pueden ejecutarse porque el padre termino su ciclo
    }

    for (int i = 0; i < nHilos; i++)
        pthread_join(tids[i], NULL);

    pthread_barrier_destroy(data.miBarrera);
    pthread_barrier_destroy(data.miBarrera2);

    return EXIT_SUCCESS;
}

void error(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

void* deBosqueADeforestado(void *arg) {
    Data *data = (Data*) arg;

    for (int t = 0; t < data->anios; t++) {

        printf("Hilo 1 ejecutando año %d\n", t);
        
        // este primer barrier permite que los hilos secundarios ejecuten un t mientras que el principal primero se bloque y despues ejecute
        pthread_barrier_wait(data->miBarrera); // esperar a que todos los hilos (2 aqui) lleguen a este punto para continuar y asi controlar la sincronizacion 
        // este se usa para que una vez el principal termine su ciclo permita a los secundarios continuar
        pthread_barrier_wait(data->miBarrera2);
    }
    
    pthread_exit(0);
}

void* deDeforestadoARegenerado(void *arg) {
    Data *data = (Data*) arg;

    for (int t = 0; t < data->anios; t++) {
        
        printf("Hilo 2 ejecutando año %d\n", t);
        pthread_barrier_wait(data->miBarrera); // esperar a que todos los hilos (2 aqui) lleguen a este punto para continuar y asi controlar la sincronizacion 
        pthread_barrier_wait(data->miBarrera2);
    }
    
    pthread_exit(0);
}