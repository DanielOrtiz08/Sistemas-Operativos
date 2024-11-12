#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>

void error(const char *msg, ...);
size_t shmSize(int, int, size_t);
char** segmentoMatriz(int*, int);
void matrizIndex(void**, int, int, size_t);
void escanearMatriz(char**, int, int, FILE*);

int main(int argc, char **argv) {

	if(argc != 2) error("Error, se espera un argumento\n");

	FILE *file = fopen(argv[1], "r");

    int nSecuencias, nBases;
    fscanf(file, "%d %d", &nSecuencias, &nBases);

	size_t sizeShm = shmSize(nBases, nSecuencias, sizeof(char));

    int shmId;
    char **matriz = segmentoMatriz(&shmId, sizeShm);

    matrizIndex((void**)matriz, nBases, nSecuencias, sizeof(char));

    escanearMatriz(matriz, nBases, nSecuencias, file);

	int nHijos = nBases*(nBases-1) / 2;
	// el numero de pipes y numero de estudios tambien sera igual nHijos

	int fd[nHijos][2];
	for (int i = 0; i < nHijos; i++){
		pipe(fd[i]);
	}

	pid_t root = getpid(), children[nHijos];

	int idx;
	for(idx = 0; idx < nHijos; idx++) {
		children[idx] = fork();
		if(children[idx] < 0) error("Error en fork\n");
		if(!children[idx]) break;
	}

	if(root == getpid()) {
		for (int i = 0; i < nHijos; i++) fclose(fd[i][1]);

		
		
	} else {
		for (int i = 0; i < nHijos; i++) {
			fclose(fd[i][0]);
			if(idx != i) fclose(fd[i][1]); 
		}
		int contador = 0;
		for (int i = 0; i < nBases-1; i++){
			for (int j = i+1; j < nBases; j++) {
				if(contador == idx) {
					for(int k = 0; k < nSecuencias; k++) {
						if(matriz[i][k] != matriz[j][k]) {
							char diferencia[20];
							//sprintf(diferencia, "pos %d: %c %c", k, matriz[i][k], matriz[j][k]);
							// asegura que no haya desvordamiemto verficando que no supere el tamaño
							snprintf(diferencia, sizeof(diferencia), "pos %d: %c %c", k, matriz[i][k], matriz[j][k]);
							write(fd[idx][1], diferencia, strlen(diferencia));
						}
					}
					break;
				}
				contador++;
			}
			if(contador == idx) break;
		}


		/*
		// nBase = 4
		// idx = 0, filaLocal = 0, filaVisitante = 1;
		// idx = 1, filaLocal = 0, filaVisitante = 2;
		// idx = 2, filaLocal = 0, filaVisitante = 3;
		// **********************************************
		// idx = 3, filaLocal = 1, filaVisitante = 2;
		// idx = 4, filaLocal = 1, filaVisitante = 3;
		// **********************************************
		// idx = 5, filaLocal = 2, filaVisitante = 3;

		int fila1 = 0;
		int acumulado = 0;
		// para idx = 0, condicion = 0 + 4 - 0 - 1 = 3, no entra
		// para idx = 1, condicion = 0 + 4 - 0 - 1 = 3, no entra
		// para idx = 2, condicion = 0 + 4 - 0 - 1 = 3, no entra
		// para idx = 3, condicion = 0 + 4 - 0 - 1 = 3, si entra, acumulado = 0 + 4 - 0 - 1 = 3, fila1 = 1, condicion = 3 + 4 - 1 -1 = 5
		// para idx = 4, condicion = 0 + 4 - 0 - 1 = 3, si entra
		// para idx = 5, condicion = 0 + 4 - 0 - 1 = 3, si entra, acumulado = 0 + 4 - 0 - 1 = 3, fila1 = 1, condicion = 3 + 4 - 1 -1 = 5, si entra, acumulado = 3 + 4 - 1 - 1 = 5, fila = 2, condicion = 5 + 4 - 2 - 1 = 6
		while (idx >= acumulado + (nBases - fila1 - 1)) {
			acumulado += nBases - fila1 - 1;
			fila1++;
		}
		// para idx = 0, fila2 = 0 + 1 + 0 - 0 = 1
		// para idx = 1, fila2 = 0 + 1 + 1 - 0 = 2
		// para idx = 2, fila2 = 0 + 1 + 2 - 0 = 3
		// para idx = 3, fila2 = 0 + 1 + 0 - 0
		// para idx = 4, fila2 = 0 + 1 + 0 - 0
		// para idx = 5, fila2 = 0 + 1 + 0 - 0
		int fila2 = fila1 + 1 + (idx - acumulado);

		for (int k = 0; k < nSecuencias; k++) {
			if (matriz[fila1][k] != matriz[fila2][k]) {
				char diferencia[30]; // Ajuste de tamaño del buffer
				snprintf(diferencia, sizeof(diferencia), "Pos %d: %c %c\n", k, matriz[fila1][k], matriz[fila2][k]);
				write(fd[idx][1], diferencia, strlen(diferencia));
			}
		}
		*/

		
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

char** segmentoMatriz(int *shmId, int sizeShm) {
	*shmId = shmget(IPC_PRIVATE, sizeShm, IPC_CREAT | S_IRUSR | S_IWUSR);
	if(*shmId == -1) error("Error al crear el segmento de memoria compartida\n");

	char **matriz = (char**) shmat(*shmId, NULL, 0);
	if(matriz == (void*)-1) error("No se pudo acoplar la matriz al segmento de memoria\n");
	return matriz;
}

void matrizIndex(void **matriz, int N, int M, size_t size) {
	matriz[0] = matriz + N;
	for(int i = 1; i < N; i++) {
		matriz[i] = matriz[i-1] + M * size;
	}
}

void escanearMatriz(char** matriz, int row, int col, FILE* file) {
    printf("%d, %d\n", row, col);
	for(int i = 0; i < row; i++) {
		for(int j = 0; j < col; j++) {
            fscanf(file, "%1c", &matriz[i][j]);
			printf("%1c", matriz[i][j]);
		}
		//printf("\n");
	}
}

// nBase = 5
		// idx = 0, filaLocal = 0, filaVisitante = 1;
		// idx = 1, filaLocal = 0, filaVisitante = 2;
		// idx = 2, filaLocal = 0, filaVisitante = 3;
		// idx = 3, filaLocal = 0, filaVisitante = 4;
		// **********************************************
		// idx = 4, filaLocal = 1, filaVisitante = 2;
		// idx = 5, filaLocal = 1, filaVisitante = 3;
		// idx = 6, filaLocal = 1, filaVisitante = 4;
		// **********************************************
		// idx = 7, filaLocal = 2, filaVisitante = 3;
		// idx = 8, filaLocal = 2, filaVisitante = 4;
		// **********************************************
		// idx = 9, filaLocal = 3, filaVisitante = 4;

/*

		int filaLocal = 0;
		int filaVisitante = 0;
		int idx_actual = idx;

		

		for (filaLocal = 0; filaLocal < nBases - 1; filaLocal++) {
			int paresRestantes = nBases - filaLocal - 1;
			if (idx_actual < paresRestantes) {
				filaVisitante = filaLocal + 1 + idx_actual;
				break;
			}
			idx_actual -= paresRestantes;
		}
*/
