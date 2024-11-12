
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // para la funcion pipe y getpid
#include <string.h>
#include <wait.h>

int main(void) {
    int fd[2], n; // fd[0] será el extremo de lectura del pipe, fd[1] el de escritura.
    char Buffer_out[1024]; // Buffer para alamacena los datos a escribir en el pipe.
    char Buffer_in[1024]; // Buffer para leer los datos del pipe


    // Creacion de un pipe, y almacenamiento de los descriptores de archivos en fd
    pipe(fd);

    if(fork()) {
        char data[] = "Mensaje enviado por un pipe";
        close(fd[0]);
        sleep(3);
        strcpy(Buffer_out, data);
        /*
        argumentos 
        (int:descriptor de archivo, 
        void*: puntero que contiene los datos a escribir,
        size_t: el numero de bytes a escribir)
        retorna el numero de bytes escritos o -1 en caso de error
        */
        write(fd[1], Buffer_out, strlen(Buffer_out)); // EOL = '0'

        printf("[%d] write: --> %s\n", getpid(), Buffer_out);

        close(fd[1]);

        wait(NULL);
    } else {
        close(fd[1]);

        // Imprime un separador para indicar que el hijo está listo para leer.
        printf("--->\n");
        /*
        argumentos 
        (int:descriptor de archivo, 
        void*: puntero donde se almacenarán los datos leidos,
        size_t: número maximo de bytes a leer)
        retorna el numero de bytes leidos, 0 si llego al final del archivo(EOF) o -1 en caso de error
        */
        n = read(fd[0], Buffer_in, sizeof(Buffer_in) -1);
        // Asegura que el mensaje leido sea una cadena de texto válida
        Buffer_in[n] = '\0';

        printf("[%d] read: <-- %s\n", getpid(), Buffer_in);

        close(fd[0]);
    }
    
    return EXIT_SUCCESS;
}


/*
open(): Abre un archivo y devuelve un descriptor de archivo.
int fd = open("archivo.txt", O_RDONLY);  // Abre un archivo en modo lectura.

read(): Lee datos del archivo usando el descriptor de archivo.
char buffer[100];
ssize_t bytes_read = read(fd, buffer, sizeof(buffer));

write(): Escribe datos en el archivo.
const char *data = "Hello, World!";
ssize_t bytes_written = write(fd, data, strlen(data));

close(fd);
*/