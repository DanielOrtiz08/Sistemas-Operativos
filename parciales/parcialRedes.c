#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>

void error(char*);
size_t shmSize(int, int, size_t);
int** segmentoMatriz(int*, int);
void matrizIndex(void**, int, int, size_t);
void escanearMatriz(int**, int, int, FILE*);
bool transicionActivoSaturado(int**, int, int, int, int);
bool transicionSaturadoActivo(int**, int, int, int, int);
bool transicionActivoInactivo(int**, int, int, int, int);
bool transicionInactivoActivo(int**, int, int, int, int);
bool transicionSaturadoInactivo(int**, int, int, int, int);
     

int main(int argc, char **argv){
    if(argc != 2) error("Se espera un argumento (nombre del archivo a leer)\n");
	FILE *file = fopen(argv[1], "r");
	if(!file) error("error al abrir el archivo\n");

    int nHoras, N, M;
    fscanf(file, "%d %d %d", &nHoras, &N, &M);
    size_t sizeShm = shmSize(N, M, sizeof(int));

    int shmIdRed = -1;
	int **matrizRed = segmentoMatriz(&shmIdRed, sizeShm);
	matrizIndex((void**)matrizRed, N, M, sizeof(int));
	escanearMatriz(matrizRed, N, M, file);
    fclose(file);

    int shmIdRedAux = -1;
	int **matrizRedAux = segmentoMatriz(&shmIdRedAux, sizeShm);
	matrizIndex((void**)matrizRedAux, N, M, sizeof(int));

    int nHijos = 2;
    pid_t root = getpid(), children[nHijos];

    int idx;
	for(idx = 0; idx < nHijos; idx++) {
		children[idx] = fork();
		if(children[idx] < 0) error("Error en fork\n");
		if(!children[idx]) break;
	}

    if(root == getpid()) {
        for(int t = 0; t < nHoras; t++) {
			for(int i = 0; i < nHijos; i++) {
				int status;
				if(waitpid(-1, &status, WUNTRACED) == -1) error("Error en waipid\n");
				if(!WIFSTOPPED(status)) i--;
                //else printf("hijo detenido\n");
			}

			printf("\nEstado de la red en el ciclo %d\n", t);

			for(int i = 0; i < N; i++) {
                for(int j = 0; j < M; j++) {
                    printf("%d ", matrizRedAux[i][j]);
                }
                printf("\n");
            }

			for(int i = 0; i < nHijos; i++) {
				kill(children[i], SIGCONT);
                //printf("hijo continuado\n");
			}
			sleep(2);
		}
    }else{
        for(int t = 0; t < nHoras; t++) {
            //printf("Desde el hijo %d\n", idx);
            for(int i = 0; i < N; i++) {
                for(int j = 0; j < M; j++) {
					bool band;
                    if(idx == 0) {
                        band = transicionActivoSaturado(matrizRed, i, j, N, M);
                        if(band) { matrizRedAux[i][j] = 2; }
                        band = transicionSaturadoActivo(matrizRed, i, j, N, M);
                        if(band) {matrizRedAux[i][j] = 1;}
                    } else {
                        band = transicionActivoInactivo(matrizRed, i, j, N, M);
                        if(band) {matrizRedAux[i][j] = 0;}
                        band = transicionInactivoActivo(matrizRed, i, j, N, M);
                        if(band) {matrizRedAux[i][j] = 1;}
                        band = transicionSaturadoInactivo(matrizRed, i, j, N, M);
                        if(band) {matrizRedAux[i][j] = 0;}
                    }
                    
                }
            }

            raise(SIGSTOP); 
        }
    }

    shmdt(matrizRed);
	shmdt(matrizRedAux);
	if(root == getpid()) {
		shmctl(shmIdRed, IPC_RMID, NULL);
		shmctl(shmIdRedAux, IPC_RMID, NULL);
	}

    return EXIT_SUCCESS;
}

void error(char *msg) {
	perror(msg);
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

bool transicionActivoSaturado(int **matriz, int row, int col, int N, int M) {
    if(matriz[row][col] != 1) return false;
	int nVecinos = 0;
	for(int i = -1; i <= 1; i++) {
		for(int j = -1; j <= 1; j++) {
			if(i == 0 && j == 0) continue;

			int pi = row + i; int pj = col + j;

			if(pi < 0 || pi >= N || pj < 0 || pj > M) continue;

			if(matriz[pi][pj] == 1) nVecinos++;
			if(nVecinos >= 5) return true;
		}
	}
	return false;
}

bool transicionSaturadoActivo(int **matriz, int row, int col, int N, int M) {
    if(matriz[row][col] != 2) return false;
	int nVecinos = 0;
	for(int i = -1; i <= 1; i++) {
		for(int j = -1; j <= 1; j++) {
			if(i == 0 && j == 0) continue;

			int pi = row + i; int pj = col + j;

			if(pi < 0 || pi >= N || pj < 0 || pj > M) continue;

			if(matriz[pi][pj] == 1) nVecinos++;
		}
	}
    if(nVecinos < 4) return true;
	return false;
}

bool transicionActivoInactivo(int **matriz, int row, int col, int N, int M) {
    if(matriz[row][col] != 1) return false;
	int nVecinos = 0;
	for(int i = -1; i <= 1; i++) {
		for(int j = -1; j <= 1; j++) {
			if(i == 0 && j == 0) continue;

			int pi = row + i; int pj = col + j;

			if(pi < 0 || pi >= N || pj < 0 || pj > M) continue;

			if(matriz[pi][pj] != 1) nVecinos++;
			if(nVecinos >= 4) return true;
		}
	}
	return false;
}
bool transicionInactivoActivo(int **matriz, int row, int col, int N, int M) {
    if(matriz[row][col] != 0) return false;
	int nVecinos = 0;
	for(int i = -1; i <= 1; i++) {
		for(int j = -1; j <= 1; j++) {
			if(i == 0 && j == 0) continue;

			int pi = row + i; int pj = col + j;

			if(pi < 0 || pi >= N || pj < 0 || pj > M) continue;

			if(matriz[pi][pj] == 1) nVecinos++;
            //printf("[%d][%d], c = %d", row, col, nVecinos);
			if(nVecinos >= 5) return true;
		}
	}
	return false;
}

bool transicionSaturadoInactivo(int **matriz, int row, int col, int N, int M) {
    if(matriz[row][col] != 2) return false;
	int nVecinos = 0;
	for(int i = -1; i <= 1; i++) {
		for(int j = -1; j <= 1; j++) {
			if(i == 0 && j == 0) continue;

			int pi = row + i; int pj = col + j;

			if(pi < 0 || pi >= N || pj < 0 || pj > M) continue;

			if(matriz[pi][pj] == 0) nVecinos++;
			if(nVecinos >= 5) return true;
		}
	}
	return false;
}
                    