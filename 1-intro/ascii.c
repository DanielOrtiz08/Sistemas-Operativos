#include <stdlib.h>
#include <stdio.h>

int main(void) {
    // A = 65, Z = 90, a = 97, z = 122
    for (int i = 0; i < 127; i++) {
        printf("%c ", 'z');
    }
    
    return EXIT_SUCCESS;
}