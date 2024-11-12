#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

void error(char *msg, ...);
void multiplicarMatrices(int**, int **, int )

int main(int argc, char **argv) {
    if(argc != 3) error("Se requieren dos argumentos\n");

    FILE *file1 = fopen(argv[1], 'r');
    FILE *file2 = fopen(argv[2], 'r');
    if(file1 == NULL || file2 == NULL) 
        error("No se logro abrir alguno de los archivos\n");

    int rowM1, colM1, rowM2, colM2;
    if( (fscanf(file1, "%d", &rowM1) != 1) || (fscanf(file2, "%d", &rowM2) != 1) )
        error("No se logro leer el numero de filas en alguna de las matrices\n");
    
    if( (fscanf(file1, "%d", &colM1) != 1) || (fscanf(file2, "%d", &colM2) != 1) )
        error("No se logro leer el numero de filas en alguna de las matrices\n");

    if(rowM1 != colM2) 
        error("El numero de filas de la matriz 1 debe coincidir con el numero de columnas de la matriz 2\n");
    
    int **matriz1 = (int**) calloc(rowM1, sizeof(int*));
    if(matriz1 == NULL) error("No se pudo reservar memoria para matriz1\n");
    for (int i = 0; i < rowM1; i++) {
        matriz1[i] = (int*) calloc(colM1, sizeof(int));
        if(matriz1[i] == NULL) error("No se pudo reservar memoria para matriz1[%d]\n", i);
    }

    int **matriz2 = (int**) calloc(rowM2, sizeof(int*));
    if(matriz2 == NULL) error("No se pudo reservar memoria para matriz2\n");
    for (int i = 0; i < rowM2; i++) {
        matriz2[i] = (int*) calloc(colM2, sizeof(int));
        if(matriz2[i] == NULL) error("No se pudo reservar memoria para matriz2[%d]\n", i);
    }

    for (int i = 0; i < rowM1; i++){
        for (int j = 0; j < colM1; j++) {
            if( fscanf(file1, "%1d", &matriz1[i][j]) != 1 ) 
                error("No se pudo leer el elemento [%d][%d] de la matriz1\n", i, j);
        }
    }
    for (int i = 0; i < rowM2; i++){
        for (int j = 0; j < colM2; j++) {
            if( fscanf(file2, "%1d", &matriz2[i][j]) != 1)
                error("No se pudo leer el elemento [%d][%d] de la matriz2\n", i, j);
        }
    }

    fclose(file1); fclose(file2);

    int nProcesos = (colM1 > rowM2)? rowM2: colM1;

    int fd[nProcesos][2];
    for (int i = 0; i < nProcesos; i++)
        if(pipe(fd[i]) == -1) error("Error al crear el pipe %d", i);

    pid_t root = getpid();

    int idx;
    for (idx = 0; idx < nProcesos; idx++){
        pid = fork();
        if(pid < 0) error("Error en fork\n");
        if(!pid) break;
    }
    
    if(root != getpid()) {
        for (int i = 0; i < nProcesos; i++)
            close(fd[i][1]);



        for (int i = 0; i < nProcesos; i++)
            close(fd[i][0]);

    }else {
        for (int i = 0; i < nProcesos; i++) {
            close(fd[i][0]);
            if(idx != i) close(fd[i][1]);
        }

        for (int i = 0; i < rowM1; i++)
        {
            /* code */
        }
        

        close(fd[idx][1]);
    }

    for (int i = 0; i < rowM1; i++)
        free(matriz1[i]);
    free(matriz1);

    for (int i = 0; i < rowM2; i++)
        free(matriz2[i]);
    free(matriz2);

    return EXIT_SUCCESS;
}

void error(char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

int** multiplicarMatrices(int **matriz1, int **,matriz2, int row, int col, int comun) {
    int **matrizResultante = (int**)malloc(row*sizeof(int*));
    for (int i = 0; i < row; i++)
        matrizResultante[i] = (int*)malloc(col*sizeof(int));
    
    for (int i = 0; i < row; i++){
        for (int j = 0; j < col; j++){
            for (int i = 0; i < comun; i++) {
                matrizResultante[i][j] += matriz1[i][k] * matriz1[k][j];    
            }
        }  
    }
    return matrizResultante;
}
