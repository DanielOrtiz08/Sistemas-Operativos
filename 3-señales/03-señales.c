#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define NUM_HIJOS 3

// Función para verificar si el proceso padre está pausado
int is_parent_paused(pid_t ppid) {
    char path[40], state;
    FILE *status_file;

    snprintf(path, sizeof(path), "/proc/%d/status", ppid);
    status_file = fopen(path, "r");
    if (!status_file) {
        perror("Error al abrir /proc/<pid>/status");
        return 0;
    }

    // Leer el estado del proceso
    while (fscanf(status_file, " State: %c", &state) != 1) {
        // Ignorar hasta encontrar el campo de estado
        fscanf(status_file, "%*[^\n]\n");
    }
    fclose(status_file);

    return state == 'T';  // 'T' indica que el proceso está detenido
}

// Función que ejecutan los procesos hijos
void child_process(pid_t ppid) {
    sleep(1);  // Espera un poco antes de intentar continuar al padre
    if (is_parent_paused(ppid)) {
        printf("Hijo %d: El padre está pausado, enviando SIGCONT.\n", getpid());
        kill(ppid, SIGCONT);
    } else {
        printf("Hijo %d: El padre no está pausado, no envío SIGCONT.\n", getpid());
    }
}

int main() {
    pid_t ppid = getpid();
    pid_t pid;
    int i;

    // Crear procesos hijos
    for (i = 0; i < NUM_HIJOS; i++) {
        pid = fork();
        if (pid < 0) {
            perror("Error al crear proceso hijo");
            exit(1);
        }
        if (pid == 0) {
            // Proceso hijo
            child_process(ppid);
            exit(0);
        }
    }

    // Proceso padre se detiene a sí mismo
    printf("Padre (PID %d): Me voy a pausar con SIGSTOP.\n", ppid);
    raise(SIGSTOP);

    // Este código se ejecuta después de recibir SIGCONT
    printf("Padre (PID %d): He sido continuado.\n", ppid);

    // Esperar a que terminen los hijos
    for (i = 0; i < NUM_HIJOS; i++) {
        wait(NULL);
    }

    return 0;
}
