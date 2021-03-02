#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include "compute.h"
#include "ref1.c"

 pthread_barrier_t barrier;

/* Struct with parameters for each thread */
typedef struct thread_param
{
    int id;
    int row_start;
    int row_end;
    int maxiter;
    size_t *iter;

    struct results *results;
    struct parameters *parameters;
    struct timeval *before;
    struct timeval *after;
    double **src;
    double **dst;
    double **cond;
} thread_param;


void *threadwork(void *param)
{
    static const double c_cdir = 0.25 * M_SQRT2 / (M_SQRT2 + 1.0);
    static const double c_cdiag = 0.25 / (M_SQRT2 + 1.0);

    /* Get all values from parameter struct */
    thread_param *thread_parameters = (thread_param*) param;
    size_t i, j, iter;
    struct timeval *before = thread_parameters->before;
    struct timeval *after = thread_parameters->after;
    int maxiter = thread_parameters->parameters->maxiter;
    int h = thread_parameters->parameters->N + 2;
    int w = thread_parameters->parameters->M + 2;
    int print_reports = thread_parameters->parameters->printreports;
    double (*restrict c)[h][w] = thread_parameters->cond;
    double (*restrict src)[h][w] = thread_parameters->src;
    double (*restrict dst)[h][w] = thread_parameters->dst;
    int start_row = thread_parameters->row_start;
    int end_row = thread_parameters->row_end;
    iter = thread_parameters->iter;

    for (iter = 1; iter <= maxiter; ++iter)
    {
        /* swap source and destination */
        { void *tmp = src; src = dst; dst = tmp; }

        /* initialize halo on source */
        do_copy(h, w, src);

        double maxdiff = 0.0;

        pthread_barrier_wait(&barrier);
        /* compute */
        for (i = start_row; i < end_row; ++i)
            for (j = 1; j < w - 1; ++j)
            {
                double w = (*c)[i][j];
                double restw = 1.0 - w;

                (*dst)[i][j] = w * (*src)[i][j] +

                    ((*src)[i+1][j  ] + (*src)[i-1][j  ] +
                     (*src)[i  ][j+1] + (*src)[i  ][j-1]) * (restw * c_cdir) +

                    ((*src)[i-1][j-1] + (*src)[i-1][j+1] +
                     (*src)[i+1][j-1] + (*src)[i+1][j+1]) * (restw * c_cdiag);

                double diff = fabs((*dst)[i][j] - (*src)[i][j]);
                if (diff > maxdiff) maxdiff = diff;
            }
           thread_parameters->results->maxdiff=maxdiff;
        // if(maxdiff < thread_parameters->parameters->threshold){iter++;break;}
        /* conditional reporting */
        // if (thread_parameters->id < 2) printf("thread %d iter = %d\n",thread_parameters->id, iter);
        if (thread_parameters->id == 0 && iter % thread_parameters->parameters->period == 0) {
           fill_report(thread_parameters->parameters, thread_parameters->results, h, w, dst, src, iter, thread_parameters->before);
            if(thread_parameters->parameters->printreports) report_results(thread_parameters->parameters, thread_parameters->results);
        }
    }
}

void do_compute(const struct parameters *p, struct results *r)
{
    size_t i, j;
    int num_threads = p->nthreads;

    /* alias input parameters */
    const double(*restrict tinit)[p->N][p->M] = (const double(*)[p->N][p->M])p->tinit;
    const double(*restrict cinit)[p->N][p->M] = (const double(*)[p->N][p->M])p->conductivity;

    /* allocate grid data */
    const size_t h = p->N + 2;
    const size_t w = p->M + 2;
    double(*restrict g1)[h][w] = malloc(h * w * sizeof(double));
    double(*restrict g2)[h][w] = malloc(h * w * sizeof(double));

    /* allocate halo for conductivities */
    double(*restrict c)[h][w] = malloc(h * w * sizeof(double));

    struct timespec before, after;

    static const double c_cdir = 0.25 * M_SQRT2 / (M_SQRT2 + 1.0);
    static const double c_cdiag = 0.25 / (M_SQRT2 + 1.0);

    /* set initial temperatures and conductivities */
    for (i = 1; i < h - 1; ++i)
        for (j = 1; j < w - 1; ++j)
        {
            (*g1)[i][j] = (*tinit)[i - 1][j - 1];
            (*c)[i][j] = (*cinit)[i - 1][j - 1];
        }

    /* smear outermost row to border */
    for (j = 1; j < w - 1; ++j)
    {
        (*g1)[0][j] = (*g2)[0][j] = (*g1)[1][j];
        (*g1)[h - 1][j] = (*g2)[h - 1][j] = (*g1)[h - 2][j];
    }

    /* compute */
    size_t iter;
    double(*restrict src)[h][w] = g2;
    double(*restrict dst)[h][w] = g1;

    clock_gettime(CLOCK_MONOTONIC, &before);

    /* Determine split size of grid over threads */
    int rows_per_threads[num_threads];
    int row_count = (h - 2) / num_threads;
    for (int i = 0; i < num_threads; i++) { rows_per_threads[i] = row_count; }
    int rest = (h - 2) - (num_threads * row_count);
    int fill_rest_thread = 0;
    while (rest != 0)
    {
        rows_per_threads[fill_rest_thread % num_threads]++;
        rest--;
        fill_rest_thread++;
    }

    /* Determine start end endpoints per thread */
    int threads_row_start[num_threads];
    int threads_row_end[num_threads];
    int filled_rows = 0;
    for (int proc = 0; proc < num_threads; proc++)
    {
        threads_row_start[proc] = 1 + filled_rows;
        threads_row_end[proc] = 1 + filled_rows + rows_per_threads[proc];
        filled_rows += rows_per_threads[proc];
    }

    pthread_t pthreads[num_threads];
    int ret;
    ret = pthread_barrier_init(&barrier, NULL, num_threads);

    /*Fill array of structs with all threads their parameters */
    thread_param thread_parameters[num_threads];
    for (int proc = 0; proc < num_threads; proc++)
    {
        thread_param *current_params = &thread_parameters[proc];
        current_params->id = proc;
        current_params->before = &before;
        current_params->after = &after;
        current_params->results = r;
        current_params->parameters = p;
        current_params->row_end = threads_row_end[proc];
        current_params->row_start = threads_row_start[proc];
        current_params->cond = (double **)c;
        current_params->src = (double **)src;
        current_params->dst = (double **)dst;
        current_params->iter = &iter;

        pthread_create(&pthreads[proc], NULL, threadwork, current_params);
    }

    /* wait for threads to finish */
    for (int proc = 0; proc < num_threads; proc++)
    {
        if (pthread_join(pthreads[proc], NULL))
        {
            fprintf(stderr, "Error joining thread\n");
            /* TODO returns status exit */
        }
    }

    /* report at end in all cases */
     iter = p->maxiter;
    // iter--;
    // printf("iter = %d\n", iter);
    fill_report(p, r, h, w, dst, src, iter, &before);

    free(c);
    free(g2);
    free(g1);

    clock_gettime(CLOCK_MONOTONIC, &after);
    r->time = (double)(after.tv_sec - before.tv_sec) +
              (double)(after.tv_nsec - before.tv_nsec) / 1e9;
}
