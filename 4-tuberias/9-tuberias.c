#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h> // para isdigit
#include <stdarg.h>
#include <unistd.h>
#include <sys/wait.h>

void error(char *msg, ...);
bool esNumero(char*);
int strToInt(char*);

int main(int argc, char **argv) {

    if(argc != 3) error("Error, Se esperan 2 argumento 'test1.in' y 'numero de procesos hijos'\n");

    int nHijos;
    if(esNumero( *(argv+2) ))
        nHijos = strToInt( *(argv+2) );
    else
        error("Debe pasar un numero valido\n");

    pid_t padre = getpid();

    int fdIda[nHijos][2];
    int fdVuelta[nHijos][2];
    for (int i = 0; i < nHijos; i++) {
        pipe(fdIda[i]);
        pipe(fdVuelta[i]);
    }
    
    int idx;
    for (idx = 0; idx < nHijos; idx++) {
        pid_t pid = fork();
        if( pid < 0) error("Error en fork en la iteracion %d", idx);
        if(!pid) break;
    }

    if(padre == getpid()) {
        for (int i = 0; i < nHijos; i++) {
            close(fdIda[i][0]);
            close(fdVuelta[i][1]);
        }
        
        FILE *file = fopen(*(argv+1), "r");
        if(file == NULL) error("No se pudo abrir el archivo\n");
        int tam;
        if(fscanf(file, "%d", &tam) != 1) error("No se logro leer el tamaño del archivo\n");

        for (int i = 0; i < nHijos; i++) {
            int limInf = tam*i/nHijos;
            int limSup = (i != nHijos-1)? tam*(i+1)/nHijos : tam;
            int rango = limSup-limInf;
            
            write(fdIda[i][1], &rango, sizeof(int));

            int *datos = (int*)malloc( (rango)*sizeof(int) );
            for (int j = 0; j < rango; j++){
                fscanf(file, "%d", datos+j);
            }
            write(fdIda[i][1], datos, rango*sizeof(int));  
            free(datos);
        }

        fclose(file);

        for (int i = 0; i < nHijos; i++)
            close(fdIda[i][1]);
                
        for (int i = 0; i < nHijos; i++) {
            wait(NULL);
        }
        
        int sumaTotal = 0;
        for (int i = 0; i < nHijos; i++) {
            int suma;
            read(fdVuelta[i][0], &suma, sizeof(int));
            sumaTotal += suma;
        }
        
        printf("La suma total es: %d\n", sumaTotal);

        for (int i = 0; i < nHijos; i++)
            close(fdVuelta[i][0]);
        
    } else {
        for (int i = 0; i < nHijos; i++) {
            if(idx != i) {
                close(fdIda[i][1]);
                close(fdVuelta[i][0]);
            }
        }
        
        int rango;
        read(fdIda[idx][0], &rango, sizeof(int));

        int *datos = (int*)malloc(rango*sizeof(int));
        read(fdIda[idx][0], datos, rango*sizeof(int));

        close(fdIda[idx][0]);

        int suma = 0;
        for (int i = 0; i < rango; i++)
            suma += *(datos+i);

        write(fdVuelta[idx][1], &suma, sizeof(int));

        free(datos);

        close(fdVuelta[idx][1]);
    }
    
    return EXIT_SUCCESS;
}

void error(char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

bool esNumero(char *numeroStr) {
    int len = strlen(numeroStr);
    for (int i = 0; i < len; i++) {
        if(!isdigit( *(numeroStr+i) )) return false;
    }
    return true;
}

int strToInt(char *numeroStr) {
    int numero = 0;
    for (int i = 0; i < strlen(numeroStr); i++) {
        numero *= 10;
        numero += *(numeroStr+i) - '0';
    }
    return numero;
}