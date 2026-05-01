#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int chair[2] = {0,0};

void *professor(void *p) {
    int see = 0;
    while(see < p){
        // wait for a student to show up
        // sleep(3)
        // go for next student


    }
    // announce all done
    // 

    
    
    pthread_exit(NULL);
}

void *student(void) {
    // waits for rand()%3+1 seconds to knock.
    // added to buffer or leaves
    // next student
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: ./solution seed n p\n");
        exit(1);
    }

    int seed = atoi(argv[1]);
    int n = atoi(argv[2]);
    int p = atoi(argv[3]);
    srand(seed);

    pthread_t prof;
    pthread_create(&prof, NULL, professor, &p);

    pthread_t *stu = malloc(sizeof(pthread_t) * n);
    for (int i = 1; i <= n; i++) {
        pthread_create(&stu[i-1], NULL, student, i);
    }
}