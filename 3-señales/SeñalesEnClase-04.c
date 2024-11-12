#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

void validaciones(int, char**, int*, int*);
void error(char*, ...);
bool esNumero(char*);
int strToInt(char*);
void handler();
void manejadorHijosDelMedio(pid_t*, pid_t[]);
void manejadorHijosDeAfuera(pid_t*, pid_t[]);
int jerarquiaProcesosHijosPar(int, pid_t*, pid_t, pid_t[]);
int jerarquiaProcesosHijosImpar(int, pid_t*, pid_t, pid_t[]);
void showTree();

int main(int argc, char **argv) {

    int nHijos, nIteraciones;
    validaciones(argc, argv, &nHijos, &nIteraciones);
    bool esPar = (nHijos % 2 == 0)? true: false;

    pid_t padre = getpid();
    pid_t *children = malloc(sizeof(pid_t) * nHijos);
    pid_t grandChild;
    pid_t greatGrandChild[2];

    void *oldHandler = signal(SIGUSR2, handler);
    if(oldHandler == SIG_ERR)
        error("Error signal\n");

    int i = (esPar)?
            jerarquiaProcesosHijosPar(nHijos, children, grandChild, greatGrandChild):
            jerarquiaProcesosHijosImpar(nHijos, children, grandChild, greatGrandChild);

    if(padre == getpid()) {
        usleep(100);
        showTree();
    }

    sleep(3);

    if(signal(SIGUSR2, oldHandler) == SIG_ERR)
        error("Error signal\n");

    free(children);

    return EXIT_SUCCESS;
}

void validaciones(int argc, char** argv, int* nHijos, int* nIteraciones) {
    if(argc != 3)
     error("Se requieren 2 argumentos.\nEjemplo: name nHijos nIteraciones -> %s 6 3\n", *(argv));

    if( !esNumero( *(argv+1) ))
        error("Debe ingresar un numero valido para nHijos\n");
    if( !esNumero( *(argv+2) ))
        error("Debe ingresar un numero valido para nIteraciones\n");

    *nHijos = strToInt( *(argv+1) );

    sscanf( *(argv+2), "%d", nIteraciones);
}

void error(char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

bool esNumero(char *numeroStr) {
    for (int i = 0; i < strlen(numeroStr); i++)
        if(! isdigit( *(numeroStr+i) )) return false;
    
    return true;
}

int strToInt(char *numeroStr) {
    int numeroInt = 0;
    for (int i = 0; i < strlen(numeroStr); i++) {
        if( *(numeroStr+i) >= '0' && *(numeroStr+i) <= '9' ) {
            numeroInt *= 10;
            numeroInt += *(numeroStr+i) - '0';
        } else break;
    }
    return numeroInt;
}

void handler(){}

void manejadorHijosDelMedio(pid_t *grandChild, pid_t greatGrandChild[]) {
    *grandChild = fork();
    if(*grandChild) return;

    for (int j = 0; j < 2; j++) {
        greatGrandChild[j] = fork();
        if(!greatGrandChild[j]) break;
    }
}

void manejadorHijosDeAfuera(pid_t *grandChild, pid_t greatGrandChild[]) {
    *grandChild = fork();
    if (!*grandChild) greatGrandChild[0] = fork(); 
}

int jerarquiaProcesosHijosPar(int nHijos, pid_t *children, pid_t grandChild, pid_t greatGrandChild[]) {
    int i;
    for (i = 0; i < nHijos; i++) {
        *(children+i) = fork();
        if( *(children+i) ) continue;

        if(nHijos/2 - 1 == i || nHijos/2 == i)
            manejadorHijosDelMedio(&grandChild, greatGrandChild);

        if((i % 2 == 0 && i < nHijos/2 - 1) || (i % 2 != 0 && i > nHijos/2) )
            manejadorHijosDeAfuera(&grandChild, greatGrandChild);

        break;
    }
    return i;
}

int jerarquiaProcesosHijosImpar(int nHijos, pid_t *children, pid_t grandChild, pid_t greatGrandChild[]) {
    int i;
    for (i = 0; i < nHijos; i++) {
        *(children+i) = fork();
        if( *(children+i) ) continue;
        
        if(nHijos/2 == i)
            manejadorHijosDelMedio(&grandChild, greatGrandChild);

        if(i % 2 == 0 && i != nHijos/2)
            manejadorHijosDeAfuera(&grandChild, greatGrandChild); 

        break;
    }
    return i;
}

/*
int jerarquiaProcesosHijosPar(int nHijos, pid_t *children, pid_t grandChild, pid_t greatGrandChild[]) {
    for (int i = 0; i < nHijos; i++) {
        *(children+i) = fork();
        if(! *(children+i) ) {
            if(nHijos/2 - 1 == i || nHijos/2 == i) {
                grandChild = fork();
                if(!grandChild) {
                    for (int j = 0; j < 2; j++) {
                        greatGrandChild[j] = fork();
                        if(!greatGrandChild[j]) break;
                    } 
                }
            }
            if((i % 2 == 0 && i < nHijos/2 - 1) || (i % 2 != 0 && i > nHijos/2) ) {
                grandChild = fork();
                if (!grandChild) greatGrandChild[0] = fork(); 
            }
            break;
        }
    }
}

int jerarquiaProcesosHijosImpar(int nHijos, pid_t *children, pid_t grandChild, pid_t greatGrandChild[]) {
    for (int i = 0; i < nHijos; i++) {
        *(children+i) = fork();
        if(! *(children+i) ) {
            if(nHijos/2 == i) {
                grandChild = fork();
                if(!grandChild) {
                    for (int j = 0; j < 2; j++) {
                        greatGrandChild[j] = fork();
                        if(!greatGrandChild[j]) break;
                    } 
                }
            }
            if(i % 2 == 0 && i != nHijos/2) {
                grandChild = fork();
                if (!grandChild) greatGrandChild[0] = fork(); 
            }
            break;
        }
    }
}
*/

void showTree() {
    char cmd[50];
    sprintf(cmd, "pstree -cAlp %d", getpid());
    system(cmd);
}
