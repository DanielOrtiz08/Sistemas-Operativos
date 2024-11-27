#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int *ptr = malloc(sizeof(int)); // No se necesita cast
    if (ptr == NULL) { // Comprobar si malloc falló
        perror("Error al asignar memoria");
        return EXIT_FAILURE;
    }

    *ptr = 5; // Ahora es válido
    printf("su direccion %p, a donde apunta, %p, valor %d\n", (void*)&ptr, (void*)ptr, *ptr);

    free(ptr); // Liberar memoria dinámica
    return EXIT_SUCCESS;
}

// correr con 
//gcc -Wall -Werror estandar.c
