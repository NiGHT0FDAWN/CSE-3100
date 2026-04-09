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
    // USE valgrind --track-fds=yes ./file args
    
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
    
    // printf("...\n");
    // int fd = open("dup2-output.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    // if (fd<0){
    //     printf("error openfile \n");
    //     return -1;
    // }
    // dup2(fd,1);
    // close(fd);
    // printf(".\n");
    
    // int fd = open("testtxt.txt", O_RDWR);
    // char buff[10];
    // int v = read(fd,buff,10);
    // printf(". %d", v);

    // execl("/usr/bin/zip", "zip", "files.zip", "a.txt", "b.c", "b", NULL);
    // execlp("zip", "zip", "files.zip", "a.txt", "b.c", "b", NULL);
    // execle("zip", "zip", "files.zip", "a.txt", "b.c", "b", NULL);
    // char *argv[]={"files.zip", "files.zip", "a.txt", "b.c", "b", NULL};
    // execv("/usr/bin/zip", argv);
    // execvp("zip", argv);
    // execvpe("zip", argv);
    
    // int fd = open("a.txt", O_RDWR | O_CREAT);
    // int dfd = dup(fd);
    // dup2(fd, 0);
    // dup2(dfd, 1);
    // char *s1 = "Hello";
    // close(fd);
    char *load[16];
    read("a.txt", &load, sizeof(int));
    printf("%s", load);
    return 0;
}