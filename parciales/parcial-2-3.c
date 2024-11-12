#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <signal.h>
#include <wait.h>

void error(const char *msg, ...);
size_t shmSize(int, int, size_t);
int** segmentoMatriz(int*, int);
void matrizIndex(void**, int, int, size_t);
void escanearMatriz(int**, int, int, FILE*);

int main(int argc, char **argv) {

	if(argc != 2) error("Se espera un argumento (nombre del archivo)\n");

	FILE *file = fopen(argv[1], "r");
	if(!file) error("No se pudo abrir el archivo\n");

	int N, M;
	if(fscanf(file, "%d %d", &N, &M) != 2) error("No se pudo leer las dimensiones de la matriz\n");


	size_t sizeShm = shmSize(N, M, sizeof(int));

	// Matriz de objetos
	int shmIdobjetos = -1;
	int **matrizObjetos = segmentoMatriz(&shmIdobjetos, sizeShm);
	matrizIndex((void**)matrizObjetos, N, M, sizeof(int));
	escanearMatriz(matrizObjetos, N, M, file);


	// Matriz de referencias
	int shmIdReferencias = -1;
	int **matrizReferencias = segmentoMatriz(&shmIdReferencias, sizeShm);
	matrizIndex((void**)matrizReferencias, N, M, sizeof(int));
	escanearMatriz(matrizReferencias, N, M, file);

	// Matriz de tiempo
	int shmIdTiempo = -1;
	int **matrizTiempo = segmentoMatriz(&shmIdTiempo, sizeShm);
	matrizIndex( (void**)matrizTiempo, N, M, sizeof(int) );
	escanearMatriz(matrizTiempo, N, M, file);

	fclose(file);

	// Matriz de Fragmentacion
	int shmIdFragmentacion = -1;
	int **matrizFragmentacion = segmentoMatriz(&shmIdFragmentacion, sizeShm);
	matrizIndex( (void**)matrizFragmentacion, N, M, sizeof(int) );

	// Matriz de Recoleccion
	int shmIdRecoleccion = -1;
	int **matrizRecoleccion = segmentoMatriz(&shmIdRecoleccion, sizeShm);
	matrizIndex( (void**)matrizRecoleccion, N, M, sizeof(int) );

	int nHijos = 2;
	pid_t root = getpid(), children[nHijos];

	int idx;
	for(idx = 0; idx < nHijos; idx++) {
		children[idx] = fork();
		if(children[idx] < 0) error("Error en fork\n");
		if(!children[idx]) break;
	}


	if(root == getpid()) {
		int status;
		for (int i = 0; i < nHijos; i++) {
			waitpid(-1, &status, WUNTRACED);
			if(! WIFSTOPPED(status)) i--;
		}

		for (int i = 0; i < N; i++) {
			for (int j = 0; j < M; j++) {
				if(matrizFragmentacion[i][j] > 0.5 && matrizRecoleccion[i][j] > 0.6) matrizObjetos[i][j] = 2;
				else if(matrizFragmentacion[i][j] > 0.7 && matrizRecoleccion[i][j] > 0.8) matrizObjetos[i][j] = 1;
				else matrizObjetos[i][j] = 0;
				printf("%d\t", matrizObjetos[i][j]);

				matrizTiempo[i][j] = (matrizObjetos[i][j] != 0)? ((matrizTiempo[i][j] == -1)? 1 : matrizTiempo[i][j]++) : -1;

			}
			printf("\n");
		}

		for (int i = 0; i < nHijos; i++) {
			kill(children[i], SIGCONT);
		}
	} else {
		for(int i = 0; i < N; i++) {
			for(int j = 0; j < M; j++) {
				if(idx == 0) {
					matrizFragmentacion[i][j] = (100 * (matrizObjetos[i][j] == 1) * (9 - matrizReferencias[i][j]) * (matrizTiempo[i][j] + 1)) / 90;
				} else {
					matrizRecoleccion[i][j] = ( 50 * (matrizObjetos[i][j] == 2) * (1 + matrizTiempo[i][j]) * (1 + 9 - matrizRecoleccion[i][j]) ) / 100;
				}
				if(matrizReferencias[i][j] == 9) matrizReferencias[i][j] = 0;
				if(matrizFragmentacion[i][j] != 0 || matrizRecoleccion[i][j] != 0) matrizReferencias[i][j]++;
			}
		}
		raise(SIGSTOP);
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
