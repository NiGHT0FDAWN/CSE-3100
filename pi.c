#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define N_THREADS 2          // Two threads by default
#define SAMPLE_SIZE 10000000 // Ten million by default

typedef struct thread_arg_tag {
  int id;    // id for the thread, used as seed for random number generator
  int count; // number of points whose distance to (0,0) is within 1
  int total; // number of points - (x,y) pairs, this thread should generate
} thread_arg_t;

double rand_double(unsigned int *s) { return (rand_r(s) / (double)RAND_MAX); }

void *thread_main(void *thread_arg) {
	// 6 points max for all the initialization
  thread_arg_t *arg = thread_arg;

  int count = 0;
  int total = arg->total;
  unsigned int s = arg->id;

  double x, y;

  for (int i = 0; i < total; i++) { // 1 point
    x = rand_double(&s); //1 point
    y = rand_double(&s); //1 points

    if (x * x + y * y <= 1) // 1 point
      count++; // 1 point
  }
  arg->count = count; // 1 point
  pthread_exit(NULL); //2 point
}

int main(int argc, char *argv[]) {

  int n = SAMPLE_SIZE;
  int n_threads = N_THREADS;

  int total_count = 0;

  if (argc > 1) {
    n = atoi(argv[1]);
  }
  if (argc > 2) {
    n_threads = atoi(argv[2]);
  }

  pthread_t thread_arr[n_threads];
  thread_arg_t thread_args[n_threads];
  int rv;

  for (int i = 0; i < n_threads; i++) { // 1 point
    thread_args[i].id = i; // 1 point
    thread_args[i].count = 0; // 1 point
    thread_args[i].total = n / n_threads; // 1 point
    rv = pthread_create(&thread_arr[i], NULL, thread_main, &thread_args[i]); // 3 points
  }

  for (int i = 0; i < n_threads; i++) { // 1 point
    rv = pthread_join(thread_arr[i], NULL); // 3 points
  }

  for (int i = 0; i < n_threads; i++) { //1 point
    total_count += thread_args[i].count; //1 point
  }

  double pi = 4.0 * total_count / (double)n;

  printf("pi = %.2f\n", pi);

  return 0;
}
