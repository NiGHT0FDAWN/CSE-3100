#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// write an integer to a pipe
void write_int(int pd, int value) { write(pd, &value, sizeof(int)); }
// read an integer from a pipe
// the function returns the number of bytes read
int read_int(int pd, int *value) { return read(pd, value, sizeof(int)); }

double random_double() { return (double)rand() / RAND_MAX; }

int coin_flip(double p) { return random_double() < p; }

void run_simulation(int n, double p) {
    int pd1[2];
    // pipe creation
    if (pipe(pd1) < 0) {
        perror("Error.");
        exit(-1);
    }

    int pd2[2];
    // pipe creation
    if (pipe(pd2) < 0) {
        perror("Error.");
        exit(-1);
    }
    pid_t pid = fork();
    if (pid == 0) {
        // TODO
        // fill in code below
        // note this is player A

        srand(3100);

        int v;
        // TODO
        // complete the following line of code
        while (read_int(pd2[], &v) != 0) {
        // TODO
        // fill in code below
        }
        // TODO
        // fill in code below
    }
    // TODO
    // fill in code below

    int pd3[2];
    // pipe creation
    if (pipe(pd3) < 0) {
        perror("Error.");
        exit(-1);
    }

    int pd4[2];
    // pipe creation
    if (pipe(pd4) < 0) {
        perror("Error.");
        exit(-1);
    }
    pid_t pid1 = fork();

    if (pid1 == 0) {
        // TODO
        // fill in code below
        // note this is player B

        srand(3504);

        int v;
        // TODO
        // complete the following line of code
        while (read_int(pd4[], &v) != 0) {
        // TODO
        // fill in code below
        }

        // TODO
        // fill in code below
    }

    int n1 = 0;
    int n2 = 0;
    int v1, v2;

    while (n1 + n2 < n) {
        // TODO
        // finish the follow lines of code
        write_int(, 1);
        write_int(, 1);
        read_int(, &v1);
        read_int(, &v2);
        if (v1)
        n1++;
        else {
        if (v2)
            n2++;
        }
    }
    printf("prob of A wins = %.3lf\n", (double)(n1) / (n1 + n2));
    // TODO
    // fill in code below
}
