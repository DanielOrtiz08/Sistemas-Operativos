/*
Archivo 'datos.txt'
1. Leer datos (numeros enteros) del archivo
2. Alojarlos en un vector
3. Se deben hacer operaciones en procesos
    - Estimar el número de valores impares en el conjunto
    - Estimar el número de valores pares en el conjunto
    - Calcular el promedio de los valores usando la fórmula (promedio)
    - Identificar la moda (numero que mas se repite en el conjunto)

Requerimientos:
1) La lectura de datos debe realizarse posterior a la creación de los procesos hijos (para que cada hijo no lo herede y tenga una copia que puede llegar a ser execivamente grande)
2) El proceso principal (padre) deberá asegurarse de que cada tarea sea ejecutada por un proceso hijo diferente
3) Tras la creación de los subprocesos, el proceso deberá enviar el conjunto de datos a cada hijo
4) El proceso principal deberá esperar hasta que los procesos hijos completen su tarea y comuniquen los resultado
5) La comunicacion entre el proceso padre e hijos deberá hacerse a través de tuberias (pipe)
6) Finalmente, el proceso principal es el encargado de consolidar y mostrar los resultados de todas las tareas
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdarg.h>


void error(char*);
int cantidadImpares(int*, int);
int cantidadPares(int*, int);
float promedio(int*, int);
int moda(int*, int);

void handler(int sig) {}

int main(int argc, char **argv) {


    signal(SIGUSR1, handler);
    pid_t root = getpid();

    int fd1[4][2]; // para pasar los datos y tamaño a los hijos
    int fd2[4][2]; // para pasar el resultado del hijo al padre 
    for (int i = 0; i < 4; i++) {
        if(pipe(fd1[i]) == -1 || pipe(fd2[i]) == -1) error("Error al crear la tuberia\n");
    }

    pid_t *children = (pid_t*)malloc(4 * sizeof(pid_t));
    if(children == NULL) error("No se pudo asignar memoria para children\n");

    int i;
    for (i = 0; i < 4; i++) {
        if ((children[i] = fork()) < 0) error("Error en fork\n");
        else if(!children[i]) break;
    }

    if(root == getpid()) {
        sleep(2);
        FILE *file = fopen(argv[1], "r");
        if(file == NULL) error("No se pudo abrir el archivo\n");

        for (int j = 0; j < 4; j++)
            close(fd1[j][0]);
    
        int sizeFile;
        if(fscanf(file, "%d", &sizeFile) != 1) {
            fclose(file);
            error("No se puedo leer el tamaño del archivo\n");
        }

        printf("sizeFile en root: %d\n", sizeFile);
        
        int *datos = (int *)malloc(sizeFile * sizeof(int));
        if(datos == NULL) error("No se pudo asignar memoria para datos\n");
        
        for (int j = 0; j < sizeFile; j++)
            fscanf(file, "%d", &datos[j]);
        
        fclose(file);

        for (int i = 0; i < 4; i++) {
            write(fd1[i][1], &sizeFile, sizeof(int));
            write(fd1[i][1], datos, sizeFile*sizeof(int));
            close(fd1[i][1]);
            kill(children[i], SIGUSR1);
            pause();
        }

        free(datos);

        for (int j = 0; j < 4; j++)
            close(fd2[j][1]);
        
        for (int i = 0; i < 4; i++) {
            int resultado;
            float promedio;
            if(i == 2)
                read(fd2[i][0], &promedio, sizeof(float));
            else
                read(fd2[i][0], &resultado, sizeof(int));
            if(i == 0)
                printf("El número de impares es %d\n", resultado);
            else if(i == 1)
                printf("El número de pares es %d\n", resultado);
            else if(i == 2)
                printf("El promedio es %.3f\n", promedio);
            else if(i == 3)
                printf("El número más repetido es %d\n", resultado);
        }

        for (int j = 1; j < 5; j++)
            close(fd2[j][0]);

    } else {

        for (int j = 0; j < 4; j++) {
            close(fd1[j][1]);
            if(i != j) close(fd1[j][0]);
        }
        
        pause();

        int sizeFile;
        read(fd1[i][0], &sizeFile, sizeof(int));

        int *datos = (int*)malloc(sizeFile*sizeof(int));
        if(datos == NULL) error("No se pudo asignar memoria para datos\n");
        read(fd1[i][0], datos, sizeFile*sizeof(int));
        
        kill(getppid(), SIGUSR1);

        close(fd1[i][0]);

        for (int j = 0; j < 4; j++){
            close(fd2[j][0]);
            if(i != j) close(fd2[j][1]);
        }

        if(i == 0) {
            int nImpares = cantidadImpares(datos, sizeFile);
            write(fd2[i][1], &nImpares, sizeof(int));
        }
        else if(i == 1) {
            int nPares = cantidadPares(datos, sizeFile);
            write(fd2[i][1], &nPares, sizeof(int));
        }
        else if(i == 2) {
            float media = promedio(datos, sizeFile);
            write(fd2[i][1], &media, sizeof(float));
        }
        else if(i == 3) {
            int masRepetio = moda(datos, sizeFile);
            write(fd2[i][1], &masRepetio, sizeof(int));
        }

        close(fd2[i][1]);
        free(datos);
        exit(EXIT_SUCCESS);
    }
    free(children);
    return EXIT_SUCCESS;
}

void error(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int cantidadImpares(int *datos, int tam) {
    int cant = 0;
    for (int i = 0; i < tam; i++) {
        if(datos[i] % 2 != 0) cant++;
    }
    return cant;
}

int cantidadPares(int *datos, int tam) {
    int cant = 0;
    for (int i = 0; i < tam; i++) {
        if(datos[i] % 2 == 0) cant++;
    }
    return cant;
}

float promedio(int *datos, int tam) {
    int suma = 0;
    for (int i = 0; i < tam; i++) {
        suma += datos[i];
    }
    return (float) suma/tam;
}

int moda(int *datos, int tam) {
    int masRepetido = datos[0], vecesMasRepetido = 0;
    for(int i = 0; i < tam; i++) {
        int vecesRepetido = 0;
        for (int j = 0; j < tam; j++) {
            if(datos[i] == datos[j]) {
                vecesRepetido++;
            }
        }
        if(vecesRepetido > vecesMasRepetido) {
            vecesMasRepetido = vecesRepetido;
            masRepetido = datos[i];
        }
    }
    return masRepetido;
}


