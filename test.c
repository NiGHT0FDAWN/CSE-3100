#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
    int id;
    int points;
} Questions;

int main() {
    // Questions * q1;
    // q1->id = 1;
    // q1->points = 3;

    // printf("%d\n",q1->points);
    

    int a = fork();
    if (a==0){
        execl("/bin/ls","ls","-l", NULL);
        char *args[]={"ls","-l",NULL};
        printf("If you can see this, exec failed\n");
    } else {
        wait(NULL);
    }
    return 0;
}