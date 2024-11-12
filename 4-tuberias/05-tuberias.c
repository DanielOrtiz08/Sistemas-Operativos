#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int tubs[2][2];

    pipe(tubs[0]);
    pipe(tubs[1]);

    if(!fork()) {
        int data;

        if(!fork()) {
            data = 50;

            close(tubs[0][0]);
            close(tubs[0][1]);
            close(tubs[1][0]);

            printf("llego nieto\n");
            write(tubs[1][1], &data, sizeof(int));
            printf("nieto escribe %d \n", data);
        } else {
            sleep(10);
            close(tubs[0][0]);
            close(tubs[1][1]);

            printf("llego hijo\n");
            read(tubs[1][0], &data, sizeof(int));
            printf("Hijo lee del nieto %d\n", data);

            write(tubs[0][1], &data, sizeof(int));
            printf("Hijo escribe %d\n", data);
            
            wait(NULL);
        }
    } else {
        int data;
        usleep(250);
        close(tubs[0][1]);
        close(tubs[1][0]);
        close(tubs[1][1]);

        printf("llego padre\n");
        read(tubs[0][0], &data, sizeof(int));
        printf("padre lee %d \n", data);
        
        wait(NULL);
    }

    printf("termine\n");
    return EXIT_SUCCESS;
}