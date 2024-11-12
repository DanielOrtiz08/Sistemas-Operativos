#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

struct Info{
    int i, j;
    pid_t pid;
    bool esNido;
};

void error(const char *msg, ...);
bool hayNido(int **, int, int, int, int);

int main(int argc, char **argv){
    if (argc != 2)
        error("Se requiere 1 argumento\n");
    FILE *file = fopen(argv[1], "r");
    if (file == NULL){
        error("No se pudo abrir el archivo\n");
    }

    int row, cols;
    if (fscanf(file, "%d", &row) != 1)
        error("No se pudo leer el numero de fila\n");
    if (fscanf(file, "%d", &cols) != 1)
        error("No se pudo leer el numero de columnas\n");

    int **matrix = (int **)malloc(row * sizeof(int *));
    if (matrix == NULL)
        error("No se pudo reservar memoria para matriz\n");

    for (int i = 0; i < row; i++){
        matrix[i] = (int *)malloc(cols * sizeof(int));
        if (matrix[i] == NULL)
            error("No se pudo reservar memoria para matriz[%d]\n", i);
    }

    for (int i = 0; i < row; i++){
        for (int j = 0; j < cols; j++){
            fscanf(file, "%1d", &matrix[i][j]);
        }
    }

    fclose(file);
    pid_t root = getpid();

    int fd[2][2];
    pipe(fd[0]);
    pipe(fd[1]);

    int idx;
    for (idx = 0; idx < 2; idx++){
        pid_t pid = fork();
        if (!pid)
            break;
    }

    if (root == getpid()) {
        close(fd[0][1]); 
        close(fd[1][1]);

        for (int i = 0; i < row / 2; i++) { 
            for (int j = 0; j < cols; j++) {
                struct Info info;
                read(fd[0][0], &info, sizeof(struct Info));
                if (info.esNido)
                    printf("nido encontrado en la posicion [%d][%d] por el proceso %d\n", info.i, info.j, info.pid);
            }
        }

        for (int i = row / 2; i < row; i++) {  
            for (int j = 0; j < cols; j++) {
                struct Info info;
                read(fd[1][0], &info, sizeof(struct Info));
                if (info.esNido)
                    printf("nido encontrado en la posicion [%d][%d] por el proceso %d\n", info.i, info.j, info.pid);
            }
        }

        close(fd[0][0]);  
        close(fd[1][0]);
        
        wait(NULL); 
        wait(NULL);
    }else{
        close(fd[0][0]);
        close(fd[1][0]);
        if (idx == 0){
            close(fd[1][1]);
        }
        else{
            close(fd[0][1]);
        }
        if (idx == 0){
            for (int i = 0; i < row / 2; i++){
                for (int j = 0; j < cols; j++){
                    bool band = hayNido(matrix, i, j, row, cols);
                    struct Info info = {i, j, getpid(), band};
                    write(fd[idx][1], &info, sizeof(struct Info));
                }
            }
        } else {
            for (int i = row / 2; i < row; i++){
                for (int j = 0; j < cols; j++){
                    bool band = hayNido(matrix, i, j, row, cols);
                    struct Info info = {i, j, getpid(), band};
                    write(fd[idx][1], &info, sizeof(struct Info));
                }
            }
        }

        if (idx == 0){
            close(fd[0][1]);
        }
        else{
            close(fd[1][1]);
        }
    }

    for (int i = 0; i < row; i++)
        free(matrix[i]);
    free(matrix);

    return EXIT_SUCCESS;
}

void error(const char *msg, ...){
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

bool hayNido(int **matriz, int i, int j, int row, int cols) {
    if(matriz[i][j] != 1) return false; 
    for (int vi = -1; vi <= 1; vi++) {
        for (int vj = -1; vj <= 1; vj++) {
            if(vi == 0 && vj == 0) continue;
            int ti = i + vi;
            int tj = j + vj;
            if((ti < 0 || ti >= row) || (tj < 0 || tj >= cols)) continue; // Evitar fuera de límites
            if(matriz[ti][tj] == 2) return true; 
        }
    }
    return false;
}
