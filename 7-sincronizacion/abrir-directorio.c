#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

// Prototipo de la función
int nombres_archivos_en_directorio(char *ruta_carpeta);

int main() {
    char *ruta_carpeta = ".";
    int pipe_fd = nombres_archivos_en_directorio(ruta_carpeta);

    char **archivos = NULL;
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
    for (size_t i = 0; i < cantidad; i++) {
        printf("%s\n", archivos[i]);
        free(archivos[i]);
    }

    free(archivos);
    return 0;
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
