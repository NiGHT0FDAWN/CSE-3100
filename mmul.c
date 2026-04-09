#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "matrix.h"

// Search TODO to find the locations where code needs to be completed

#define     NUM_THREADS     2

typedef struct {
    unsigned int id;
    TMatrix *m, *n, *t;
} thread_arg_t;

static void * thread_main(void * p_arg)
{
    // TODO
    thread_arg_t *arg = (thread_arg_t *) p_arg;
    unsigned int id = arg->id;
    TMatrix *m = arg->m;
    TMatrix *n = arg->n;
    TMatrix *t = arg->t;

    unsigned int total_rows = m->nrows;
    unsigned int rows_per_thread = total_rows / NUM_THREADS;
    unsigned int start_row = id * rows_per_thread;
    unsigned int end_row = (id == NUM_THREADS - 1) ? total_rows : (id + 1) * rows_per_thread;

    for (unsigned int i = start_row; i < end_row; i++) {
        for (unsigned int j = 0; j < n->ncols; j++) {
            TElement sum = 0.0;
            for (unsigned int k = 0; k < m->ncols; k++) {
                sum += m->data[i][k] * n->data[k][j];
            }
            t->data[i][j] = sum;
        }
    }
    return NULL;
}

/* Return the sum of two matrices.
 *
 * If any pthread function fails, report error and exit. 
 * Return NULL if anything else is wrong.
 *
 * Similar to mulMatrix, but with multi-threading.
 */
TMatrix * mulMatrix_thread(TMatrix *m, TMatrix *n)
{
    if (    m == NULL || n == NULL
         || m->ncols != n->nrows )
        return NULL;

    TMatrix * t = newMatrix(m->nrows, n->ncols);
    if (t == NULL)
        return t;

    // TODO
    pthread_t threads[NUM_THREADS];
    thread_arg_t args[NUM_THREADS];
    int rv;

    for (unsigned int i = 0; i < NUM_THREADS; i++) {
        args[i].id = i;
        args[i].m = m;
        args[i].n = n;
        args[i].t = t;
        rv = pthread_create(&threads[i], NULL, thread_main, &args[i]);
        check_pthread_return(rv, "pthread_create");
    }

    for (unsigned int i = 0; i < NUM_THREADS; i++) {
        rv = pthread_join(threads[i], NULL);
        check_pthread_return(rv, "pthread_join");
    }
    return t;
}