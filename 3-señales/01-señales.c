#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h> // Para argumentos variables

void error(char* str, ...);
void manejoStatus(int status);
void usoDeWnohang();
void usoDeWuntraced();
void usoDeWcontinued();

int main(int argc, char **argv) {

    if (argc < 2) error("Se requiere un argumento del 1 al 3, ejemplo: %s 3\n", argv[0]);

    int value;
    if (sscanf(argv[1], "%d", &value) != 1) error("El argumento proporcionado no es un número válido\n");

    switch (value) {
    case 1:
        usoDeWnohang();
        break;
    case 2:
        usoDeWuntraced();
        break;
    case 3:
        usoDeWcontinued();
        break;
    default:
        printf("Opción NO válida.\n");
        break;
    }

    return EXIT_SUCCESS;
}

void error(char* str, ...) {
    va_list args;
    va_start(args, str);
    vfprintf(stderr, str, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

void manejoStatus(int status) {
    if (WIFEXITED(status))
        printf("El proceso terminó normalmente con código de salida: %d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("El proceso fue terminado por una señal: %d\n", WTERMSIG(status));
    else if (WIFSTOPPED(status))
        printf("El proceso fue detenido por la señal: %d\n", WSTOPSIG(status));
    else if (WIFCONTINUED(status))
        printf("El proceso fue reanudado.\n");
}

void usoDeWnohang() {
    pid_t pid = fork();

    if (pid < 0) error("Error en fork\n");

    if (pid == 0) {
        printf("Proceso hijo ejecutándose...\n");
        sleep(5);
        printf("Proceso hijo finalizado.\n");
        //raise(SIGKILL);
        exit(EXIT_SUCCESS);
    } else {
        int status;
        printf("Esperando al proceso hijo con WNOHANG...\n");

        pid_t result = waitpid(pid, &status, WNOHANG);
        if (result == -1) 
            error("Error en waitpid\n");

        if (result == 0)
            printf("El proceso hijo sigue ejecutándose.\n");
        else if (result == pid)
            manejoStatus(status);

        result = waitpid(pid, &status, 0);        
        if (result == -1) error("Error en waitpid\n");
        manejoStatus(status);
    }

    exit(EXIT_SUCCESS);
}

void usoDeWuntraced() {
    pid_t pid = fork();

    if (pid < 0) error("Error en fork");

    if (pid == 0) {
        printf("Proceso hijo ejecutándose...\n");
        sleep(2);
        raise(SIGSTOP);
        printf("Proceso hijo reanudado.\n");
        //raise(SIGKILL);
        exit(EXIT_SUCCESS);
    } else {
        int status;
        printf("Esperando que el proceso hijo sea detenido...\n");

        pid_t result = waitpid(pid, &status, WUNTRACED);
        if (result == -1) error("Error en waitpid");

        if (WIFSTOPPED(status)) {
            printf("El proceso hijo ha sido detenido.\n");
            printf("Señal que causó la pausa [%d] y señal devuelta por WSTOPSIG [%d]\n", SIGSTOP, WSTOPSIG(status));
        }

        kill(pid, SIGCONT);
        result = waitpid(pid, &status, 0);        
        if (result == -1) error("Error en waitpid\n");
        manejoStatus(status);
    }

    exit(EXIT_SUCCESS);
}

void usoDeWcontinued() {
    pid_t pid = fork();

    if (pid < 0) error("Error en fork");

    if (pid == 0) {
        printf("Proceso hijo ejecutándose...\n");
        sleep(2);
        raise(SIGSTOP);
        sleep(2);
        printf("Proceso hijo reanudado y finalizando.\n");
        //raise(SIGKILL);
        exit(EXIT_SUCCESS);
    } else {
        int status;
        waitpid(pid, &status, WUNTRACED);

        if (WIFSTOPPED(status)) {
            printf("El proceso hijo ha sido detenido.\n");
            printf("Señal que causó la pausa [%d] y señal devuelta por WSTOPSIG [%d]\n", SIGSTOP, WSTOPSIG(status));
        }

        kill(pid, SIGCONT);
        printf("Reanudando proceso hijo...\n");

        waitpid(pid, &status, WCONTINUED);
        if (WIFCONTINUED(status)) printf("El proceso hijo ha sido reanudado.\n");

        pid_t result = waitpid(pid, &status, 0);        
        if (result == -1) error("Error en waitpid");
        manejoStatus(status);
    }

    exit(EXIT_SUCCESS);
}
