#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void error(char*);
size_t shmSize(int, int, size_t);
void matrizIndex(void**, int, int, size_t);
void llenadoMatriz(int, int, int**, FILE*);
int segmentoMatriz(int, int, int***, size_t);
int segmentoMatrizDouble(int, int, double***, size_t);

int main(int argc, char **argv) {

	if(argc != 3) error("Se requieren 2 argumentos, (nombre del archivo) y (numero de intervalos de tiempo)\n");

	int t = atoi(argv[2]); // no entiendo para que sirve el intervalo de tiempo, si las 3 matrices iniciales no están cambiando en el tiempo
	// por ende siempre será la misma salida

	FILE *file = fopen(argv[1], "r");
	if(!file) error("No se pudo leer el archivo\n");

	int row, col;
	fscanf(file, "%d %d", &row, &col);

	// tamaño de los segmentos
	size_t sizeShm = shmSize(row, col, sizeof(int));

	// matriz de niveles
	int **matrizNiveles;
	int shmIdNiveles = segmentoMatriz(row, col, &matrizNiveles, sizeShm);

	// matriz de calidad
	int **matrizCalidad;
	int shmIdCalidad = segmentoMatriz(row, col, &matrizCalidad, sizeShm);
	
	// matriz de prediccion
	int **matrizPrediccion;
	int shmIdPrediccion = segmentoMatriz(row, col, &matrizPrediccion, sizeShm);

	// llenando las 3 matrices
	llenadoMatriz(row, col, matrizNiveles, file);
	printf("\n");
	llenadoMatriz(row, col, matrizCalidad, file);
	printf("\n");
	llenadoMatriz(row, col, matrizPrediccion, file);	
	printf("\n");
	printf("\n");


	fclose(file);

	size_t sizeShmR = shmSize(row, col, sizeof(double));
	// matriz de riesgo
	double **matrizRiesgo;
	int shmIdRiesgo = segmentoMatrizDouble(row, col, &matrizRiesgo, sizeShmR);

	// matriz de sostenibilidad
	double **matrizSostenibilidad;
	int shmIdSostenibilidad = segmentoMatrizDouble(row, col, &matrizSostenibilidad, sizeShmR);

	
	pid_t root = getpid(), nHijos = 2, children[2];
	int idx;
	for(idx = 0; idx < nHijos; idx++) {
		children[idx] = fork();
		if(children[idx] < 0) error("Error en fork\n");
		if(!children[idx]) break;
	}

	if(root == getpid()) {
		usleep(1000); // espera un rato que los hijos se detengan con SIGSTOP

		for(int k = 0; k < t; k++) {
			printf("\n\tInstante de tiempo t = %d", k);
			kill(children[0], SIGCONT); // reanuda los hijos
			kill(children[1], SIGCONT);
			printf("\nRiesgo\n");
			for(int i = 0; i < row; i++) {
				for(int j = 0; j < col; j++) {
					printf("%.3f\t", matrizRiesgo[i][j]);
				}
				printf("\n");
			}
			printf("\nSostenibilidad\n");
			for(int i = 0; i < row; i++) {
				for(int j = 0; j < col; j++) {
					printf("%.3f\t", matrizSostenibilidad[i][j]);
				}
				printf("\n");
			}

			printf("\nAlertas\n\n");

			for(int i = 0; i < row; i++) {
				for(int j = 0; j < col; j++) {
					if(matrizRiesgo[i][j] > 1.5 && matrizSostenibilidad[i][j] < 0.3)
						printf("%d  ", 2);
					else if(matrizRiesgo[i][j] > 1.2 && matrizSostenibilidad[i][j] < 0.4)
						printf("%d  ", 1);
					else
						printf("%d  ", 0);
				}
				printf("\n");
			}
			printf("\n\n");
		}


		shmdt(matrizNiveles);
		shmdt(matrizCalidad);
		shmdt(matrizPrediccion);
		shmdt(matrizRiesgo);
		shmdt(matrizSostenibilidad);
		shmctl(shmIdNiveles, IPC_RMID, NULL);
		shmctl(shmIdCalidad, IPC_RMID, NULL);
		shmctl(shmIdPrediccion, IPC_RMID, NULL);
		shmctl(shmIdRiesgo, IPC_RMID, NULL);
		shmctl(shmIdSostenibilidad, IPC_RMID, NULL);
	} else {

		for(int k = 0; k < t; k++) {
			raise(SIGSTOP);
			if(idx == 0) {
				for(int i = 0; i < row; i++) {
					for(int j = 0; j < col; j++) {
						// formula original dada en el documento
						//matrizRiesgo[i][j] =  100 * (100 - matrizNiveles[i][j]) * (2 - matrizCalidad[i][j]) * (1 - matrizPrediccion[i][j]);
						
						// pero creo que tiene mas sentido asi (probar ambas)
						matrizRiesgo[i][j] = (double) ((100 - matrizNiveles[i][j]) * (2 - matrizCalidad[i][j]) * (1 - matrizPrediccion[i][j])) / 100;
					}
				}
			} else if(idx == 1) {
				for(int i = 0; i < row; i++) {
					for(int j = 0; j < col; j++) {
						// formula original
						//matrizSostenibilidad[i][j] =  200 * matrizNiveles[i][j] * (matrizCalidad[i][j] + 1) * (matrizPrediccion[i][j] + 2);
						
						// pero tiene mas sentido asi
						matrizSostenibilidad[i][j] = (double) (matrizNiveles[i][j] * (matrizCalidad[i][j] + 1) * (matrizPrediccion[i][j] + 2)) / 200;
					}
				}
			}
		}

		shmdt(matrizNiveles);
		shmdt(matrizCalidad);
		shmdt(matrizPrediccion);
		shmdt(matrizRiesgo);
		shmdt(matrizSostenibilidad);
	}


	return EXIT_SUCCESS;
}

void error(char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

size_t shmSize(int row, int col, size_t size) {
	return (size_t) row * sizeof(void*) + row * col * size;
}

void matrizIndex(void** matriz, int row, int col, size_t size) {
	matriz[0] = matriz + row;
	for(int i = 1; i < row; i++) {
		matriz[i] = matriz[i-1] + (col * size);
	}
}

void llenadoMatriz(int row, int col, int** matriz, FILE* file) {
	for(int i = 0; i < row; i++) {
		for(int j = 0; j < col; j++) {
			fscanf(file, "%d", &matriz[i][j]);
			printf("%d  ", matriz[i][j]);
		}
		printf("\n");
	}
}

int segmentoMatriz(int row, int col, int*** matriz, size_t sizeShm) {
	int shmId = shmget(IPC_PRIVATE, sizeShm, IPC_CREAT | S_IRUSR | S_IWUSR);
	if(shmId == -1) error("No se pudo crear el segmento de memoria\n");

	*matriz = (int**) shmat(shmId, NULL, 0);
	if(matriz == (void*)-1) error("No se pudo acoplar la matriz de niveles al segmento de memoria\n");

	matrizIndex((void**)*matriz, row, col, sizeof(int));
	return shmId;
}

int segmentoMatrizDouble(int row, int col, double*** matriz, size_t sizeShm) {
	int shmId = shmget(IPC_PRIVATE, sizeShm, IPC_CREAT | S_IRUSR | S_IWUSR);
	if(shmId == -1) error("No se pudo crear el segmento de memoria\n");

	*matriz = (double**) shmat(shmId, NULL, 0);
	if(matriz == (void*)-1) error("No se pudo acoplar la matriz de niveles al segmento de memoria\n");

	matrizIndex((void**)*matriz, row, col, sizeof(double));
	return shmId;
}



