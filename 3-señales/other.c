#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

pid_t primer_hijo = -1;  // Guardará el ID del primer hijo en terminar
pid_t segundo_hijo = -1; // Guardará el ID del segundo hijo en terminar

void handle_signal(int sig, siginfo_t *info, void *context) {
    // Esta función manejará las señales que envían los hijos
    pid_t sender_pid = info->si_pid;  // Obtener el PID del hijo que envió la señal
    if (primer_hijo == -1) {
        primer_hijo = sender_pid;  // El primer hijo en terminar
    } else {
        segundo_hijo = sender_pid; // El segundo hijo en terminar
    }
}

int main() {
    pid_t pid1, pid2;

    // Configurar el manejador de señales con más información
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_signal;
    sigaction(SIGUSR1, &sa, NULL); // Configurar para recibir SIGUSR1
    sigaction(SIGUSR2, &sa, NULL); // Configurar para recibir SIGUSR2

    // Crear el primer proceso hijo
    if ((pid1 = fork()) == 0) {
        // Este es el primer hijo (Hijo 1)
        printf("Hijo 1: Realizando la tarea 1...\n");
        sleep(2); // Simular tiempo para la tarea 1
        kill(getppid(), SIGUSR1); // Notificar al padre que ha terminado la tarea 1
        pause(); // Esperar asignación de tarea 3 o 4
        printf("Hijo 1: Realizando tarea asignada...\n");
        sleep(2); // Simular tiempo para la tarea
        exit(0);
    }

    // Crear el segundo proceso hijo
    if ((pid2 = fork()) == 0) {
        // Este es el segundo hijo (Hijo 2)
        printf("Hijo 2: Realizando la tarea 2...\n");
        sleep(3); // Simular tiempo para la tarea 2
        kill(getppid(), SIGUSR2); // Notificar al padre que ha terminado la tarea 2
        pause(); // Esperar asignación de tarea 3 o 4
        printf("Hijo 2: Realizando tarea asignada...\n");
        sleep(2); // Simular tiempo para la tarea
        exit(0);
    }

    // Proceso padre
    printf("Padre: Esperando que los hijos terminen sus primeras tareas...\n");

    // Esperar a que ambos hijos notifiquen que han terminado sus primeras tareas
    while (primer_hijo == -1 || segundo_hijo == -1) {
        pause(); // Pausar el proceso padre hasta recibir las señales de los hijos
    }

    // Asignar tareas dinámicamente según el orden de finalización
    printf("Padre: El primer hijo en terminar fue %d, se le asignará la tarea 3.\n", primer_hijo);
    kill(primer_hijo, SIGCONT); // Continuar el primer hijo para la tarea 3

    printf("Padre: El segundo hijo en terminar fue %d, se le asignará la tarea 4.\n", segundo_hijo);
    kill(segundo_hijo, SIGCONT); // Continuar el segundo hijo para la tarea 4

    // Esperar a que ambos hijos terminen sus tareas
    wait(NULL);
    wait(NULL);

    printf("Padre: Todos los hijos han terminado sus tareas.\n");

    return 0;
}
