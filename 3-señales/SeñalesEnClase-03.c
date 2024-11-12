#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h> // para isdigit
#include <string.h> // para strlen
#include <stdbool.h> // para bool
#include <signal.h>
#include <sys/wait.h>
#include <stdarg.h>

void validaciones(int, char**, int*, int*);
void error(char*, ...);
bool esNumero(const char*);
int strToInt(const char*);
void showTree();
void handler();
void actuador(int, pid_t);

//-------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv) {

    int nHijos, nIteracioens;
    validaciones(argc, argv, &nHijos, &nIteracioens);
    
    pid_t *hijos = malloc(sizeof(pid_t) * nHijos),
          child,
          root = getpid();

    void *oldhanler = signal(SIGUSR1, handler);
    if(oldhanler == SIG_ERR)
        error("Error signal\n");

    int i;
    for (i = 0; i < nHijos; i++) {
        *(hijos+i) = fork();
        if(! *(hijos+i) ) {
            if(i == nHijos/2 || i == nHijos/2-1)
                child = fork();
            break;
        }
    }

    if(root == getpid()) { usleep(100); showTree(); }

    for (int j = 0; j < nIteracioens; j++) {
        if(root == getpid()) {
            printf("\nIteración %d\n", j+1);
            printf("Padre [%d]\n", getpid());
            actuador(SIGUSR1, *(hijos+nHijos-1));
            pause();
            printf("Padre [%d]\n", getpid());
        } else {
            pause();
            if(i == nHijos/2 || i == nHijos/2-1) {
                if(!child) {
                    printf("Hijo%d1 [%d]\n", (i+1), getpid());
                    actuador(SIGUSR1, getppid());
                } else {
                    printf("Hijo%d [%d]\n", i+1, getpid());
                    actuador(SIGUSR1, child);
                    pause();
                    printf("Hijo%d [%d]\n", i+1, getpid());
                    actuador(SIGUSR1, *(hijos+i-1));
                }
            } else {
                if(i == 0) {
                    printf("Hijo1 [%d]\n", getpid());
                    actuador(SIGUSR1, getppid());
                } else {
                    printf("Hijo%d [%d]\n", i+1, getpid());
                    actuador(SIGUSR1, *(hijos+i-1));
                }
            }
        }
    }
    
    if(signal(SIGUSR1, oldhanler) == SIG_ERR)
        error("Error signal\n");
    
    if(root == getpid())
        for (int j = 0; j < nHijos; j++)
            wait(NULL);
    
    if(i == nHijos/2 || i == nHijos/2-1)
        wait(NULL);

    free(hijos);

    return EXIT_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------------

void validaciones(int argc, char **argv, int *nHijos, int *nIteraciones) {
    if(argc != 3 )
        error("Se requieren 2 argumentos.\nEjemplo: name nHijos(par) nIteraciones -> %s 6 9\n", *(argv));
    
    if(! esNumero( *(argv+1) ))
        error("Debe ingresar un numero valido para nHijos\n");
    if(! esNumero( *(argv+2) ))
        error("Debe ingresar un numero valido para nIteraciones\n");
    
    sscanf( *(argv+1), "%d", nHijos);

    *nIteraciones = strToInt( *(argv+2) );

    if( *nHijos % 2 != 0 )
        error("El numero debe ser par\n");
}

void error(char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

bool esNumero(const char *cadena) {
    for (int i = 0; i < strlen(cadena); i++)
        if(! isdigit( *(cadena+i) ))
            return false;

    return true;
}

int strToInt(const char* cadena){
    int n = 0;
    for (int i = 0; i < strlen(cadena); i++){

        char caracter = *(cadena+i);
        if (caracter < '0' || caracter > '9')
            break;

        n *= 10;
        n += caracter - '0';
    }
    return n;
}

void showTree() {
    char cmd[50];
    sprintf(cmd, "pstree -cAlp %d", getpid());
    system(cmd);
}

void handler() {}

void actuador(int signal, pid_t pid) {
    usleep(100);
    kill(pid, signal);
}
