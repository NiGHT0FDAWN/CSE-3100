#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct thread_data {
    int thread_num;
};

/* Global synchronisation primitives */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond_prof = PTHREAD_COND_INITIALIZER;
pthread_cond_t  cond_student = PTHREAD_COND_INITIALIZER;

/* Global state */
int waiting_student = 0;   /* 0 if empty, otherwise id of waiting student */
int current_student = 0;   /* id of student being seen (0 if none)  */
int next_expected = 1;     /* next student that must be processed   */
int done = 0;              /* flag: professor has finished          */

void *professor(void *p) {
    int max_see = *(int *)p;
    int seen = 0;

    /* See at most max_see students, or until no more students exist */
    while (seen < max_see && next_expected <= max_see + (max_see - seen)) {
        pthread_mutex_lock(&mutex);

        /* Wait for the next expected student to enter the waiting room */
        while (waiting_student != next_expected && !done) {
            pthread_cond_wait(&cond_prof, &mutex);
        }

        if (done) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        /* Admit the waiting student */
        current_student = waiting_student;
        waiting_student = 0;
        pthread_cond_signal(&cond_student);  /* tell the student it has been promoted */
        pthread_mutex_unlock(&mutex);

        printf("Professor seeing student %d\n", current_student);
        sleep(3);                            /* meeting duration */

        /* Meeting finished, update counters */
        pthread_mutex_lock(&mutex);
        current_student = 0;
        seen++;
        next_expected++;
        pthread_mutex_unlock(&mutex);
    }

    /* All done: dismiss any waiting student and set the done flag. */
    pthread_mutex_lock(&mutex);
    done = 1;
    if (waiting_student != 0) {
        pthread_cond_broadcast(&cond_student);
    }
    pthread_mutex_unlock(&mutex);

    printf("All done\n");
    pthread_exit(NULL);
}

void *student(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    int id = data->thread_num;
    free(data);

    /* Simulate travel time before the student knocks */
    int sleep_time = rand() % 3 + 1;
    sleep(sleep_time);

    pthread_mutex_lock(&mutex);

    /* Professor has already finished */
    if (done) {
        pthread_mutex_unlock(&mutex);
        printf("Student %d leaves because professor done\n", id);
        pthread_exit(NULL);
    }

    /* Out of turn OR buffer already occupied */
    if (id != next_expected || waiting_student != 0) {
        pthread_mutex_unlock(&mutex);
        printf("Student %d leaves because buffer full\n", id);
        pthread_exit(NULL);
    }

    /* It is this student's turn and the waiting spot is free */
    waiting_student = id;
    printf("Student %d is waiting\n", id);
    pthread_cond_signal(&cond_prof);        /* alert the professor */

    /* Wait until promoted or the whole session ends */
    while (waiting_student == id && !done) {
        pthread_cond_wait(&cond_student, &mutex);
    }

    if (done) {
        if (waiting_student == id) {        /* never promoted */
            waiting_student = 0;
            printf("Student %d leaves because professor done\n", id);
        }
        /* else: was promoted right before done was set (impossible here) */
    }
    /* If not done, the student was successfully promoted – do nothing further. */

    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: ./solution seed n p\n");
        exit(1);
    }

    int seed = atoi(argv[1]);
    int n    = atoi(argv[2]);
    int p    = atoi(argv[3]);
    srand(seed);

    /* Professor thread */
    pthread_t prof_tid;
    int p_val = p;
    pthread_create(&prof_tid, NULL, professor, &p_val);

    /* Student threads (ids 1..n) */
    pthread_t *stud_tids = malloc(sizeof(pthread_t) * n);
    for (int i = 1; i <= n; i++) {
        struct thread_data *td = malloc(sizeof(struct thread_data));
        td->thread_num = i;
        pthread_create(&stud_tids[i-1], NULL, student, td);
    }

    /* Wait for all threads to finish */
    pthread_join(prof_tid, NULL);
    for (int i = 0; i < n; i++) {
        pthread_join(stud_tids[i], NULL);
    }

    free(stud_tids);
    return 0;
}