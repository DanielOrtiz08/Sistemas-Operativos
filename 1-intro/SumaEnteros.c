
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

double calcularVarianza(int[], int, double);
double calcularDesviacionEstandar(double);
double calcularMediana(int[], int);
int calcularModa(int[], int);
void intercambiar(int*, int*);
int particionar(int[], int, int);
void quickSort(int[], int, int);

int main(int argc, char *argv[]) {

    struct timespec start, end;

    clock_gettime(CLOCK_REALTIME, &start);

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("No se pudo abrir el archivo");
        return 1;
    } else {
        printf("Archivo encontrado [%s]\n", argv[1]);
    }

    int size;
    if (fscanf(file, "%d", &size) != 1) {
        fprintf(stderr, "No se pudo leer el tamaño del archivo\n");
        fclose(file);
        return 1;
    }

    int *array = (int *)malloc(size * sizeof(int));
    if (array == NULL) {
        perror("No se puedo asignar memoria");
        fclose(file);
        return 1;
    }

    long int suma = 0;
    for (int i = 0; i < size; i++) {
        fscanf(file, "%d", &array[i]);
        suma += array[i];
    }

    fclose(file);

    clock_gettime(CLOCK_REALTIME, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Tiempo transcurrido: %.6f segundos\n", elapsed);

    printf("Suma de los elementos: %ld\n", suma);

    // Calcular la media
    double media = (double) suma / size;
    printf("Media: %.2f\n", media);

    // Calculo de la varianza
    double varianza = calcularVarianza(array, size, media);
    printf("Varianza: %.2f\n", varianza);

    // Calculo de la Desviacion Estandar
    double desv_estand = calcularDesviacionEstandar(varianza);
    printf("Desviación estándar: %.2f\n", desv_estand);

    // para calculo la mediana, ordenamos el arreglo
    quickSort(array, 0, size - 1);

    // calculo de la mediana
    double mediana = calcularMediana(array, size);
    printf("Mediana: %.2f\n", mediana);

    // valor que mas se repite (moda)
    int moda = calcularModa(array, size);
    printf("Valor que más se repite (moda): %d\n", moda);

    free(array);

    return 0;
}

// FIN MAIN - COMIENZO FUNCIONES IMPLEMENTADAS ---------------------------------------

// Funcion de Varianza
double calcularVarianza(int array[], int size, double media) {
    double varianza = 0.0;
    for (int i = 0; i < size; i++) {
        varianza += pow(array[i] - media, 2);
    }
    return varianza / size;
}

// funcion de la Desviacion Estandar
double calcularDesviacionEstandar(double varianza) {
    return sqrt(varianza);
}

// funcion de la mediana
double calcularMediana(int array[], int size) {
    double mediana;
    if (size % 2 == 0) {
        mediana = (array[size / 2 - 1] + array[size / 2]) / 2.0;
    } else {
        mediana = array[size / 2];
    }
    return mediana;
}

// funcion de la moda
int calcularModa(int array[], int size) {
    int moda = array[0];
    int max_cont = 1, cont = 1;
    for (int i = 1; i < size; i++) {
        if (array[i] == array[i - 1]) {
            cont++;
        } else {
            if (cont > max_cont) {
                max_cont = cont;
                moda = array[i - 1];
            }
            cont = 1;
        }
    }
    if (cont > max_cont) {
        max_cont = cont;
        moda = array[size - 1];
    }
    return moda;
}

// Funciones para ordenamiento -----------------------------------------
void intercambiar(int *a, int *b) {
    int temporal = *a;
    *a = *b;
    *b = temporal;
}

int particionar(int arreglo[], int bajo, int alto) {
    int pivote = arreglo[alto];
    int i = (bajo - 1);

    for (int j = bajo; j < alto; j++) {
        if (arreglo[j] <= pivote) {
            i++;
            intercambiar(&arreglo[i], &arreglo[j]);
        }
    }
    intercambiar(&arreglo[i + 1], &arreglo[alto]);
    return (i + 1);
}

void quickSort(int arreglo[], int bajo, int alto) {
    if (bajo < alto) {
        int indiceParticion = particionar(arreglo, bajo, alto);
        quickSort(arreglo, bajo, indiceParticion - 1);
        quickSort(arreglo, indiceParticion + 1, alto);
    }
}

// ------------------------------------------------------------------
