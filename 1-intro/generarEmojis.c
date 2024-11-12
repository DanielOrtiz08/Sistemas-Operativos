#include <stdio.h>
#include <stdlib.h>

void imprimirEmoji(int codigoEmoji) {
    // Convertir el código Unicode a bytes UTF-8
    unsigned char utf8[5];
    utf8[0] = 0xF0 | ((codigoEmoji >> 18) & 0x07);
    utf8[1] = 0x80 | ((codigoEmoji >> 12) & 0x3F);
    utf8[2] = 0x80 | ((codigoEmoji >> 6) & 0x3F);
    utf8[3] = 0x80 | (codigoEmoji & 0x3F);
    utf8[4] = '\0'; // Terminador de cadena

    printf("%s ", utf8);
}

int main() {
    int inicio = 0x1D000; // Primer emoji en el rango (😀)

    for (int i = 0; i < 11300; i++) {
        imprimirEmoji(inicio + i);
    }
    printf("\n");

    return 0;
}
