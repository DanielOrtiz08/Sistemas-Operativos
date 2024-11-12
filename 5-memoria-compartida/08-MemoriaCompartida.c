#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

void error(const char *msg, ...);
size_t shmSize(int, int, size_t);
void matrixIndex(void**, int, int, size_t);
void multiplicarMatrices(int**, int**, int**, int, int, int);
//void multiplicarMatricesNomal(int**, int**, int**, int, int, int);

int main(int argc, char **argv) {
    if(argc != 3) error("Error, se esperan dos argumentos\n");

    FILE *fileM1 = fopen(argv[1], "r");
    if(!fileM1) error("Error al abrir el archivo de la matriz 1\n");
    
    int rowM1, colM1;
    if(fscanf(fileM1, "%d", &rowM1) != 1) error("No se logro leer correctamente el numero de filas de la matriz 1\n");
    if(fscanf(fileM1, "%d", &colM1) != 1) error("No se logro leer correctamente el numero de columnas de la matriz 1\n");

    FILE *fileM2 = fopen(argv[2], "r");
    if(!fileM2) error("Error al abrir el archivo de la matriz 2\n");
    
    int rowM2, colM2;
    if(fscanf(fileM2, "%d", &rowM2) != 1) error("No se logro leer correctamente el numero de filas de la matriz 2\n");
    if(fscanf(fileM2, "%d", &colM2) != 1) error("No se logro leer correctamente el numero de columnas de la matriz 2\n");

    if(colM1 != rowM2) error("El numero de filas de la matriz 1 debe coinsidir con el numero de columnas de la matriz 2\n");

    int sizeShmM1 = shmSize(rowM1, colM1, sizeof(int));
    int sizeShmM2 = shmSize(rowM2, colM2, sizeof(int));

    int shmIdM1 = shmget(IPC_PRIVATE, sizeShmM1, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(shmIdM1 == -1) error("Error al crear el segmento de memoria para la matriz 1\n");
    
    int shmIdM2 = shmget(IPC_PRIVATE, sizeShmM2, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(shmIdM2 == -1) error("Error al crear el segmento de memoria para la matriz 2\n");

    int **matriz1 = (int**) shmat(shmIdM1, NULL, 0);
    if(matriz1 == (void*)-1) error("Error al acoplar la matriz 1 al segmento de memoria compartido\n"); // Ensure correct checks for all shared memory

    int **matriz2 = (int**) shmat(shmIdM2, NULL, 0);
    if(matriz2 == (void*)-1) error("Error al acoplar la matriz 2 al segmento de memoria compartido\n");

    matrixIndex((void**)matriz1, rowM1, colM1, sizeof(int));
    matrixIndex((void**)matriz2, rowM2, colM2, sizeof(int));

    for (int i = 0; i < rowM1; i++) {
        for (int j = 0; j < colM1; j++) {
            fscanf(fileM1, "%d", &matriz1[i][j]);
        }
    }
    for (int i = 0; i < rowM2; i++) {
        for (int j = 0; j < colM2; j++) {
            fscanf(fileM2, "%d", &matriz2[i][j]);
        }
    }
    fclose(fileM1);
    fclose(fileM2);

    int rowR = rowM1, colR = colM2;
    int sizeShmR = shmSize(rowR, colR, sizeof(int));
    int shmIdR = shmget(IPC_PRIVATE, sizeShmR, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(shmIdR == -1) error("Error al crear el segmento de memoria para la matriz 1\n");
    
    int **matrizR = (int**) shmat(shmIdR, NULL, 0);
    if(matrizR == (void*)-1) error("Error al acoplar la matriz 1 al segmento de memoria compartido\n");
    
    matrixIndex((void**)matriz1, rowM1, colM1, sizeof(int));

    int idx, nHijos = rowR;
    for (idx = 0; idx < nHijos; idx++) {
        pid_t pid = fork();
        if(pid < 0) error("Error en fork en la iteracion %d", idx);
        if(!pid) break;
    }

    if(idx == nHijos) {

        for (int i = 0; i < nHijos; i++) {
            wait(NULL);
        }

        for (int i = 0; i < rowR; i++) {
            for (int j = 0; j < colR; j++) {
                printf("[%d] ", matrizR[i][j]);
            }
            printf("\n");
        }
        

        shmdt(matriz1);
        shmdt(matriz2);
        shmdt(matrizR);
        shmctl(shmIdM1, IPC_RMID, NULL);
        shmctl(shmIdM2, IPC_RMID, NULL);
        shmctl(shmIdR, IPC_RMID, NULL);
    } else {

        for(int i = idx; i < rowR; i+=nHijos) {
            multiplicarMatrices(matriz1, matriz2, matrizR, i, colR, rowM1);
        }

        shmdt(matriz1);
        shmdt(matriz2);
        shmdt(matrizR);
    } 

    return EXIT_SUCCESS;
}

void error(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

size_t shmSize(int row, int col, size_t size) {
    return (size_t) row * sizeof(void*) + (row * col * size);
}

void matrixIndex(void **matrix, int row, int col, size_t size) {
    *matrix = matrix + row;
    for (int i = 1; i < row; i++) {
        *(matrix+i) = *(matrix+(i-1)) + (col*size);
    }
}

void multiplicarMatrices(int** matrizA, int **matrizB, int **matrizR, int row, int col, int comun) {
    for (int j = 0; j < col; j++) {
        matrizR[row][j] = 0;
        for (int k = 0; k < comun; k++) {
            matrizR[row][j] += matrizA[row][k]*matrizB[k][j];
        }
    }   
}

/*
void multiplicarMatricesNomal(int** matrizA, int **matrizB, int **matrizR, int row, int col, int comun) {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            matrizR[i][j] = 0;
            for (int k = 0; k < comun; k++) {
                matrizR[i][j] += matrizA[i][k]*matrizB[k][j];
            }  
        }
    }   
}

*/


