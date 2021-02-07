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

void do_compute(const struct parameters *p, struct results *r)
{
    /* Define weak strong influences */
    double weak_inf = 1 / (4 * (sqrt(2) + 1));
    double strong_inf = sqrt(2) / (4 * (sqrt(2) + 1));
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
    /* Init vars */
    double total_inf_strong;
    double total_inf_weak ;

    /* Init vectors */
    __m256d neigh_vec_s;
    __m256d neigh_vec_w;
    __m256d weak_scalar;
    __m256d strong_scalar;
    __m256d outcome_s;
    __m256d outcome_w;

    int step;
    /* Get start time */
    gettimeofday(&start, 0);
    double inf;
    double curr_cond;
    double new_val;
    /* Iterate timesteps */
    for (step = 0; step < p->maxiter; ++step)
    {
        /* Copy next into current */
        memcpy(current, next, sizeof(double) * num_bodies);
        // #pragma GCC ivdep
        for (int i = 1; i < N + 1; i++)
        {
            // #pragma GCC ivdep
            for (int j = 0; j < M; j++)
            {
                curr_cond = cond[i][j];
                new_val = curr_cond * current[i][j];
                inf = (1 - curr_cond);

                strong_scalar = _mm256_set1_pd(strong_inf * inf);
                weak_scalar = _mm256_set1_pd(weak_inf * inf);
                neigh_vec_s = _mm256_set_pd(current[(i + 1)][j], current[(i - 1)][j], current[i][(j + 1) % M], current[i][(j - 1 + M) % M]);
                neigh_vec_w = _mm256_set_pd(current[(i - 1)][(j - 1 + M) % M], current[(i + 1)][(j - 1 + M) % M], current[(i - 1)][(j + 1) % M], current[(i + 1)][(j + 1) % M]);
                outcome_s = _mm256_mul_pd(neigh_vec_s, strong_scalar);
                outcome_w = _mm256_mul_pd(neigh_vec_w, weak_scalar);

                for (int k = 0; k < 4; k++)
                {
                    new_val += outcome_s[k] + outcome_w[k];
                }
                next[i][j] = new_val;
            }
        }
        // if ((step + 1) % p->printreports == 0|| p->maxiter - 1 == step)
        // {
        //     /* Get end time and print intermediate step */
        //     gettimeofday(&end, 0);
        //     rtime = (end.tv_sec + (end.tv_usec / 1000000.0)) -
        //             (start.tv_sec + (start.tv_usec / 1000000.0));
        //     compute_results(&p, r, step + 1, M, N, &current, &next, rtime);
        //     report_results(&p, r);
        // }
    }
    gettimeofday(&end, 0);
    rtime = (end.tv_sec + (end.tv_usec / 1000000.0)) -
            (start.tv_sec + (start.tv_usec / 1000000.0));
    compute_results(&p, r, step, M, N, &current, &next, rtime);
}

void compute_results(const struct parameters *p, struct results *r, int k, int M, int N, double t_array[N][M], double t_array_new[N][M], double rtime)
{
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
