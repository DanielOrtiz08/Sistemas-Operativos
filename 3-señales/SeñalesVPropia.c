#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

void showtree() {
   char command[50];
   sprintf(command, "pstree -lp %d", getpid());
   system(command);
}

void sighandler() {}

void accion(const char *nombre, pid_t idProceso, int signal) {
   printf("%s [%d]\n", nombre, getpid());
   usleep(100);
   kill(idProceso, signal);
}

int main(int argc, char **argv) {

   pid_t pid_hijos[4]; // [h1, h2, h3, h21]
   pid_t root = getpid(); // padre

   void * oldhandler = signal(SIGUSR1, sighandler);
   if (oldhandler == SIG_ERR) {
      perror("signal:");
      exit(EXIT_FAILURE);
   }
   
   int i;
   for (i = 0; i < 3; i++) {
      pid_hijos[i] = fork();
      if (pid_hijos[i] == 0) {
         if (i == 1) {
            pid_hijos[3] = fork();
         }
         break;
      }
   }

   if(root == getpid()) { // padre
      showtree();
      accion("Padre", pid_hijos[2], SIGUSR1);
      pause();
   } else { // hijos
      pause();
      if(i == 0) { // si es h1
         accion("Hijo1", root, SIGUSR1);
      }else if(i == 1) {
         if(pid_hijos[3] == 0) { // si es nieto(h21) o hijo de h2
            accion("Hijo21", pid_hijos[1], SIGUSR1);
         } else { // si es h2
            accion("Hijo2", pid_hijos[3], SIGUSR1);
            pause();
            accion("Hijo2", pid_hijos[0], SIGUSR1);
         }
      }else if(i == 2) { // si es h3
         accion("Hijo3", pid_hijos[1], SIGUSR1);
      }
   }

   if(i == 1) wait(NULL);
   if(root == getpid()) {
      for (int j = 0; j < 3; j++){
         wait(NULL);
      }
   }
   

   if(signal(SIGUSR1, oldhandler) == SIG_ERR) {
      perror("signal:");
      exit(EXIT_FAILURE);
   }

   return EXIT_SUCCESS;
}