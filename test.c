#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
    int id;
    int points;
} Questions;

int main() {
    // Questions * q1;
    // q1->id = 1;
    // q1->points = 3;
    // printf("%d\n",q1->points);

    // int a = fork();
    // if (a==0){
    //     execl("/bin/ls","ls","-l", NULL);
    //     char *args[]={"ls","-l",NULL};
    //     printf("If you can see this, exec failed\n");
    // } else {
    //     wait(NULL);
    // }
    // return 0;

    // int a = 0;
    // pid_t pid = fork();
    // if (pid == 0){
    //     a++;
    // }
    // a--;
    // if (pid) wait (NULL);
    // printf("a = %d\n",a);
    // return 0;
    // int count = 0;
    // if (fork()){
    //     printf("%p\n", &count);
    //     wait(NULL);
    // } else {
    //     printf("%p\n", &count);
    //     count++;
    // }
    // printf("%d\n",count);
    // return 0;
    
    // printf("...\n");
    // int fd = open("dup2-output.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    // if (fd<0){
    //     printf("error openfile \n");
    //     return -1;
    // }
    // dup2(fd,1);
    // close(fd);
    // printf(".\n");
    // return 0;

    
}