#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

void error(const char *msg, ...);

int main(int argc, char **argv) {

    int nHijos;
    printf("ingrese el numero de hijos: ");
    scanf("%d", &nHijos);
    getchar();

    pid_t root = getpid();

    int nPipes = nHijos + 1;
    
    int fd[nPipes][2];
    for (int i = 0; i < nPipes; i++) {
        if(pipe(fd[i]) == -1) error("Error al crear el pipe en la iteracion %d", i);
    }
    

    int idx;
    for (idx = 0; idx < nHijos; idx++) {
        pid_t pid = fork();
        if(pid < 0 ) error("Error en fork en la iteracion %d\n", idx);
        if(!pid) break;
    }

    if(root == getpid()) {
        for (int i = 0; i < nPipes; i++) {
            if(i != 0) close(fd[i][1]);
            if(i != nPipes-1) close(fd[i][0]);
        }

        char msg[1024];
        printf("ingrese el mensaje: ");
        fgets(msg, 1024, stdin);
        msg[strcspn(msg, "\n")] = 0;
        printf("mensaje escrito y enviado a los hijos:\n'%s'\n\n", msg);
        write(fd[0][1], msg, strlen(msg)+1);

        for (int i = 0; i < nHijos; i++)
            wait(NULL);

        read(fd[nPipes-1][0], msg, sizeof(msg)); // +1 para incluir '\0'

        printf("Mensaje final recibido de los hijos:\n'%s'\n", msg);
        
        close(fd[0][1]);
        close(fd[nPipes-1][0]);
    } else {
        for (int i = 0; i < nPipes; i++) {
            close(fd[0][1]);
            if(idx != i) {
                close(fd[i][0]);
                close(fd[i+1][1]);
            }
        }

        char mensaje[1024];
        read(fd[idx][0], mensaje, sizeof(mensaje));
        //printf("\nhijo %d ", idx);
        //printf("recibio mansaje:\n'%s' con longitud %ld\n", mensaje, strlen(mensaje));

        char aConcatenar[100];
        sprintf( aConcatenar, "\nHijo %d, con PID %d", idx+1, getpid() );
        
        //printf("a concatenar:\n'%s 'con longitud %ld\n", aConcatenar, strlen(aConcatenar));

        strcat(mensaje, aConcatenar);

        //printf("mensaje despues de concatenar:\n'%s' y longitud %ld\n", mensaje, strlen(mensaje));
        write(fd[idx+1][1], mensaje, strlen(mensaje)+1);
        
        close(fd[idx][0]);
        close(fd[idx+1][1]);
    }

    return EXIT_SUCCESS;
}

void error(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(EXIT_FAILURE);
}


    // Consumir el '\n' que queda en el búfer después de scanf
    //getchar();
    //char msg[100];
    //printf("ingrese el mensaje: ");
    //scanf("%99s", msg);
    //fgets(msg, 100, stdin);

    //char* mensaje = char(*)malloc(100*sizeof(char));
    //printf("ingrese el mensaje: ");
    //scanf("%99s", mensaje);
    //fgets(mensaje, 100, stdin);
    /*char *mensaje = NULL;
    size_t len = 0;
    getline(&mensaje, &len, stdin);

    printf("El mensaje ingresado es: %s", mensaje);


    free(mensaje);*/