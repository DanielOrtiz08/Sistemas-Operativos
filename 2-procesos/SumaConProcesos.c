// Daniel Ortiz
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void error(char*);
int numeroProcesos();
int leerNumeros(char*, int**);
int creacionProcesos(int);
void escribirSuma(long int);
long int leerTotal(int);

int main(int argc, char *argv[]) {
   
   int nProcesos = numeroProcesos();

   int proceso = creacionProcesos(nProcesos);

   if(proceso != nProcesos) { // si no es el proceso padre (algún hijo)
      int *vector;
      int cantidadNumeros = leerNumeros(argv[1], &vector); // solo es necesario en los hijos por eso está dentro de este scope
      
      long int sumap = 0;
      
      double delta = (double) cantidadNumeros/nProcesos;  
      int inf = delta * proceso;
      int sup = delta * (proceso + 1); 
      
      if(proceso == nProcesos - 1) sup = cantidadNumeros; // si es el ultimo proceso se asigna el total de numeros para que incluya el ultimo numero que se perderia por el resultado de la division que da decimales
      
      for (int j = inf; j < sup; j++) {
         sumap += vector[j];
      }

      escribirSuma(sumap);

      free(vector);
   } 
   else { // si es el proceso padre
      for(int j = 0; j < nProcesos; j++) {
         wait(NULL);
      }
      long int sumaTotal = leerTotal(nProcesos);
      printf("la suma total es %ld\n", sumaTotal);
      
      remove("out.txt");
   }

   return EXIT_SUCCESS;
}

int numeroProcesos() {
   int nProcesos;
   printf("Numero de procesos: ");
   scanf("%d", &nProcesos);
   if(nProcesos <= 0 || nProcesos > 8000) error("Debe ser mayor a 0 y menor o igual a 8000");
   return nProcesos;
}

int leerNumeros(char *filename, int **vec){
   FILE *infile = fopen(filename, "r");
   if(!infile ){ 
      error("Error fopen\n");
   }
   int totalNumeros;
   fscanf(infile, "%d", &totalNumeros);
   *vec = (int *)calloc(totalNumeros, sizeof(int));
   if(!*vec){
      error("error calloc");
   }
   for(int c = 0; c < totalNumeros; c++){
      fscanf(infile, "%d", &(*vec)[c]);
   }
   fclose(infile);
   return totalNumeros;
}

int creacionProcesos(int nProcesos) {
   int i;
   for(i = 0; i < nProcesos; i++) {
      if(!fork()) {
         break;
      }
   }
   return i;
}

void error(char *msg) {
   perror(msg);
   exit(-1);
}

void escribirSuma(long int suma) {
   FILE *outfile = fopen("out.txt", "a");
   if(!outfile) {
      error("Error al abrir el archivo\n");
   }
   fprintf(outfile, "%ld\n", suma);
   fclose(outfile);
}

long int leerTotal(int nProceso){
   FILE *infile = fopen("out.txt","r");
   if(!infile) {
      error("Error padre archivo resultados");
   }
   long int sumap = 0, total = 0;
   for(int i = 0; i < nProceso; i++) {
      fscanf(infile,"%ld", &sumap);
      total += sumap;
   }
   fclose(infile);
   return total;
}

