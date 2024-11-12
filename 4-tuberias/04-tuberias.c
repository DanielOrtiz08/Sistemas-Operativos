#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {

    int tubs[2][2];
    int root = getpid();
    
    for (int i = 0; i < 2; i++) {
        pipe(tubs[i]);
    }

    int i;
    for (i = 0; i < 2; i++) {
        if(!fork()) break;
    }
    
    if(root == getpid()) {
        float datos[2];

        close(tubs[0][1]);
        close(tubs[1][1]);

        for (int i = 0; i < 2; i++) {
            read(tubs[i][0], &datos[i], sizeof(float));
            printf("[%d] hijo : <-- %.3f\n", i, datos[i]);
        }

        printf("suma total = %.3f\n", datos[0] + datos[1]);

        wait(NULL);
        wait(NULL);
    } else {
        int tc = (i == 1)? 0 : 1;

        float data;
        
        close(tubs[0][0]);
        close(tubs[1][0]);
        close(tubs[tc][1]);

        data = (i == 0)? 10.3: 8.4;

        write(tubs[i][1], &data, sizeof(data));
        printf("[%d] Hijo --> %.3f\n", getpid(), data);
    }

    return EXIT_SUCCESS;
}
