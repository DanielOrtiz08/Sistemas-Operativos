#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void error(const char *, ...);
void* funcionHiloBusqueda(void *arg);
void* funcionHiloConteo(void *arg);
bool isSecuencia(int start, char *caracteres);


typedef struct {
    int index;
}Nodo;

typedef struct {
    int posicion;
    int hilo;
}Secuencia;

int numBases;
char *vectorADN;
int nHilos;
int nCaracteres[4] = {0,0,0,0};
Secuencia secuencias[1000000];
int nSec = 0;

int main(int argc, char **argv) {

    if(argc!=3) error("Se requieren dos argumentos\n");

    FILE *file = fopen(argv[1], "r");
    if(!file) error("No se pudo abrir el archivo\n");

    fscanf(file, "%d", &numBases);

    nHilos = atoi(argv[2]);

    if(nHilos+1 > numBases) error("No sé necesitan tantos hilos, el vector tiene %d bases, maximo %d hilos\n", numBases, numBases+1);

    vectorADN = (char*) malloc(numBases * sizeof(char));
    for (int i = 0; i < numBases; i++) {
        fscanf(file, " %c", &vectorADN[i]);
        //printf("%c", vectorADN[i]);
    }
    //printf("\n");

    pthread_t tids[nHilos];
    pthread_create(&tids[nHilos-1], NULL, funcionHiloConteo, NULL);
    Nodo *indices[nHilos-1];
    for (int i = 0; i < nHilos-1; i++) {
        indices[i] = (Nodo*) malloc(sizeof(Nodo));
        indices[i]->index = i;
        pthread_create(&tids[i], NULL, funcionHiloBusqueda, (void*) indices[i]);
    }

    for (int i = 0; i < nHilos; i++) {
        pthread_join(tids[i], NULL);
    }

    printf("Estadisticas:\n");
    printf("A: %d veces\n", nCaracteres[0]);
    printf("T: %d veces\n", nCaracteres[1]);
    printf("C: %d veces\n", nCaracteres[2]);
    printf("G: %d veces\n", nCaracteres[3]);

    printf("La secuencia TTGTAC aparece %d veces\n", nSec);

    printf("Secuencias:\n");
    for (int h = 0; h < nHilos - 1; h++) {
        printf("Hilo [%d]:\n", h);
        bool encontrado = false;
        for (int i = 0; i < nSec; i++) {
            if (secuencias[i].hilo == h) {
                printf("Secuencia en la posición %d\n", secuencias[i].posicion);
                encontrado = true;
            }
        }
        if (!encontrado) {
            printf("No se encontraron secuencias.\n");
        }
    }

    fclose(file);
    
    for (int i = 0; i < nHilos - 1; i++) {
        free(indices[i]);
    }

    free(vectorADN);

    return EXIT_SUCCESS;
}


void error(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

void* funcionHiloBusqueda(void *arg) {
    Nodo *idxs = (Nodo*) arg;
    int delta = (numBases + nHilos - 2) / (nHilos - 1);
    int start = idxs->index * delta;
    int end = (idxs->index == nHilos - 2) ? numBases : start + delta;
 
    //printf("hilo %d, start %d, end %d\n", idxs->index, start, end);
    
    for(int j=start; j<end; j++){
        if(vectorADN[j] == 'T' && j < numBases-5){ // j < numBases-5 para no verificar los ultimos 5 caracters, que ya no cumplirian la secuencia por su tamaño
            if(isSecuencia(j+1, vectorADN)){
                pthread_mutex_lock(&mutex);
                secuencias[nSec].posicion = j;
                secuencias[nSec++].hilo = idxs->index;
                pthread_mutex_unlock(&mutex);
                j+=5; 
            }
        }
    }
    pthread_exit(0);
}

bool isSecuencia(int start, char *caracteres) {
    const char aux[] = "TGTAC"; // No incluye la primera 'T', ya fue validada fuera de la función
    int auxLength = sizeof(aux) - 1; // Tamaño real sin el carácter nulo

    for (int i = 0; i < auxLength; i++) {
        if (caracteres[start + i] != aux[i]) {
            return false;
        }
    }

    return true;
}

void* funcionHiloConteo(void *arg) {
    for (int i = 0; i < numBases; i++) {
        switch (vectorADN[i]) {
        case 'A':
            nCaracteres[0]++;
            break;
        case 'T':
            nCaracteres[1]++;
            break;
        case 'C':
            nCaracteres[2]++;
            break;
        case 'G':
            nCaracteres[3]++;
            break;
        }
    }
    pthread_exit(0);
}

