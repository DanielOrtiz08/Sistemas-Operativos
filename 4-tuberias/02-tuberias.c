
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <sys/wait.h>

#define MAX_READ 1024

struct data {
    int a;
    float b;
    // sizeof(struct) > (sizeof(int) + sizeof(float))
};

int main(void) {
    int fd[2]; 

    pipe(fd);

    if(fork()) {
        struct data *post = malloc(sizeof(struct data));

        close(fd[0]);

        post->a = 10;
        post->b = 2.3;

        printf("[%d] write: --> [%d | %.2f]\n", getpid(), post->a, post->b);

        write(fd[1], post, sizeof(struct data));

        free(post);

        close(fd[1]);

        wait(NULL);
    } else {
        struct data get;

        close(fd[1]);

        read(fd[0], &get, sizeof(struct data));

        for (int i = 0; i < 1000; i++) {
            printf("-");
        }
        
        
        printf("[%d] read: <-- [%d | %.2f]\n", getpid(), get.a, get.b);

        close(fd[0]);
    }
    
    return EXIT_SUCCESS;
}
