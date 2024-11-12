#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>

void error(const char*, ...);
size_t shmSize(int, int, size_t);


int main(int argc, char **argv) {

    if(argc != 2) error("Se requiere un argumento (nombre del archivo)\n");

    FILE *file = fopen(argv[1], "r");
    if(!file) error("No se pudo abrir el archivo\n");

    
    fseek(file, 0, SEEK_END);
    int tamCodigo = ftell(file);
    fseek(file, 0, SEEK_SET);

    int shmId = shmget(IPC_PRIVATE, tamCodigo*sizeof(char), IPC_CREAT | S_IRUSR | S_IWUSR);
    if(shmId == -1) error("Error al crear el segmento de memoria compartida\n");

    int *code = (int*) shmat(shmId, NULL, 0);
    if(code == (void*)-1) error("Error al acoplar code al segmento de memoria compartida\n");

    int idx

    return EXIT_SUCCESS;
}

void error(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(EXIT_FAILURE);
}