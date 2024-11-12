#include <stdio.h>      // Para funciones de entrada/salida como printf.k
#include <sys/wait.h>   // Para la función wait.
#include <signal.h>
#include <unistd.h>  

void manejador() {
    printf("Señal\n");
}

int main() {
    
    if(fork()) {
        signal(SIGUSR1, manejador);
        pause();
        printf("Soy el padre\n");
        wait(NULL);
    } else {
        printf("Soy el hijo\n");
        kill(getppid(), SIGUSR1);
    }
    return 0;
}
