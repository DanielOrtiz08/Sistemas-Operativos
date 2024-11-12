#include <stdlib.h>
#include <stdarg.h> // para las funcion error
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

struct Info {
    int i, j;
    pid_t pid;
    bool esMina;
};

void error(const char *msg, ...);
bool hayMina(int**, int, int, int, int);


int main(int argc, char **argv) {

    if(argc != 2) error("Se requiere 1 argumento\n");

    FILE *file = fopen(argv[1], "r");
    if(file == NULL) {
        error("No se pudo abrir el archivo\n");
    }

    int row, cols;
    if(fscanf(file, "%d", &row) != 1)
        error("No se pudo leer el numero de fila\n");
    if(fscanf(file, "%d", &cols) != 1)
        error("No se pudo leer el numero de columnas\n");

    int **matrix = (int**) malloc(row*sizeof(int*));
    if(matrix == NULL) error("No se pudo reservar memoria para matriz\n");

    for (int i = 0; i < row; i++) {
        matrix[i] = (int *) malloc(cols*sizeof(int));
        if(matrix[i] == NULL) error("No se pudo reservar memoria para matriz[%d]\n", i);
    }

    for (int i = 0; i < row; i++) {
        for (int j = 0; j < cols; j++) {
            fscanf(file, "%1d", &matrix[i][j]);
        }
    }

    fclose(file);

    for (int i = 0; i < row; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d", matrix[i][j]);
        }
        printf("\n");
    }
    
    pid_t root = getpid();

    int fd[row][2];
    for (int i = 0; i < row; i++) {
        if(pipe(fd[i]) == -1) error("Error al crear el pipe\n");
    }

    int idx;
    for ( idx = 0; idx < row; idx++) {
        pid_t pid = fork();
        if(!pid) break;
    }
    
    if(root == getpid()) {
        for (int i = 0; i < row; i++)
            close(fd[i][1]);

        char mapaConMinas[row][cols];
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < cols; j++) {
                mapaConMinas[i][j] = 'o'; // Initialize with 'o'
            }
        }

        for (int i = 0; i < row; i++) {
            for (int j = 0; j < cols; j++) {
                struct Info info;

                read(fd[i][0], &info, sizeof(struct Info));

                if(info.esMina)
                    mapaConMinas[info.i][info.j] = '*';
                    printf("Mina encontrada en la posicion [%d][%d] por el proceso %d\n", info.i, info.j, info.pid);
                }else {
                    mapaConMinas[info.i][info.j] = 'o';
                }
            }
        }{}

        for (int i = 0; i < row; i++) {
            for (int j = 0; j < cols; j++) {
                printf("%c ", mapaConMinas[i][j]);
            }
            printf("\n");
        }

        for (int i = 0; i < row; i++)
            close(fd[i][0]);
        
    }else{
        //printf("%d\n", idx);
        for (int i = 0; i < row; i++) {
            close(fd[i][0]);
            if(idx != i) close(fd[i][1]);
        }
        
        for (int j = 0; j < cols; j++) {
            //printf("idx %d, j %d", idx, j);
            bool band = hayMina(matrix, idx, j, row, cols);

            struct Info info = {idx, j, getpid(), band};

            //printf("Child %d sending info: [%d][%d] esMina: %d\n", getpid(), idx, j, band);
            write(fd[idx][1], &info, sizeof(struct Info));
        }

        close(fd[idx][1]);
    }

    for (int i = 0; i < row; i++) {
        free(matrix[i]);
    }
    free(matrix);

    return EXIT_SUCCESS;
}

void error(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

bool hayMina(int **matriz, int i, int j, int row, int cols) {
    if(matriz[i][j] != 1) return false;
    for (int vi = -1; vi <= 1; vi++) {
        for (int vj = -1; vj <= 1; vj++) {
            if(vi == 0 && vj == 0) continue;
            int ti = i + vi;
            int tj = j + vj;
            if((ti < 0 || ti >= row) || (tj < 0 || tj >= cols)) continue;
            if(matriz[ti][tj] == 2) return true;
        }
    }
    return false;
}