#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

void error(char*, ...);
void manejoStatus(int);
void usoDeWnohang(pid_t);
void usoDeWuntraced(pid_t);
void usoDeWcontinued(pid_t);

int main(int argc, char **argv) {
    
    if(argc < 2) 
        error("Se requiere un argumento del 1 al 3.\n");

    int value;
    if(sscanf(*(argv+1), "%d", &value) != 1) 
        error("El argumento proporcionado no es un número válido.\n");

    pid_t pid = fork();

    if (pid < 0) error("Error en fork\n");

    switch (value) {
    case 1:
        usoDeWnohang(pid);
        break;
    case 2:
        usoDeWuntraced(pid);
        break;
    case 3:
        usoDeWcontinued(pid);
        break;
    default:
        printf("Opción NO válida.\n");
        break;
    }

    return EXIT_SUCCESS;
}

void error(char *str, ...) {
    va_list args;
    va_start(args, str);
    vfprintf(stderr, str, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

void manejoStatus(int status) {
    if (WIFEXITED(status))
        printf("El proces0 terminó normalmente con código de salida: %d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("El proceso fue terminado por la señal: %d\n", WTERMSIG(status));
    else if (WIFSTOPPED(status))
        printf("El proceso fue detenido por la señal: %d\n", WSTOPSIG(status));
    else if (WIFCONTINUED(status))
        printf("El proceso fue reanudado. \n");
}

void usoDeWnohang(pid_t pid) {
    if (pid == 0) {
        printf("Proceso hijo ejecutándose...\n");
        sleep(5);
        printf("Proceso hijo finalizado\n");
        raise(SIGKILL);
    } else {
        printf("Esperando al proceso hijo con WNOHANG...\n");
        pid_t result; int status;
        for (int i = 0; i < 2; i++) {
            if(i == 0) 
                result = waitpid(pid, &status, WNOHANG);
            else
                result = waitpid(pid, &status, 0);

            if(result == -1)
                error("Error en waitpid\n");
            else if (result == 0)
                printf("El proces hijo sigue ejecutándose.\n");
            else if (result == pid)
                manejoStatus(status);
        }
    }
}

void usoDeWuntraced(pid_t pid) {
    if (pid == 0) {
        printf("Proceso hijo ejecutandose...\n");
        sleep(2);
        raise(SIGSTOP);
        printf("Proceso hijo reanudado.\n");
        //raise(SIGKILL)
    } else {
        int status;
        printf("Esperando que el proceso hijo sea detenido...\n");

        pid_t result = waitpid(pid, &status, WUNTRACED);
        if(result == -1)
            error("Error en waitpid\n");
        
        manejoStatus(status);

        kill(pid, SIGCONT);

        result = waitpid(pid, &status, 0);
        if(result == -1)
            error("Error en waitpid\n");
        
        manejoStatus(status);
    }
}

void usoDeWcontinued(pid_t pid) {
    if (pid == 0) {
        printf("Proceso hijo ejecutándose...\n");
        sleep(2);
        raise(SIGSTOP);
        sleep(2);
        printf("Proceso hijo reanudado y finalizado.\n");
        //raise(SIGKILL);
    } else {        

        int status;
        printf("Esperando que el proceso hijo sea detenido...\n");
        
        pid_t result = waitpid(pid, &status, WUNTRACED);
        if(result == -1)
            error("Error en waitpid\n");
        
        manejoStatus(status);

        kill(pid, SIGCONT);
        printf("Reanudando proceso hijo...\n");

        result = waitpid(pid, &status, WCONTINUED);
        if(result == -1)
            error("Error en waitpid\n");

        manejoStatus(status);
        
        result = waitpid(pid, &status, 0);
        if(result == -1)
            error("Error en waitpid\n");

        manejoStatus(status);
    }
}