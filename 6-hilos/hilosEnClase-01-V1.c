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

    FILE *file = fopen(argv[1], "r");

    Data data;
    fscanf(file, "%d %d %d", &data.anios, &data.N, &data.M);

    // para las matrices si reservamos memoria :/
    data.matrizDeforestacion = (int**) malloc(data.N * sizeof(int*));
    for (int i = 0; i < data.N; i++)
        data.matrizDeforestacion[i] = (int*) malloc(data.M * sizeof(int));

    data.matrizAxiliar = (int**) malloc(data.N * sizeof(int*));
    for (int i = 0; i < data.N; i++)
        data.matrizAxiliar[i] = (int*) malloc(data.M * sizeof(int));

    printf("Matriz inicial \n");
    for (int i = 0; i < data.N; i++) {
        for (int j = 0; j < data.M; j++) {
            fscanf(file, "%d", &data.matrizDeforestacion[i][j]);
            printf("%d ", data.matrizDeforestacion[i][j]);
        }
        printf("\n");
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

    for (int t = 0; t < data.anios; t++) {
        pthread_barrier_wait(data.miBarrera); // una vez que los dos hilos secundarios terminen el primer ciclo mostrar la matriz
        printf("\nAño %d\n", t+1);
        for (int i = 0; i < data.N; i++) {
            for (int j = 0; j < data.M; j++) {
                printf("%d ", data.matrizDeforestacion[i][j]);
            }
            printf("\n");
        }
        sleep(1); // se debe quitar, solo es para visualizar mejor el resulado de cada año

        // reiniciamos la matriz en falsos
        for (int i = 0; i < data.N; i++) {
            for (int j = 0; j < data.M; j++) {
                data.matrizAxiliar[i][j] =  0;
            }
        }
        pthread_barrier_wait(data.miBarrera2); // ahora si los hijos pueden ejecutarse porque el padre termino su ciclo
    }

    for (int i = 0; i < nHilos; i++)
        pthread_join(tids[i], NULL);

    pthread_barrier_destroy(data.miBarrera);
    pthread_barrier_destroy(data.miBarrera2);

    for (int i = 0; i < data.N; i++) {
        free(data.matrizDeforestacion[i]);
        free(data.matrizAxiliar[i]);
    }
    free(data.matrizDeforestacion);
    free(data.matrizAxiliar);


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

        for (int i = 0; i < data->N; i++) {
            for (int j = 0; j < data->M; j++) {
                if(data->matrizDeforestacion[i][j] != 1) continue; // si no es un bosque no vale la pena verificar
                
                // visitar vesinos por cada posicion
                int vecinosDeforestados = 0;
                for (int vi = -1; vi <= 1; vi++) {
                    for (int vj = -1; vj <= 1; vj++) {
                        if(vi == 0 && vj == 0) continue; // no validad cuando se está en la misma posicion
                        int pi = vi + i; int pj = vj + j; // obtengo la posicion del vecino 
                        if(pi < 0 || pi >= data->N || pj < 0 || pj >= data->M) continue; // rango fuera de la matriz (considerado bosque para este problema)
                        if(data->matrizDeforestacion[pi][pj] == 0 && data->matrizAxiliar[pi][pj] != 1) vecinosDeforestados++; // matriz auxiliar almacena true si el elemento fue modificado en este ciclo 
                    }
                } 
                // printf("bosque [%d][%d]: tiene %d vecinos deforestados\n", i, j, vecinosDeforestados);
                if(vecinosDeforestados >= 4) {
                    pthread_mutex_lock(&mutex);
                    data->matrizDeforestacion[i][j] = 0;
                    pthread_mutex_unlock(&mutex);
                    data->matrizAxiliar[i][j] = 1; // 1 quiere decir que cumplio la condicion, mas no que es un bosque // no es necesario usar mutex porque es el unico que actualiza esta matriz
                    /*
                    se guarda su valor en una matriz auxiliar porque si se llega a cumplir esta condicion, se establecera en 0(deforestacion),
                    y si en este mismo ciclo el otro hilo, llega a leer esta posicion como 0(deforestacio) puede succeder que tambien la actualice 
                    en 2(regeneracion) provocando que en un mismo ciclo una posicion cambie dos veces de estado
                    */
                }
            }
        }
        
        // este primer barrier permite que los hilos secundarios ejecuten un t mientras que el principal primero se bloque y despues ejecute
        pthread_barrier_wait(data->miBarrera); // esperar a que todos los hilos (2 aqui) lleguen a este punto para continuar y asi controlar la sincronizacion 
        // este se usa para que una vez el principal termine su ciclo permita a los secundarios continuar
        pthread_barrier_wait(data->miBarrera2);
    }
    
    pthread_exit(0);
}

void* deDeforestadoARegenerado(void *arg) {
    Data *data = (Data*) arg;

    // matriz con logica booleana

    for (int t = 0; t < data->anios; t++) {
        int aux[data->N][data->M];
        for (int i = 0; i < data->N; i++) {
            for (int j = 0; j < data->M; j++) {
                aux[i][j] =  0;
            }
        }
        for (int i = 0; i < data->N; i++) {
            for (int j = 0; j < data->M; j++) {
                if(data->matrizDeforestacion[i][j] != 0 || data->matrizAxiliar[i][j] == 1) continue;  // si no esta deforestado no vale la pena verificar, y si estta deforestado pero es porque el otro hilo lo cambio entonces tampoco validar
                int vecinosRegenerados = 0;

                // visitar vesinos por cada posicion
                for (int vi = -1; vi <= 1; vi++) {
                    for (int vj = -1; vj <= 1; vj++) {
                        if(vi == 0 && vj == 0) continue; // no valida cuando se está en la misma posicion
                        int pi = vi + i; int pj = vj + j;
                        if(pi < 0 || pi >= data->N || pj < 0 || pj >= data->M) continue; // rango fuera de la matriz (considerado bosque para este problema)
                        if(data->matrizDeforestacion[pi][pj] == 2 && aux[pi][pj] != 1) vecinosRegenerados++;
                    }
                }
                // +printf("deforestado [%d][%d]: tiene %d vecinos regenerados\n", i, j, vecinosRegenerados);
                if(vecinosRegenerados >= 5) {
                    pthread_mutex_lock(&mutex);
                    data->matrizDeforestacion[i][j] = 2; // se puede modificar la matriz principal directamente, porque su valor no será afectado por el otro hilo
                    aux[i][j] = 1; // se pone en 1 para para luego validar que otro vecino que posiblemente cumpla la condicion de deforestado no cuente a este como un vecino deforestado
                    // ejemplo si [0][0] es desforestado y cambia a regenerado, entonces el vecino de al lado [0][1] suponiendo tamben es deforestado no debe tener en cuenta a [0][0] como regenerado 
                    pthread_mutex_unlock(&mutex);
                }
            }
        }

        pthread_barrier_wait(data->miBarrera); // esperar a que todos los hilos (2 aqui) lleguen a este punto para continuar y asi controlar la sincronizacion 
        pthread_barrier_wait(data->miBarrera2);
    }
    
    pthread_exit(0);
}