#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    int n;

    do {
        printf("Entrada de n: ");
        scanf("%d", &n);
        if (n < 1 || n > 80)
            printf("El valor de n debe ser mayor o igual a 1 y menor o igual a 80\n");
    } while (n < 1 || n > 80);

    // Configuración de estilos y colores para cada anillo
    int estilos[] = {0, 1, 4};  // 0: Normal, 1: Negrita, 4: Subrayado, 5: Parpadeo lento, 7: Inverso, 8: Oculto

    int coloresFondo[] = { 40, 41, 42, 43, 44, 45, 46, 47, 48 };

    int maxi = n * 2 - 1;
    printf("\n");

    // Impresión de los números en forma de anillos con colores
    for (int i = 0; i < maxi; i++) {
        for (int j = 0; j < maxi; j++) {
            for (int k = 0; k < n; k++) {
                if (i == k || i == maxi - (k + 1) || j == k || j == maxi - (k + 1)) {
                    int estilo = estilos[k % (sizeof(estilos) / sizeof(estilos[0]))];
                    int colorFondo = coloresFondo[k % (sizeof(coloresFondo) / sizeof(coloresFondo[0]))];

                    printf("\033[%d;%dm", estilo, colorFondo);
                    printf(" %d%s", n-k, (n - k) / 10 == 0? "  " : " ");
                    printf("\033[0m");
                    break;
                }
            }
        }
        printf("\n");
    }

    printf("\n\n");
    return 0;
}
