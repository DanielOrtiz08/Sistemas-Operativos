#include <stdlib.h> // para EXIT_SUCCESS, EXIT_FAILURE, malloc(), calloc(), free(), atoi(), rand(), system()
#include <sys/types.h> // para pid_t, size_t, ssize_t, off_t, git_t, uid_t
#include <unistd.h> // para fork(), exec(), getpid(), sleep(), pipe(), getcwd(), chdir()
#include <stdio.h> // para perror, prinf(), scanf(), fopoen(), sprintf(), fclose()
#include <sys/wait.h> // para waitpid(), WIFEXITED(), WEXITSTATUS()


// Función para mostrar el árbol de procesos actual.
void showtree() {
    char cmd[50] = {""};
    sprintf(cmd, "pstree -cAlp %d\n", getpid());
    printf("cadena: %s", cmd);
    system(cmd);
}

int main(void) {
    pid_t pidchild = fork();

    switch (pidchild) {
    case -1:
        perror("Error fork\n");
        exit(EXIT_FAILURE);
    case 0:
        printf("Proceso hijo con id:%d y padre con id: %d \n", getpid(), getppid());
        pid_t nieto = fork();
        if(!nieto) {
            printf("Proceso hijo con id:%d y padre con id: %d \n", getpid(), getppid());
        }
        break;
    default:
        sleep(10);  // Espera un segundo para que se estabilice la creación de procesos.
        showtree();
        printf("Proceso padre con id: %d y padre con id:%d\n", getpid(), getppid());
        pid_t salida = wait(NULL);
        printf("salida: %d", salida);
        break;
    }

    return EXIT_SUCCESS;
}

// revisar p5.c que tiene el estado de salida de un proces (retorno de wait());
// revisar p6-2.c que tambien tiene el retorno de wait
