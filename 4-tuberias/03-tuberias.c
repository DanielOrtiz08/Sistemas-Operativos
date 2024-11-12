
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <wait.h>

#define MAX_READ 256

int main(void) {
    int fd[2], n; 
    char buffer[MAX_READ];

    printf("fd[0]=%d, fd[1]=%d\n", fd[0], fd[1]);

    pipe(fd);

    printf("Escribir 'salir' para terminar el programa\n");

    if(fork()) {
        close(fd[0]);

        do {
            fgets(buffer, MAX_READ, stdin);
            if(strlen(buffer) > 1) {
                buffer[strlen(buffer) -1] = '\0';
                write(fd[1], buffer, strlen(buffer)); // EOL = '0'
                printf("[%d] write: --> %s\n", getpid(), buffer);
            }
        } while (strcmp(buffer, "salir") != 0);        
        
        close(fd[1]);

        wait(NULL);
    } else {
        close(fd[1]);
        
        while ((n = read(fd[0], buffer, MAX_READ)) > 0) {
            printf("%d", n);
            buffer[n] = '\0';
            printf("[%d] read <--: %s\n", getpid(), buffer);
        }

        close(fd[0]);
    }
    
    return EXIT_SUCCESS;
}
