#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char ** argv)
{
    pid_t child;
    int exitStatus;

    // at least, there should be 3 arguments
    // 2 for the first command, and the rest for the second command
    if (argc < 4) {
        fprintf(stderr, "Usage: %s cmd1 cmd1_arg cmd2 [cmd2_args ..]\n", argv[0]);
        return 1;
    }

    child = fork();
    if (child == -1){
        perror("fork()");
        return -1;
    } else {
        execlp(argv[1], argv[1], argv[2], NULL);
        perror("execlp()");
    }
    child = fork();
    if (child == -1){
        perror("fork()");
        return 1;
    } else if (child == 0){
        execlp(argv[1], argv[1], argv[2], NULL);
        perror("execlp()");
    }
    
    pid_t child2;
    int exitStatus2;
    child2 = fork();
    if (child2 == -1){
        perror("fork()");
        return 1;
    } else if (child2 == 0){
        execvp(argv[3], &argv[3]);
        perror("execvp()");
    }
    waitpid(child, &exitStatus, 0);
    printf("exited=%d exitstatus=%d", WIFEXITED(exitStatus), WEXITSTATUS(exitStatus));
    waitpid(child2, &exitStatus2, 0);
    printf("exited=%d exitstatus=%d\n", WIFEXITED(exitStatus2), WEXITSTATUS(exitStatus2));
    
    
    return 0;
}
