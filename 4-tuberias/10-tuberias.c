#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

void error(char*, ...);
int contarLineasNoVacias(char*, int);
int contarKeyword(char*, int);
bool isKeyword(char*);
int contarComentarios(char*, int);

int main(int argc, char **argv) {

    if(argc != 2) error("Error, se requiere el nombre del archivo como argumento\n");
    
    int nHijos = 3;

    int fdIda[2];
    pipe(fdIda);
    int fdVuelta[3][2];
    for (int i = 0; i < 3; i++)
        pipe(fdVuelta[i]);

    pid_t padre = getpid(), children[3];

    int idx;
    for (idx = 0; idx < nHijos; idx++) {
        children[idx] = fork();
        if(children[idx] < 0) error("Error en fork en la iteracion %d", idx);
        if(!children[idx]) break;
    }
     
    if(padre == getpid()) {
        close(fdIda[0]);
        for (int i = 0; i < 3; i++)
            close(fdVuelta[i][1]);        

        FILE *file = fopen( argv[1], "r");
        if(file == NULL) error("Error al abrir el archivo\n");

        int tamaño;
        fseek(file, 0, SEEK_END); // lleva el cursor a la ultima posicion
        tamaño = ftell(file); // retorna la poscion del curso, en este caso la ultima (numero de caracteres)
        fseek(file, 0, SEEK_SET); // devuelve el cursor a la primera poscion
        
        char *contenido = (char *)malloc( (tamaño+1) * sizeof(char) );
        if(contenido == NULL) {
            fclose(file);
            error("Error al asignar memoria\n");
        }

        fread(contenido, sizeof(char), tamaño, file); // lee todo el archivo y lo almacena en contenido se puede reemplazar con for y fscanf
        contenido[tamaño] = '\0';
        tamaño++;

        fclose(file);

        for (int i = 0; i < 3; i++) {
            write(fdIda[1], &tamaño, sizeof(int));
            write(fdIda[1], contenido, tamaño*sizeof(char));
            kill(children[i], SIGCONT); // para no usar SIGUSR y tener que crear un manejador
            usleep(100);
        }

        close(fdIda[1]);

        for (int i = 0; i < 3; i++) {
            wait(NULL);
            int cant = 0;
            read(fdVuelta[i][0], &cant, sizeof(int));
            printf("Conteo es igual a: %d\n", cant);
        }

        free(contenido);
        
    } else {
        close(fdIda[1]);
        for (int i = 0; i < 3; i++) {
            if(idx != i) close(fdVuelta[i][1]);
            close(fdVuelta[i][0]);
        }

        raise(SIGSTOP); // con SIGCONT no se puede usar pause(), en este caso el mismo hijo se llama la señal de pausa

        int tam;
        read(fdIda[0], &tam, sizeof(int));
        
        char *contenido = (char*)malloc(tam*sizeof(char));
        read(fdIda[0], contenido, tam*sizeof(char));

        close(fdIda[0]);

        int cant;
        if(idx == 0) // no estoy seguro de si funcionan correctamente los 3 metodos
            cant = contarLineasNoVacias(contenido, tam);
        else if(idx == 1)
            cant = contarKeyword(contenido, tam);
        else if(idx == 2)
            cant = contarComentarios(contenido, tam);

        write(fdVuelta[idx][1], &cant, sizeof(int));

        free(contenido);

        close(fdVuelta[idx][1]);
    }
    
    return EXIT_SUCCESS;
} 

void error(char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

int contarLineasNoVacias(char* codigo, int tam) {
    int cont = 0, charPorLinea = 0;

    for (int i = 0; i < tam; i++) {
        if(codigo[i] != '\n') {
            charPorLinea++;
        }
        else if (charPorLinea > 0) {
            cont++;
            charPorLinea = 0;
        }
    }
    if(charPorLinea > 0) {
        cont++;
    }
    return cont;
}

// A = 65, Z = 90, a = 97, z = 122
int contarKeyword(char *codigo, int tam) {
    char *token = (char *)malloc(50*sizeof(char));
    int j = 0, cont = 0;
    
    for (int i = 0; i < tam; i++) {
        if(codigo[i] >= 97 && codigo[i] <= 122) {
            token[j++] = codigo[i];
        } else if (j == 0) {
            continue;
        } else {
            token[j] = '\0';
            if(isKeyword(token)) cont++;
            j = 0;
        }
    }
    if(j > 0) {
        token[j] = '\0';
        if(isKeyword(token)) cont++;
    }
    free(token);
    return cont;
}   

bool isKeyword(char* palabra) {
    const char* keywords[] = {"int", "float", "return", "if", "else", "for", "while", "printf"};
    for (int i = 0; i < 8; i++)
        if(strcmp(palabra, keywords[i]) == 0) {
            return true; 
        }
    return false;
}

int contarComentarios(char* codigo, int tam) {
    int cont = 0;
    for (int i = 0; i < tam-1; i++) {
        if(codigo[i] == '/' && codigo[i+1] == '/') cont++;
    }
    return cont;
}

        //int tam = 0;
        //while (fgetc(file) != EOF)
        //    tam++;
        //fclose(file);
        //file = fopen( *(argv+1), "r");
        //if(file == NULL) error("Error al abrir el archivo\n");
        //for (int i = 0; i < tam; i++) {
        //    fscanf(file, "%c", contenido+i);
        //}