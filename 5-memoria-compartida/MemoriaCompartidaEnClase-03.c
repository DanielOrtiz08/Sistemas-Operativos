#include <stdio.h>
#include <stdlib.h>  // malloc, calloc, free, etc
#include <sys/shm.h> // para funciones de manejo de memoria comparatida
#include <sys/ipc.h> // para banderas IPC
#include <sys/stat.h> // para constantes de modo de acceso
#include <stdbool.h>
#include <sys/types.h>
#include <stdarg.h> // para las funcion error
#include <string.h>
#include <ctype.h> // para isdigit
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

unsigned int sizeof_dm(int, int, size_t);
void create_index(void **, int, int, size_t);
void validaciones(int, char **, int *, int *);
void error(char *, ...);
bool esNumero(char *);
int strToInt(char *);
void handler(int sig);

int main(int argc, char **argv) {

    int rows, cols;
    validaciones(argc, argv, &rows, &cols);

    size_t sizeMatrix = sizeof_dm(rows, cols, sizeof(double));

    int shmId = shmget(IPC_PRIVATE, sizeMatrix, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(shmId < 0) error("No se pudo crear el segmento de memoria  \n");
    
    double **matrix = (double**)shmat(shmId, NULL, 0);
    if(matrix == (void*)-1) error("Error en shmat hijo\n");

    create_index((void **)matrix, rows, cols, sizeof(double));

    pid_t pid = fork();

    if(pid == -1) error("Error en fork\n");
    if(pid) {
        
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                *(*(matrix+i)+j) = (i*cols)+j;
            }
        }
        usleep(1000);
        kill(pid, SIGCONT);
        wait(NULL);
        shmdt(matrix);
        shmctl(shmId, IPC_RMID, NULL);
    } else {
        void *oldHandler = signal(SIGUSR1, handler);
        if(oldHandler == SIG_ERR) error("Error en signal\n");

        raise(SIGSTOP);
        
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                printf("[%.1f]\t", *(*(matrix+i)+j) );
            }
            printf("\n");
        }
        shmdt(matrix);

        if(signal(SIGUSR1, oldHandler) == SIG_ERR) error("Error en signal\n");
    }

    return EXIT_SUCCESS;
}

unsigned int sizeof_dm(int rows, int cols, size_t sizeElement) {
    size_t size;
    size = rows * sizeof(void *);
    size += (cols * rows * sizeElement);
    return (unsigned int)size;
}

void create_index(void **m, int rows, int cols, size_t sizeElement) {
    size_t sizeRow = cols * sizeElement;
    m[0] = m + rows;
    for (int i = 1; i < rows; i++) {
        m[i] = m[i-1] + sizeRow;
    }
}

void validaciones(int argc, char **argv, int *rows, int *cols) {
    if (argc != 3)
        error("Se requieren 2 argumentos.\nEjemplo: name rows cols -> %s 6 3\n", *(argv));

    if (!esNumero(*(argv + 1)))
        error("Debe ingresar un numero valido para rows\n");
    if (!esNumero(*(argv + 2)))
        error("Debe ingresar un numero valido para cols\n");

    *rows = strToInt(*(argv + 1));

    sscanf(*(argv + 2), "%d", cols);
}

void error(char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

bool esNumero(char *numeroStr) {
    for (int i = 0; i < strlen(numeroStr); i++)
        if (!isdigit(*(numeroStr + i)))
            return false;

    return true;
}

int strToInt(char *numeroStr) {
    int numeroInt = 0;
    for (int i = 0; i < strlen(numeroStr); i++) {
        if (*(numeroStr + i) >= '0' && *(numeroStr + i) <= '9') {
            numeroInt *= 10;
            numeroInt += *(numeroStr + i) - '0';
        }
        else
            break;
    }
    return numeroInt;
}
void handler(int sig){}