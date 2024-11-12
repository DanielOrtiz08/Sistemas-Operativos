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
void sigHandler();
void manejadorHijosExternos(pid_t grandChild[]);/*
int jerarquiaProcesosHijosPar(int, pid_t*, pid_t[]);
int jerarquiaProcesosHijosImpar(int, pid_t*, pid_t[]);*/
int jerarquiaProcesosHijos(int, pid_t*, pid_t[], bool);
void showTree();

int main(int argc, char **argv) {

    int nHijos, nIteraciones;
    validaciones(argc, argv, &nHijos, &nIteraciones);
    bool esPar = (nHijos % 2 == 0)? true: false;

    pid_t padre = getpid();
    pid_t *children = malloc(sizeof(pid_t) * nHijos);
    pid_t grandChild[2];

    void *oldHandler = signal(SIGUSR1, sigHandler);
    if(oldHandler == SIG_ERR)
        error("Error signal\n");

    int i;
    /*i = (esPar)?
        jerarquiaProcesosHijosPar(nHijos, children, grandChild):
        jerarquiaProcesosHijosImpar(nHijos, children, grandChild);
    */
    
    i = jerarquiaProcesosHijos(nHijos, children, grandChild, esPar);
    if(padre == getpid()) { usleep(100); showTree(); }

    sleep(3);
    if(signal(SIGUSR1, oldHandler) == SIG_ERR)
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
        if(! isdigit( *(numeroStr+i) ))
            return false;
    
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

void sigHandler() {}

void manejadorHijosExternos(pid_t grandChild[]) {
    for (int j = 0; j < 2; j++) {
        grandChild[j] = fork();
        if(!grandChild[j]) break;;
    }
}
/*
int jerarquiaProcesosHijosPar(int nHijos, pid_t *children, pid_t grandChild[]) {
    int i;
    for (i = 0; i < nHijos; i++) {
        *(children+i) = fork();
        if( *(children+i) ) continue;
        
        if(nHijos/2 == i || nHijos/2 - 1 == i) grandChild[0] = fork();

        if( (i%2 != 0 && i < nHijos/2 - 1)  ||  (i%2 == 0 && i > nHijos/2) ) {
            manejadorHijosExternos(grandChild);
        }
        break;
    }
    return i;
}*/

int jerarquiaProcesosHijos(int nHijos, pid_t *children, pid_t grandChild[], bool esPar) {
    int i;
    for (i = 0; i < nHijos; i++) {
        *(children+i) = fork();
        if( *(children+i) ) continue;

        if(esPar && nHijos/2 - 1 == i) grandChild[0] = fork();
        if(nHijos/2 == i) grandChild[0] = fork();

        if ((esPar && ((i % 2 != 0 && i < nHijos / 2 - 1) || (i % 2 == 0 && i > nHijos / 2))) ||
            (!esPar && (i % 2 != 0 && i != nHijos/2))) { // espagueti
            manejadorHijosExternos(grandChild);
        }

        break;
    }
    return i;
}
/*
int jerarquiaProcesosHijosImpar(int nHijos, pid_t *children, pid_t grandChild[]) {
    int i;
    for (i = 0; i < nHijos; i++) {
        *(children+i) = fork();
        if( *(children+i) ) continue;

        if(nHijos/2 == i) grandChild[0] = fork();

        if( i%2 != 0 && nHijos/2 != i) {
            manejadorHijosExternos(grandChild);
        }
        break;
    }
    return i;
}
*/
void showTree() {
    char cmd[50];
    sprintf(cmd, "pstree -cAlp %d", getpid());
    system(cmd);
}