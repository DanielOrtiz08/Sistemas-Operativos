#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdarg.h>

void error(const char *msg, ...);
size_t shmSize(int, int, size_t);
int** segmentoMatriz(int*, size_t);
void matrizIndex(void**, int, int, size_t);
bool enFaseDePropagacion(int**, int, int, int, int);
bool enFaseDeConsumo(int**, int, int);

int main(int argc, char** argv) {
	
	if(argc != 2) error("Se espera un argumento (nombre del archivo a leer)\n");

	FILE *file = fopen(argv[1], "r");

	int nHoras, nHijos, N, M;
	fscanf(file, "%d %d %d %d\n", &nHoras, &nHijos, &N, &M);

	if(nHijos > N*M || nHijos <= 0) error("El numero de hijos debe estar entre 1 y %d\n", (N*M));


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

	system("clear");

	if(root == getpid()) {
		for(int t = 0; t < nHoras; t++) {
			for(int i = 0; i < nHijos; i++) {
				int status;
				if(waitpid(-1, &status, WUNTRACED) == -1) error("Error en waipid\n");
				if(!WIFSTOPPED(status)) i--;
			}

			printf("\nCiclo 3\t");
			for(int k = 0; k <= t; k++) printf("*");

			printf("\n");

			for(int i = 0; i < N; i++) {
				for(int j = 0; j < M; j++) {
					if(matrizIncendio[i][j] == 0)
						printf("🌳 ");
					else if(matrizIncendio[i][j] == 1)
						printf("🔥 ");
					else
						printf("🌋 ");
					//printf("%d ", matrizIncendio[i][j]);

					if(matrizConsumo[i][j] == 2)
						matrizIncendio[i][j] = 2;
					if(matrizPropagacion[i][j] == 1)
						matrizIncendio[i][j] = 1;
				}
				printf("\n");
			}

			for(int i = 0; i < nHijos; i++) {
				kill(children[i], SIGCONT);
			}
			sleep(3);
			system("clear");
		}

	} else {

		for(int t = 0; t < nHoras; t++) {
			int minRow, minCol, incrementoRow, incrementoCol;
			//printf("entro hijo: %d\n", idx);
			if(nHijos%N == 0) {
				incrementoCol = nHijos/N; // 10

				minCol = idx/N; //para 7 minCol = 9

				minRow = idx%N; // minRow = 9
				incrementoRow = N; // 10

			} else if(nHijos%M == 0 || nHijos < M) {
				incrementoRow = (nHijos%M == 0)? nHijos/M: nHijos/M+1;

				minRow = idx/M;

				minCol = idx%M;
				incrementoCol = (minRow+1 == incrementoRow && nHijos != N*M)? nHijos%M: M;
			} else {
				incrementoCol = (nHijos/N)+1;

				minCol = idx/N;

				minRow = idx%N;
				incrementoRow = (minCol+1 == incrementoCol && nHijos != N*M)? nHijos%N: N;
			} 
			for(int row = minRow; row < N; row+=incrementoRow) {
				for(int col = minCol; col < M; col+=incrementoCol) {
					bool band = enFaseDePropagacion(matrizIncendio, row, col, N, M);
					matrizPropagacion[row][col] = band? 1 : 0;

					if(t != 0 && matrizConsumo[row][col] == 1) matrizConsumo[row][col] = 2;
					else {
						band = enFaseDeConsumo(matrizIncendio, row, col);
						matrizConsumo[row][col] = band? 1: 0;
					}
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

int** segmentoMatriz(int *shmId, size_t sizeShm) {
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

			if(pi < 0 || pi >= N || pj < 0 || pj >= M) continue;

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

/*
if(nHijos%N == 0 || nHijos < N) {
	int numSectoresCol = (nHijos%N == 0)? nHijos/N: (nHijos/N)+1;

	int sectorCol = idx/N;

	int minRow = idx%10;
	int incrementoRow = (sectorCol+1 == numSectoresCol)? nHijos%10: hHijos;

	int minCol = M * sectorCol / numSectoresCol;
	int maxCol = M * sectorCol+1 / numSectoresCol;
	for(int t = 0; t < nHoras; t++) {
		for(int row = minRow; row < N; row+=nHijos) {
			for(int col = minCol; col < maxCol; col++) {
				bool band = enFaseDePropagacion(matrizIncendio, row, col, N, M);
				matrizPropagacion[row][col] = band? 1 : 0;

				if(t != 0 && matrizConsumo[row][col] == 1) matrizConsumo[row][col] = 2;
				else {
					band = enFaseDeConsumo(matrizIncendio, row, col);
					matrizConsumo[row][col] = band? 1: 0;
				}
			}
		}
		raise(SIGSTOP);
	}
} else if(nHijos%M == 0 || nHijos < M) {
	int numSectoresRow = (nHijos%M == 0)? nHijos/M: nHijos+1;

	int sectorRow = idx/M;

	int minCol = idx%10;
	int incrementoCol = (sectorRow+1 == numSectoresRow)? nHijos%10: nHijos;

	for(int t = 0; t < nHoras; t++) {
		for(int row = sectorRow; row < N; row+=numSectoresRow) {
			for(int col = minCol; col < M; col+=nHijos) {
				bool band = enFaseDePropagacion(matrizIncendio, row, col, N, M);
				matrizPropagacion[row][col] = band? 1 : 0;

				if(t != 0 && matrizConsumo[row][col] == 1) matrizConsumo[row][col] = 2;
				else {
					band = enFaseDeConsumo(matrizIncendio, row, col);
					matrizConsumo[row][col] = band? 1: 0;
				}
			}
		}
		raise(SIGSTOP);
	}
} else {

}

*/


/*
if(nHijos%N == 0 || nHijos < N) {
	int numSectoresCol = (nHijos%N == 0)? nHijos/N: (nHijos/N)+1;

	int sectorCol = idx/N;

	int minRow = idx%10;
	int incrementoRow = (sectorCol+1 == numSectoresCol)? nHijos%10: hHijos;

	for(int t = 0; t < nHoras; t++) {
		for(int row = minRow; row < N; row+=nHijos) {
			for(int col = sectorCol; col < M; col+=numSectoresCol) {
				bool band = enFaseDePropagacion(matrizIncendio, row, col, N, M);
				matrizPropagacion[row][col] = band? 1 : 0;

				if(t != 0 && matrizConsumo[row][col] == 1) matrizConsumo[row][col] = 2;
				else {
					band = enFaseDeConsumo(matrizIncendio, row, col);
					matrizConsumo[row][col] = band? 1: 0;
				}
			}
		}
		raise(SIGSTOP);
	}
} else if(nHijos%M == 0 || nHijos < M) {
	int numSectoresRow = (nHijos%M == 0)? nHijos/M: nHijos+1;

	int sectorRow = idx/M;

	int minCol = idx%10;
	int incrementoCol = (sectorRow+1 == numSectoresRow)? nHijos%10: nHijos;

	for(int t = 0; t < nHoras; t++) {
		for(int row = sectorRow; row < N; row+=numSectoresRow) {
			for(int col = minCol; col < M; col+=nHijos) {
				bool band = enFaseDePropagacion(matrizIncendio, row, col, N, M);
				matrizPropagacion[row][col] = band? 1 : 0;

				if(t != 0 && matrizConsumo[row][col] == 1) matrizConsumo[row][col] = 2;
				else {
					band = enFaseDeConsumo(matrizIncendio, row, col);
					matrizConsumo[row][col] = band? 1: 0;
				}
			}
		}
		raise(SIGSTOP);
	}
} 
*/