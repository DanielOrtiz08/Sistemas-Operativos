#include <stdio.h>
#include <stdlib.h>

int main(){
    #ifdef LINUX
    printf("Compilación para ubuntu\n");
    #else
    printf("Compilacion para otro OS\n");
    #endif

    return EXIT_SUCCESS;
}