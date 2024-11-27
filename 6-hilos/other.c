#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/shm.h>  // Para shmget(), shmctl(), shmat(), shmdt()
#include <sys/stat.h> // Para constantes de modo de acceso

typedef struct
{
    int a;
    int t;
    int c;
    int g;
    char *matriz;
    int numBases;
    int numSecuencias;
    int *posIni;
    int *posFin;
} Datos;

Datos *datos;

void leerMatriz(char **matriz, int *numBases, char *filename)
{
    FILE *file = fopen(filename, "r");
    fscanf(file, "%d", numBases);

    int sizeSecuencia = (*numBases);
    (*matriz) = (char *)malloc(sizeSecuencia * sizeof(char));

    for (int i = 0; i < sizeSecuencia; i++)
    {
        fscanf(file, " %c", &((*matriz)[i]));
    }
    fclose(file);
}

void mostrarMatriz(char *matriz, int numBases)
{
    for (int i = 0; i < numBases; i++)
    {
        printf("%c", matriz[i]);
    }
}

void *contarBases()
{

    for (int i = 0; i < datos->numBases; i++)
    {
        switch (datos->matriz[i])
        {
        case 'A':
            datos->a++;
            break;
        case 'T':
            datos->t++;
            break;
        case 'C':
            datos->c++;
            break;
        case 'G':
            datos->g++;
            break;
        }
    }
}

void *contarSecuencias()
{
    char vectorAux[6] = {'T', 'T', 'G', 'T', 'A', 'C'};

    for (int i = 0; i < datos->numBases; i++)
    {
        if (datos->matriz[i] == 'T')
        {
            int k = 0;
            int j = i;
            for (; j < i+6 && j < datos->numBases; j++)
            {
                if (datos->matriz[j] == vectorAux[k])
                {
                    k++;
                }
                else
                {
                    break;
                }
            }
            if (k == 6)
            {
                datos->posIni[datos->numSecuencias] = i;
                datos->posFin[datos->numSecuencias] = j;
                datos->numSecuencias++;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    char *matriz;
    int numBases;

    leerMatriz(&matriz, &numBases, argv[1]);
    // mostrarMatriz(matriz, numBases);

    datos = malloc(sizeof(Datos));
    datos->matriz = matriz;
    datos->numBases = numBases;
    datos->numSecuencias = 0;
    datos->posIni = (int *)malloc(numBases * sizeof(int));
    datos->posFin = (int *)malloc(numBases * sizeof(int));
    datos->a = 0;
    datos->t = 0;
    datos->c = 0;
    datos->g = 0;

    pthread_t thread_id[2];
    pthread_create(&thread_id[0], NULL, contarBases, NULL);
    pthread_create(&thread_id[1], NULL, contarSecuencias, NULL);
    
    // Esperar a que todos los hilos terminen
    for (int i = 0; i < 2; i++) {
        pthread_join(thread_id[i], NULL);
    }

    printf("Conteo de bases\n A = %d, T = %d, C = %d, G = %d \n", datos->a, datos->t, datos->c, datos->g);
    printf("Conteo de secuencias\n Numero de secuencias = %d\n", datos->numSecuencias);
    
    for (int i = 0; i < datos->numSecuencias; i++)
    {
        printf("Secuencia en la posicion inicial [%d] - posicion final [%d]\n", datos->posIni[i], datos->posFin[i]);
    }
    
}