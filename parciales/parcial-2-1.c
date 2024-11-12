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
bool enFaseDePropagacion(int**, int, int, int, int);
bool enFaseDeConsumo(int**, int, int);

int main(int argc, char **argv) {

	if(argc != 2) error("Se espera un argumento (nombre del archivo a leer)\n");

	FILE *file = fopen(argv[1], "r");

	int nHoras, nHijos, N, M;
	fscanf(file, "%d %d %d %d", &nHoras, &nHijos, &N, &M);


	size_t sizeShm = shmSize(N, M, sizeof(int));

	// matriz de incendio
	int shmIdIncendio = -1;
	int **matrizIncendio = segmentoMatriz(&shmIdIncendio, sizeShm);
	matrizIndex((void**)matrizIncendio, N, M, sizeof(int));
	
	for(int i = 0; i < N; i++) {
		for(int j = 0; j < M; j++) {
			fscanf(file, "%d", &matrizIncendio[i][j]);
		}
	}

	fclose(file);

	// matrices de estado
	int shmIdPropagacion = -1;
	int **matrizPropagacion = segmentoMatriz(&shmIdPropagacion, sizeShm);
	matrizIndex((void**)matrizPropagacion, N, M, sizeof(int));

	int shmIdConsumo = -1;
	int **matrizConsumo = segmentoMatriz(&shmIdConsumo, sizeShm);
	matrizIndex((void**)matrizConsumo, N, M, sizeof(int));

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
			}

			for(int k = 0; k < t; k++) printf("-");

			printf("\nEstado del bosque en el ciclo %d\n", t);

			for(int i = 0; i < N; i++) {
				for(int j = 0; j < M; j++) {
					printf("%d ", matrizIncendio[i][j]);

					if(matrizConsumo[i][j] == 1)
						matrizIncendio[i][j] = 2;
					if(matrizPropagacion[i][j] == 1)
						matrizIncendio[i][j] = 1;
				}
				printf("\n");
			}

			for(int i = 0; i < nHijos; i++) {
				kill(children[i], SIGCONT);
			}
			sleep(2);
			//system("clear");
		}

	} else {
		for(int t = 0; t < nHoras; t++) {
			for(int row = idx; row < N; row+=nHijos) {
				for(int col = 0; col < M; col++) {
					bool band = enFaseDePropagacion(matrizIncendio, row, col, N, M);
					matrizPropagacion[row][col] = band? 1 : 0;

					band = enFaseDeConsumo(matrizIncendio, row, col);
					matrizConsumo[row][col] = band? 1: 0;
				}
			}
			raise(SIGSTOP);
		}
	}

	shmdt(matrizIncendio);
	shmdt(matrizPropagacion);
	shmdt(matrizConsumo);
	if(root == getpid()) {
		shmctl(shmIdIncendio, IPC_RMID, NULL);
		shmctl(shmIdPropagacion, IPC_RMID, NULL);
		shmctl(shmIdConsumo, IPC_RMID, NULL);
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

bool enFaseDePropagacion(int **matriz, int row, int col, int N, int M) {
	if(matriz[row][col] != 0) return false;
	int nVecinosQuemandose = 0;
	for(int i = -1; i <= 1; i++) {
		for(int j = -1; j <= 1; j++) {
			if(i == 0 && j == 0) continue;

			int pi = row + i; int pj = col + j;

			if(pi < 0 || pi >= N || pj < 0 || pj > M) continue;

			if(matriz[pi][pj] == 1) nVecinosQuemandose++;
			if(nVecinosQuemandose >= 2) return true;
		}
	}
	return false;
}

bool enFaseDeConsumo(int **matriz, int row, int col) {
	if(matriz[row][col] != 1) return false;
	else return true;
}