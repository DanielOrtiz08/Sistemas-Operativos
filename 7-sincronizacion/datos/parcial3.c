#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

int nombres_archivos_en_directorio(char *ruta_carpeta);
void* funcionHilo(void*);

int nCasilleros = 4;
int nHilos = 4;

int nElementos = 1000;
char **archivos = NULL;

int grupo1[1000];
int grupo2[1000];
int grupo3[1000];
int grupo4[1000];

int sizes[4];
int elementosPorFila[4];
FILE *file[4];

pthread_barrier_t miBarrera1;

int main(int argc, char** argv) {

    pthread_barrier_init(&miBarrera1, NULL, nHilos);

    for (int i = 0; i < nCasilleros; i++)
        elementosPorFila[i] = 0;

    char *ruta_carpeta = argv[1];
    int pipe_fd = nombres_archivos_en_directorio(ruta_carpeta);

    size_t capacidad = 10;
    size_t cantidad = 0;

    archivos = (char **)malloc(capacidad * sizeof(char *));
    if (archivos == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    FILE *pipe_stream = fdopen(pipe_fd, "r");
    if (!pipe_stream) {
        perror("fdopen");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe_stream)) {
        buffer[strcspn(buffer, "\n")] = '\0';

        if (cantidad == capacidad) {
            capacidad *= 2;
            archivos = (char **)realloc(archivos, capacidad * sizeof(char *));
            if (archivos == NULL) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }

        archivos[cantidad] = strdup(buffer);
        if (archivos[cantidad] == NULL) {
            perror("strdup");
            exit(EXIT_FAILURE);
        }
        cantidad++;
    }
    fclose(pipe_stream);

    printf("Archivos en el directorio '%s':\n", ruta_carpeta);
    for (int i = 0; i < cantidad; i++) {
        printf("%s\n", archivos[i]);
        free(archivos[i]);
    }

    pthread_t hilos[nHilos];

    int ids[nHilos];
    for (int i = 0; i < nHilos; i++) {
        ids[i] = i;
        pthread_create(&hilos[i], NULL, funcionHilo, &ids[i]);
    }

    for (int i = 0; i < nHilos; i++) {
        pthread_join(hilos[i], NULL);
    }

    pthread_barrier_destroy(&miBarrera1);

    for (int i = 0; i < sizes[0]; i++) {
        printf("%d", grupo1[i]);
    }
    for (int i = 0; i < sizes[1]; i++) {
        printf("%d", grupo2[i]);
    }
    for (int i = 0; i < sizes[2]; i++) {
        printf("%d", grupo3[i]);
    }
    for (int i = 0; i < sizes[3]; i++) {
        printf("%d", grupo4[i]);
    }

    return EXIT_SUCCESS;
}

void* funcionHilo(void* arg) {
    int idx = *(int*)arg;

    file[idx] = fopen(archivos[idx], "r");

    int size;
    fscanf(file[idx], "%d", &size);
    sizes[idx] = size;
    printf("hilo %d abrio archivo %s con nElementos %d\n", idx, archivos[idx], size);
    int valor;
    for(int i = 0; i < size; i++) {
        fscanf(file[idx], "%d", &valor);
        if(valor < 255) {
            grupo1[elementosPorFila[0]] = valor;
            elementosPorFila[0]++;
        } else if (valor < 500) {
            grupo2[elementosPorFila[1]] = valor;
            elementosPorFila[1]++;
        } else if (valor < 750) {
            grupo3[elementosPorFila[2]] = valor;
            elementosPorFila[2]++;
        } else {
            grupo4[elementosPorFila[3]] = valor;
            elementosPorFila[3]++;
        }
    }
    fclose(file[idx]);
    pthread_barrier_wait(&miBarrera1);

    for (int i = 0; i < nElementos; i++) {
        for (int j = 0; j < nElementos; j++) {
            if(idx == 0) {
                if(grupo1[j] < grupo1[i]) {
                    int aux = grupo1[i];
                    grupo1[i] = grupo1[j];
                    grupo1[j] = aux;
                }
            } else if(idx == 1) {
                if(grupo2[j] < grupo2[i]) {
                    int aux = grupo2[i];
                    grupo2[i] = grupo2[j];
                    grupo2[j] = aux;
                }
            } else if(idx == 2) {
                if(grupo3[j] < grupo3[i]) {
                    int aux = grupo3[i];
                    grupo3[i] = grupo3[j];
                    grupo3[j] = aux;
                }
            } else if(idx == 3) {
                if(grupo4[j] < grupo4[i]) {
                    int aux = grupo4[i];
                    grupo4[i] = grupo4[j];
                    grupo4[j] = aux;
                }
            }
        }
    }

}

int nombres_archivos_en_directorio(char *ruta_carpeta) {
    int pipe_fd[2];
    char comando[256];
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE); 
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork"); 
        exit(EXIT_FAILURE);
    } 
    if (pid == 0) {
        close(pipe_fd[0]);        
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);        
        snprintf(comando, sizeof(comando), "ls %s", ruta_carpeta);
        execl("/bin/sh", "sh", "-c", comando, NULL);        
        perror("execl");
        exit(EXIT_FAILURE);
    } else {  
        close(pipe_fd[1]);        
        return pipe_fd[0];
    }
}


