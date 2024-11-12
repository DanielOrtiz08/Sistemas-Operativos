#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>

void error(const char*, ...);
size_t shmSize(int, int, size_t);


int main(int argc, char **argv) {

    if(argc != 2) error("Se requiere un argumento (nombre del archivo)\n");

    FILE *file = fopen(argv[1], "r");
    if(!file) error("No se pudo abrir el archivo\n");

    int sizeShm = shmSize()

    return EXIT_SUCCESS;
}

void error(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

size_t shmSize(int row, int col, size_t size) {
    return (size_t) row * sizeof(void*) + row * col * size;
}