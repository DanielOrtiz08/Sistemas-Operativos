#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

void error(const char *msg, ...);
size_t shmSize(int, int, size_t);
int** segmentoMatriz(int*, int);
void matrizIndex(void**, int, int, size_t);
void escanearMatriz(int**, int, int, FILE*);

int main(int argc, char **argv) {

	if(argc != ) error("");

	FILE *file = fopen(argv[], "r");

	size_t sizeShm = shmSize(, , sizeof(int));

	int nHijos;



	fclose(file);

	pid_t root = getpid(), children[nHijos];


	int idx;
	for(idx = 0; idx < nHijos; idx++) {
		children[idx] = fork();
		if(children[idx] < 0) error("Error en fork\n");
		if(!children[idx]) break;
	}


	if(root == getpid()) {

	} else {

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

size_t shmSize(int N, int M, size_t size) {
	return (size_t) N * sizeof(void*) + N * M * size;
}

int** segmentoMatriz(int *shmId, int sizeShm) {
	*shmId = shmget(IPC_PRIVATE, sizeShm, IPC_CREAT | S_IRUSR | S_IWUSR);
	if(*shmId == -1) error("Error al crear el segmento de memoria compartida\n");

	int **matriz = (int**) shmat(*shmId, NULL, 0);
	if(matriz == (void*)-1) error("No se pudo acoplar la matriz al segmento de memoria\n");
	return matriz;
}

void matrizIndex(void **matriz, int N, int M, size_t size) {
	matriz[0] = matriz + N;
	for(int i = 1; i < N; i++) {
		matriz[i] = matriz[i-1] + M * size;
	}
}

void escanearMatriz(int** matriz, int row, int col, FILE* file) {
	for(int i = 0; i < row; i++) {
		for(int j = 0; j < col; j++) {
			fscanf(file, "%d", &matriz[i][j]);
		}
	}
}