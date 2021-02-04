#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <input.h>
#include <time.h>
#include <math.h>
#include <output.h>
#include <string.h>
#include <limits.h>
#include "nmmintrin.h" // for SSE4.2
#include "immintrin.h" // for AVX  //TODO figure out which one

/* Define weak strong influences */
double weak_inf = 1 / (4 * (sqrt(2) + 1));
double strong_inf = sqrt(2) / (4 * (sqrt(2) + 1));

void do_compute(const struct parameters *p, struct results *r)
{
    double rtime;
    struct timeval start;
    struct timeval end;
    int M = p->M;
    int N = p->N;
    int num_bodies = M * (N + 2);
    int period = p->period;
    double tmin = p->io_tmin;
    double tmax = p->io_tmax;

    /* Create arrays for next and current bodies and conductivities */
    double next[N + 2][M];
    double current[N + 2][M];
    double cond[N + 2][M];

    /* Fill next and conductivity as 2D matrix */
    for (size_t i = 1; i < N + 1; i++)
    {
        for (size_t j = 0; j < M; j++)
        {
            if (i == 1)
            {
                next[0][j] = p->tinit[(i - 1) * M + j];
            }
            if (i == N)
            {
                next[N + 1][j] = p->tinit[(i - 1) * M + j];
            }
            next[i][j] = p->tinit[(i - 1) * M + j];
            cond[i][j] = p->conductivity[(i - 1) * M + j];
        }
    }

    /* Get start time */
    gettimeofday(&start, 0);

    /* Start timesteps */
    for (int step = 0; step < p->maxiter; ++step)
    {
        /* Copy next into current */
        memcpy(current, next, sizeof(double) * num_bodies);
        // #pragma GCC ivdep
        for (int i = 1; i < N + 1; i++)
        {
            // #pragma GCC ivdep
            for (int j = 0; j < M; j++)
            {
                next[i][j] = cond[i][j] * current[i][j];
                double inf = (1 - cond[i][j]);

                double total_inf_strong = strong_inf * inf;
                double total_inf_weak = weak_inf * inf;

                const __m128 strong_scalar = _mm_set1_ps(total_inf_strong);
                const __m128 weak_scalar = _mm_set1_ps(total_inf_weak);
                __m128 myVector1 = _mm_set_ps(current[(i + 1)][j], current[(i - 1)][j], current[i][(j + 1) % M], current[i][(j - 1 + M) % M]);
                __m128 outcome = _mm_mul_ps(myVector1, strong_scalar);
                float result_strong[4];
                _mm_store_ps(result_strong, outcome);

                myVector1 = _mm_set_ps(current[(i - 1)][(j - 1 + M) % M], current[(i + 1)][(j - 1 + M) % M], current[(i - 1)][(j + 1) % M], current[(i + 1)][(j + 1) % M]);
                outcome = _mm_mul_ps(myVector1, weak_scalar);
                float result_weak[4];
                _mm_store_ps(result_weak, outcome);

                for (int k = 0; k < 4; k++)
                {
                    next[i][j] += result_strong[k];
                    next[i][j] += result_weak[k];
                }
                // printf("before next[i][j] = %f\n", next[i][j]);
                // next[i][j] += strong_inf * inf * current[(i + 1)][j];
                // next[i][j] += strong_inf * inf * current[(i - 1)][j];
                // next[i][j] += strong_inf * inf * current[i][(j + 1) % M];
                // next[i][j] += strong_inf * inf * current[i][(j - 1 + M) % M];
                // printf("after next[i][j] = %f\n", next[i][j]);
                /* weak neighbors */
                // next[i][j] += weak_inf * inf * current[(i - 1)][(j - 1 + M) % M];
                // next[i][j] += weak_inf * inf * current[(i + 1)][(j - 1 + M) % M];
                // next[i][j] += weak_inf * inf * current[(i - 1)][(j + 1) % M];
                // next[i][j] += weak_inf * inf * current[(i + 1)][(j + 1) % M];
            }
        }
        if ((step + 1) % p->printreports == 0 || p->maxiter - 1 == step)
        {
            /* Get end time and print intermediate step */
            gettimeofday(&end, 0);
            rtime = (end.tv_sec + (end.tv_usec / 1000000.0)) -
                    (start.tv_sec + (start.tv_usec / 1000000.0));
            compute_results(&p, r, step + 1, M, N, &current, &next, rtime);
            report_results(&p, r);
        }
    }
}

void compute_results(const struct parameters *p, struct results *r, int k, int M, int N, double t_array[N][M], double t_array_new[N][M], double rtime)
{
    r->niter = k;
    double tmin = LONG_MAX;
    double tmax = LONG_MIN;
    double t_tot = 0;
    double max_diff = 0;
    double current_value = 0.;

    // #pragma GCC ivdep
    for (int i = 1; i < N + 1; i++)
    {
        // #pragma GCC ivdep
        for (int j = 0; j < M; j++)
        {
            /* Get current value first, as this is faster than calling the array again */
            current_value = t_array_new[i][j];
            t_tot += current_value;
            if (current_value > tmax)
                tmax = current_value;
            if (current_value < tmin)
                tmin = current_value;
            /* Calculate maximal difference */
            if (fabs(t_array_new[i][j] - t_array[i][j]) > max_diff)
                max_diff = fabs(t_array_new[i][j] - t_array[i][j]);
        }
    }
    r->time = rtime;
    r->niter = k;
    r->tmin = tmin;
    r->tmax = tmax;
    r->tavg = t_tot / (N * M);
    r->maxdiff = max_diff;
}

// /* make movie */
// begin_picture (step + 1 , p->M , p->N , p->io_tmin , p->io_tmax);
// for (size_t i = 1; i < N + 1; i++)
// {
//         for (size_t j = 0; j < M; j++)
//         {
//             draw_point (j , i - 1, next[i][j]);
//         }
//     }
// end_picture();
