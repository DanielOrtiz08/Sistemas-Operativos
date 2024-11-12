
#include <stdio.h>      // Para funciones de entrada/salida como printf.
#include <stdlib.h>     // Para funciones de utilidad general como exit.
#include <sys/types.h>  // Para tipos de datos como pid_t.
#include <sys/wait.h>   // Para la función wait.
#include <unistd.h>     // Para la función fork y sleep.

// Función 'showtree' no definida. Debe ser proporcionada para que el código funcione.
void showtree() {
    char cmd[50] = {""};
    sprintf(cmd, "pstree -cAlp %d\n", getpid());
    system(cmd);
}

int main(void) {
    int i;
    pid_t root = getpid(); // Almacena el PID del proceso padre original.

    if (!fork()) { // El proceso padre crea un proceso hijo.
        printf("proceso con id: %d y padre con id: %d y grupo: %d\n", getpid(), getppid(), getpgrp());
        for (i = 0; i < 3; i++) {
            if (!fork()) { // El proceso hijo crea tres procesos hijos. no entra a la condicion
                printf("proceso con id: %d y padre con id: %d y grupo: %d\n", getpid(), getppid(), getpgrp());
                break; // Los procesos nietos no continúan el bucle.
            }
        }

        if (i == 3) { // Si es el proceso hijo original...
            for (int j = 0; j < 3; j++) {
                wait(NULL); // Espera a que terminen sus tres hijos.
            }
        }
        sleep(3); // sin esta linea pstree solo muestra los hijos del proceso que ejecuta pstree aunque ya hayan terminado, siguientes generaciones se pierden
    } else {
        printf("proceso con id: %d y padre con id: %d y grupo: %d\n", getpid(), getppid(), getpgrp());
        sleep(1); // El proceso padre espera un segundo.
        showtree(); // Muestra el árbol de procesos. Esta función debe ser definida.
        wait(NULL); // Espera a que termine el proceso hijo original.
    }

    return EXIT_SUCCESS; // Termina el proceso con éxito.
}