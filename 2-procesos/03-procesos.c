#include <stdio.h>    // Para funciones de entrada/salida como printf.
#include <stdlib.h>   // Para la macro EXIT_SUCCESS y la función system.
#include <sys/types.h> // Para tipos de datos como pid_t.
#include <sys/wait.h> // Para la función wait.
#include <unistd.h>   // Para las funciones fork y getpid.

// Función que muestra el árbol de procesos actual utilizando el comando pstree.
void showtree() {
    char cmd[20] = {""};
    // Construye el comando pstree con el PID del proceso actual.
    sprintf(cmd, "pstree -cAlp %d", getpid());
    // Ejecuta el comando para mostrar el árbol de procesos.
    system(cmd);	
}

int main(void) {
    int i;
    pid_t root = getpid(), out; // Almacena el PID del proceso raíz y el PID del hijo que termina.
    
    // Crea un proceso hijo.
    if (!fork()) {
        // El hijo crea tres procesos hijos adicionales.
        for (i = 0; i < 3; i++) {
            if (!fork()) {
                break; // Si es un proceso hijo, sale del bucle.
            }
        }
        
        // El hijo más interno espera a que sus hijos terminen y luego imprime sus PIDs.
        if (i == 3) {
            for (int j = 0; j < 3; j++) {
                out = wait(NULL); // Espera a que un hijo termine.
                printf("p:%d-w:%d\n", getpid(), out); // Imprime el PID del proceso y del hijo que terminó.
            }
        }
    } else { 
        // El proceso raíz espera un segundo para que los hijos puedan iniciar.
        sleep(1);
        // Muestra el árbol de procesos.
        showtree();
        // Espera a que el primer hijo termine y luego imprime los PIDs.
        out = wait(NULL);
        printf("p:%d-w:%d\n", getpid(), out);
    }
    
    // Espera dos segundos antes de terminar para asegurarse de que todos los procesos puedan imprimir su salida.
    sleep(2);
    return EXIT_SUCCESS; // Indica que el programa terminó exitosamente.
}