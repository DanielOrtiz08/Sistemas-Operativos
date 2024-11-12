#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <signal.h>

void error(char*);
size_t sizeShm(int, int, size_t);
void matrizIndex(void**, int, int, size_t);
int segmentoMatriz(int***, int, int, size_t);
void escanearMatriz(int**, int, int, FILE*);
bool enFaseDePropagacion(int**, int, int, int, int);
bool enFaseDeConsumo(int**, int, int);

int main(int argc, char **argv) {

	if(argc != 2) error("Se espera un argumento (nombre del archivo)\n");

	FILE *file = fopen(argv[1], "r");
	if(!file) error("Error al intentar abrir el archivo\n");

	int horas, nHijos, N, M;
	fscanf(file, "%d %d %d %d", &horas, &nHijos, &N, &M);
	printf("%d %d %d %d\n", horas, nHijos, N, M);

	size_t shmSize = sizeShm(N, M, sizeof(int));

	// matriz de incendio
	int **matrizIncendio;
	int shmIdIncendio = segmentoMatriz(&matrizIncendio, N, M, shmSize);
	escanearMatriz(matrizIncendio, N, M, file);

	// matriz con los estados de propagacion
	int **matrizPropagacion;
	int shmIdPropagacion = segmentoMatriz(&matrizPropagacion, N, M, shmSize);

	// matriz con los estado de consumo
	int **matrizConsumo;
	int shmIdConsumo = segmentoMatriz(&matrizConsumo, N, M, shmSize);

	fclose(file);

	pid_t root = getpid(), children[nHijos];
	int idx;
	for(idx = 0; idx < nHijos; idx++) {
		children[idx] = fork();
		if(children[idx] < 0 ) error("Error en fork\n");
		if(!children[idx]) break;
	}

	if(root == getpid()) {
		sleep(2); // esperamos que los hijos terminen el primer intervalo de tiempo y se detengan
		for(int k = 0; k < horas; k++) {


			int status;
			for(int i = 0; i < nHijos; i++) {
				pid_t result = waitpid(children[i], &status, WUNTRACED);
				if(result == -1) error("Error en waitpid\n");
			}

			printf("\nFase de propagacion\n");
			for(int i = 0; i < N; i++) {
				for(int j = 0; j < M; j++) {
					fscanf(file, "%d", &matrizPropagacion[i][j]);
					printf("%d  ", matrizPropagacion[i][j]);
				}
				printf("\n");
			}

			printf("\nFase de consumo\n");
			for(int i = 0; i < N; i++) {
				for(int j = 0; j < M; j++) {
					fscanf(file, "%d", &matrizConsumo[i][j]);
					printf("%d  ", matrizConsumo[i][j]);
				}
				printf("\n");
			}

			printf("\nMatriz de incendio en intervalo %d\n", k);
			for(int i = 0; i < N; i++) {
				for(int j = 0; j < M; j++) {
					if(matrizPropagacion[i][j] == 1)
						matrizIncendio[i][j] = 1;
					if(matrizConsumo[i][j] == 1) 
						matrizIncendio[i][j] = 2;
					printf("%d", matrizIncendio[i][j]);
				}
				printf("\n");
			}

			for(int i = 0; i < nHijos; i++) 
				kill(children[i], SIGCONT);
		}

		shmdt(matrizIncendio);
		shmctl(shmIdIncendio, IPC_RMID, NULL);
		shmdt(matrizPropagacion);
		shmctl(shmIdPropagacion, IPC_RMID, NULL);
		shmdt(matrizConsumo);
		shmctl(shmIdConsumo, IPC_RMID, NULL);
	} else {

		for(int k = 0; k < horas; k++) {
			for(int i = idx; i < N; i+=nHijos) {
				for(int j = 0; j < M; j++) {
					if(enFaseDePropagacion(matrizIncendio, i, j, N, M))
						matrizPropagacion[i][j] = 1;
					else
						matrizPropagacion[i][j] = 0;

					if(enFaseDeConsumo(matrizIncendio, i, j))
						matrizConsumo[i][j] = 1;
					else 
						matrizConsumo[i][j] = 0;
				}
			}
			raise(SIGSTOP);
		}

		shmdt(matrizPropagacion);
		shmdt(matrizConsumo);
		shmdt(matrizIncendio);
	}

	return EXIT_SUCCESS;
}

void error(char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

size_t sizeShm(int row, int col, size_t size) {
	return (size_t) row * sizeof(void*) + row * col * size; 
}

int segmentoMatriz(int*** matriz, int row, int col, size_t shmSize) {
	int shmId = shmget(IPC_PRIVATE, shmSize, IPC_CREAT | S_IRUSR | S_IWUSR);
	if(shmId == -1) error("Error al crear el segmento de memoria\n");
	*matriz = (int**) shmat(shmId, NULL, 0);
	matrizIndex((void**)*matriz, row, col, sizeof(int));
	return shmId;
}

void matrizIndex(void** matriz, int row, int col, size_t size) {
	matriz[0] = matriz + row; 
	for(int i = 1; i < row; i++) {
		matriz[i] = matriz[i-1] + (col * size);
	}
}

void escanearMatriz(int** matriz, int row, int col, FILE* file) {
	for(int i = 0; i < row; i++) {
		for(int j = 0; j < col; j++) {
			fscanf(file, "%d", &matriz[i][j]);
			printf("%d  ", matriz[i][j]);
		}
		printf("\n");
	}
}

bool enFaseDePropagacion(int **matriz, int row, int col, int nRows, int nCols) {
	if(matriz[row][col] != 0) return false;
	int cant = 0;
	for(int i = -1; i <= 1; i++) {
		for(int j = -1; j <= 1; j++) {
			if(i == 0 && j == 0) continue;
			int pi = row + i;
			int pj = col + j;
			if(pi < 0 || pi > nRows || pj < 0 || pj > nCols) continue;
			if(matriz[pi][pj] == 1) cant++;
		}
	}
	return (cant >= 2)? true: false;
}

bool enFaseDeConsumo(int **matriz, int row, int col) {
	if(matriz[row][col] != 1) return false;
	return true;
}