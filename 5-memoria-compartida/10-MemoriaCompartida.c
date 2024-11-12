#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void error(const char *msg, ...);
void aplicarKernel(int **imagen, double **resultado, int N, int M, double kernel[3][3]);
size_t shmSize(int row, int col, size_t size);
void matrixIndex(void **matrix, int row, int col, size_t size);

int main(int argc, char** argv) {

    double deteccionBordes[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};
    double desenfoque[3][3] = {{1.0/9, 1.0/9, 1.0/9}, {1.0/9, 1.0/9, 1.0/9}, {1.0/9, 1.0/9, 1.0/9}}; //1.0 para que la division no de 0
    double realce[3][3] = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};

    if(argc != 2) error("Se requiere un argumento\n");

    FILE *file = fopen(argv[1], "r");
    if(!file) error("No se pudo abrir el archivo\n");

    int N, M;
    fscanf(file, "%d %d", &N, &M);

    size_t sizeShm = shmSize(N, M, sizeof(int));

    int shmId = shmget(IPC_PRIVATE, sizeShm, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(shmId == -1) error("No se pudo crear el segmento de memoria\n");


    int **imagen = (int**) shmat(shmId, NULL, 0);
    if(imagen == (void*)-1) error("Error al acoplar la imagen al segmento de memoria compartido\n");

    matrixIndex((void**)imagen, N, M, sizeof(int));

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {               // agregando error
            if(fscanf(file, "%1d", &imagen[i][j]) != 1) error("Error, no se pudo leer la posicion [%d][%d] del archivo\n", i, j);
        }
    }
  
    // resultado
    int sizeShmR = shmSize(N, M, sizeof(double));
    int shmIdR = shmget(IPC_PRIVATE, sizeShmR, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(shmIdR == -1) error("No se pudo crear el segmento de memoria\n");


    double **resultado = (double**) shmat(shmIdR, NULL, 0);
    if(resultado == (void*)-1) error("Error al acoplar la imagen al segmento de memoria compartido\n");

    matrixIndex((void**)resultado, N, M, sizeof(double));

    pid_t padre = getpid(), children[3];
    int nHijos = 3;

    int idx;
    for (idx = 0; idx < nHijos; idx++) {
        children[idx] = fork();
        if(children[idx] < 0) error("Error en fork en la iteracion %d\n", idx);
        if(!children[idx]) {
            break;
        }
    }

    if(padre == getpid()) {

        for (int i = 0; i < nHijos; i++) {
            kill(children[i], SIGCONT);
            wait(NULL);
        }

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                printf("%.2f \t", resultado[i][j]); // de 1.f a f o .3f para impresion mas precisa, tab para mejor visualización
            }
            printf("\n");
        }
        shmdt(resultado);
        shmctl(shmIdR, IPC_RMID, NULL);
        shmdt(imagen);
        shmctl(shmId, IPC_RMID, NULL);
    } else {
        raise(SIGSTOP);
        if(idx == 0) {
            aplicarKernel(imagen, resultado, N, M, deteccionBordes);
        }else if(idx == 1) {
            aplicarKernel(imagen, resultado, N, M, desenfoque);
        }else if(idx == 2 ) {
            aplicarKernel(imagen, resultado, N, M, realce);
        }

        shmdt(imagen);
        shmdt(resultado);
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

void aplicarKernel(int **imagen, double **resultado, int N, int M, double kernel[3][3]) {
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < M -1; j++) {
            double valor = 0; // cambiado de int a double
            for (int ki = 0; ki < 3; ki++) {
                for (int kj = 0; kj < 3; kj++) {
                    valor += kernel[ki][kj] * imagen[i+ki-1][j+kj-1];
                }
            }
            resultado[i][j] = valor;
        }   
    }
}

// no es necesario modificar las siguientes dos funciones
size_t shmSize(int row, int col, size_t size) {
    return (size_t) row * sizeof(void*) + (row * col * size);
}

void matrixIndex(void **matrix, int row, int col, size_t size) {
    *matrix = matrix + row;
    for (int i = 1; i < row; i++) {
        *(matrix+i) = *(matrix+(i-1)) + (col*size);
    }
}