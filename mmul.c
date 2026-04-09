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

    // Each thread computes rows where row_index % NUM_THREADS == id
    for (unsigned int i = id; i < m->nrows; i += NUM_THREADS) {
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

    // Create worker threads
    for (unsigned int i = 0; i < NUM_THREADS; i++) {
        args[i].id = i;
        args[i].m = m;
        args[i].n = n;
        args[i].t = t;
        int ret = pthread_create(&threads[i], NULL, thread_main, &args[i]);
        check_pthread_return(ret, "pthread_create failed");
    }

    // Wait for all threads to finish
    for (unsigned int i = 0; i < NUM_THREADS; i++) {
        int ret = pthread_join(threads[i], NULL);
        check_pthread_return(ret, "pthread_join failed");
    }
    return t;
}