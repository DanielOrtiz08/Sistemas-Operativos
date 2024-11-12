#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h> // para S_IRUSR y S_IWUSR
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

typedef struct {
    int x;
    int y;
    pid_t pid;
}Data;


void error(const char *msg, ...);
size_t shmSize(int, int, size_t);
void matrixIndex(void**, int, int, size_t);
bool hayNido(int**, int, int, int, int);
void showTree();

int main(int argc, char **argv) {

    if(argc != 2) error("Se requiere un argumento\n");

    FILE *file = fopen(argv[1], "r");
    if(!file) error("No se pudo abrir el archivo\n");

    int row, col;
    if(fscanf(file, "%d", &row) != 1) error("No se pudo leer el numero de filas del archivo\n");
    if(fscanf(file, "%d", &col) != 1) error("No se pudo leer el numero de columnas del archiv\n");
    
    size_t sizeShm = shmSize(row, col, sizeof(int));

    int shmIdMatriz = shmget(IPC_PRIVATE, sizeShm, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(shmIdMatriz == -1) error("No se pudo crear el segmento de memoria\n");

    int **matriz = (int**) shmat(shmIdMatriz, NULL, 0);
    if(matriz == (void*)-1) error("Error al acoplar la matriz al segmento de memoria compartido\n");

    matrixIndex((void**)matriz, row, col, sizeof(int));

    int maxNumMinas = row*col;
    int shmIdRes = shmget(IPC_PRIVATE, maxNumMinas*sizeof(Data), IPC_CREAT | S_IRUSR | S_IWUSR);
    if(shmIdRes == -1) error("Error al acoplar la matriz al segmento de memoria compartido\n");

    Data *nidos = (Data*) shmat(shmIdRes, NULL, 0);
    if(nidos == (void*)-1) error("Error al acoplar el segmento de memoria para los resultados\n");
    for (int i = 0; i < maxNumMinas; i++)
        nidos[i].pid = -1;
    
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if(nidos[i].pid == 0 && nidos[i].x == 0 && nidos[i].y == 0) continue;
            if(fscanf(file, "%1d", &matriz[i][j]) != 1) ("Error, no se pudo leer la posicion [%d][%d] del archivo\n", i, j);
        }
    }

    fclose(file);

    pid_t padre = getpid();
    int nHijos = 4;

    int idx;
    for (idx = 0; idx < nHijos; idx++) {
        pid_t pid = fork();
        if(pid < 0) error("Error en fork en la iteracion %d\n", idx);
        if(!pid) {
            break;
        }
    }
    
    if(padre == getpid()) {  
        showTree();

        for (int i = 0; i < maxNumMinas; i++)
            wait(NULL);
        
        for (int i = 0; i < maxNumMinas; i++) {
            if (nidos[i].pid != -1) { // Verificar que el `pid` no sea -1
                printf("Nido encontrado en la posicion [%d][%d] por el proceso %d\n", nidos[i].x, nidos[i].y, nidos[i].pid);
            }
        }

        shmdt(matriz);
        shmctl(shmIdMatriz, IPC_RMID, NULL);
    } else {

        int iniRow = (idx < 2) ? 0: row/2;
        int finRow = (idx < 2) ? row/2: row;
        int iniCol = (idx % 2 == 0) ? 0: col/2;
        int finCol = (idx % 2 == 0) ? col/2: col;

        int tamShm = idx;
        for (int i = iniRow; i < finRow; i++) {
            for (int j = iniCol; j < finCol; j++) {
                bool band = hayNido(matriz, i, j, row, col);
                if(band) {
                    Data data = {i, j, getpid()};
                    nidos[tamShm+=nHijos] = data;
                }
            }
        }

        shmdt(matriz);
    }

    return EXIT_SUCCESS;
}


void error(const char *msg, ...){
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

bool hayNido(int **matriz, int x, int y, int row, int col) {
    if(matriz[x][y] != 1) return false;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if(x == 0 && y == 0) continue;
            int pi = x + i;
            int pj = y + j;
            if(pi < 0 || pi >= row || pj < 0 || pj >= col) continue;
            if(matriz[pi][pj] == 2) return true;
        }
    }
    return false;
}

void showTree() {
    char cmd[50];
    sprintf(cmd, "pstree -cAlp %d", getpid());
    system(cmd);
}
