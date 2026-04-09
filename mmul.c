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
    TMatrix *m = arg->m;
    TMatrix *n = arg->n;
    TMatrix *t = arg->t;
    unsigned int nrows = m->nrows;
    unsigned int ncols = m->ncols;
    unsigned int ncols2 = n->ncols;
    unsigned int id = arg->id;

    unsigned int start, end;
    if (id == 0) {
        start = 0;
        end = nrows / 2;
    } else { // id == 1
        start = nrows / 2;
        end = nrows;
    }

    for (unsigned int i = start; i < end; i++) {
        for (unsigned int j = 0; j < ncols2; j++) {
            TElement sum = 0.0;
            for (unsigned int k = 0; k < ncols; k++) {
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
    int ret;

    for (unsigned int i = 0; i < NUM_THREADS; i++) {
        args[i].id = i;
        args[i].m = m;
        args[i].n = n;
        args[i].t = t;
        ret = pthread_create(&threads[i], NULL, thread_main, &args[i]);
        if (ret != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    for (unsigned int i = 0; i < NUM_THREADS; i++) {
        ret = pthread_join(threads[i], NULL);
        if (ret != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }
    return t;
}