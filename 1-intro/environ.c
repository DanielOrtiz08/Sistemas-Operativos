
#include <stdio.h>   // Para funciones de entrada/salida como printf.
#include <stdlib.h>  // Para la función getenv.
#include <unistd.h>  // Para la variable environ.

extern char **environ; // Declaración externa para la lista de variables de entorno.

int main(void) {
    // Puntero para recorrer la lista de variables de entorno.
    char **list = environ;
    char *var;

    // Recorre la lista de variables de entorno y muestra su contenido.
    while (*list != NULL) {
        printf("%s\n", *list); // Imprime la variable de entorno actual.
        list++; // Avanza al siguiente elemento de la lista.
    }

    // Obtiene el valor de la variable de entorno "PATH".
    var = getenv("PATH");
    // Imprime el valor de la variable de entorno "PATH".
    printf("PATH = %s\n", var);

    // Obtiene el valor de la variable de entonno "LOGNAME"
    var = getenv("LOGNAME");
    printf("LOGNAME = %s\n", var);

    // Retorna EXIT_SUCCESS para indicar que el programa finalizó correctamente.
    return EXIT_SUCCESS;
}