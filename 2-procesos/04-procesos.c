#include <stdlib.h>   // Para EXIT_SUCCESS, EXIT_FAILURE
#include <sys/types.h> // Para pid_t
#include <unistd.h>    // Para fork(), sleep()
#include <stdio.h>     // Para printf(), perror()
#include <sys/wait.h>  // Para waitpid(), WIFEXITED(), WEXITSTATUS()
#include <errno.h>     // Para manejar errores con errno
#include <signal.h>    // Para señales

// Manejador de señales que no hace nada, pero interrumpe la llamada al sistema
void signal_handler(int sig) {
    printf("Señal %d recibida, interrumpiendo waitpid()\n", sig);
}

int main(void) {
    // Establecer el manejador de la señal SIGALRM
    signal(SIGALRM, signal_handler);

    pid_t child = fork();

    if (child == -1) {
        perror("Error al hacer fork");
        return EXIT_FAILURE;
    }

    if (child == 0) {  // Proceso hijo
        sleep(5);  // Simula que el hijo está haciendo algo
        exit(EXIT_SUCCESS);
    } else {  // Proceso padre
        // Dispara una señal después de 2 segundos para interrumpir waitpid()
        alarm(2);

        int status;
        pid_t wpid = waitpid(child, &status, 0);

        if (wpid == -1) {
            if (errno == EINTR) {
                printf("waitpid() fue interrumpido y errno se estableció en EINTR (%d)\n", EINTR);
            } else {
                perror("Error en waitpid()");
            }
        } else {
            printf("Proceso hijo %d terminó correctamente\n", wpid);
        }
    }

    return EXIT_SUCCESS;
}

 