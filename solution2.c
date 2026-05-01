#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define PR 0
#define PW 1

void write_int(int pd, int value) { write(pd, &value, sizeof(int)); }
int read_int(int pd, int *value) { return read(pd, value, sizeof(int)); }

int main(int argc, char* argv[])
{
    int n;

    if(argc < 2) {
        printf("usage: ./solution n\n");
        return 1;
    }
    n = atoi(argv[1]);

    int pdp1[2];
    if (pipe(pdp1) < 0) {
        perror("Error. pdp1 pipe init");
        exit(-1);
    }

    int pd1p[2];
    if (pipe(pd1p) < 0) {
        perror("Error. pd1p pipe init");
        exit(-1);
    }
    int pdp2[2];
    if (pipe(pdp2) < 0) {
        perror("Error. pdp2 pipe init");
        exit(-1);
    }

    int pd2p[2];
    if (pipe(pd2p) < 0) {
        perror("Error. pd2p pipe init");
        exit(-1);
    }
    int pd12[2];
    if (pipe(pd12) < 0) {
        perror("Error. pd12 pipe init");
        exit(-1);
    }

    int pd21[2];
    if (pipe(pd21) < 0) {
        perror("Error. pd21 pipe init");
        exit(-1);
    }
    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(pdp1[PW]);
        close(pd1p[PR]);
        close(pd12[PR]);
        close(pd21[PW]);
        close(pdp2[PR]);
        close(pdp2[PW]);
        close(pd2p[PR]);
        close(pd2p[PW]);
        
        read_int(pdp1[PR],&n);
        printf("Child 1 read:%d\n",n);
        printf("Child 1 wrote:%d\n",n*2);
        write_int(pd12[PW],n*2);

        read_int(pd21[PR],&n);
        printf("Child 1 read:%d\n",n);
        printf("Child 1 wrote:%d\n",n*2);
        write_int(pd1p[PW],n*2);

        close(pdp1[PR]);
        close(pd1p[PW]);
        close(pd21[PR]);
        close(pd12[PW]);
        exit(0);
    }
    
    pid_t pid2 = fork();

    if (pid2 == 0) {
        close(pdp2[PW]);
        close(pd2p[PR]);
        close(pd21[PR]);
        close(pd12[PW]);
        close(pdp1[PR]);
        close(pdp1[PW]);
        close(pd1p[PR]);
        close(pd1p[PW]);

        read_int(pd12[PR],&n);
        printf("Child 2 read:%d\n",n);
        printf("Child 2 wrote:%d\n",n*4);
        write_int(pd2p[PW],n*4);

        read_int(pdp2[PR],&n);
        printf("Child 2 read:%d\n",n);
        printf("Child 2 wrote:%d\n",n*4);
        write_int(pd21[PW],n*4);

        close(pdp2[PR]);
        close(pd2p[PW]);
        close(pd21[PW]);
        close(pd12[PR]);
        exit(0);
    }
    close(pd2p[PW]);
    close(pdp2[PR]);
    close(pdp1[PR]);
    close(pd1p[PW]);
    close(pd21[PR]);
    close(pd21[PW]);
    close(pd12[PR]);
    close(pd12[PW]);

    printf("Parent wrote:%d\n",n);
    write_int(pdp1[PW], n);
    read_int(pd2p[PR],&n);
    printf("Parent read:%d\n",n);

    printf("Parent wrote:%d\n",n*8);
    write_int(pdp2[PW],n*8);
    read_int(pd1p[PR],&n);
    printf("Parent read:%d\n",n);

    close(pdp1[PW]);
    close(pd1p[PR]);
    close(pdp2[PW]);
    close(pd2p[PR]);
}