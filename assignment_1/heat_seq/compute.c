#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <input.h>
#include <time.h>
#include <output.h>
#include <string.h>
double rtime;
struct timeval start;
struct timeval end;

// assume cond is a 2-D array filled with conductivities of the lattice
// assume next is a 2-D array filled with heats at the next iteration
// assume current is a 2-D array filled with current heat configuration
// assume bound is the boundary value array (size (N, 2)) bound[x][0] is bottom

/* Define weak strong influences */
const double weak_inf = 1 / (4 * (sqrt(2) + 1));
const double strong_inf = sqrt(2) / (4 * (sqrt(2) + 1));

void do_compute(const struct parameters *p, struct results *r)
{
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
    double **output_t;

    /* Fill next and conductivity as 2D matrix */

    double min = 10E10;
    double max = -10E10;

    for (size_t i = 1; i < N + 1; i++)
    {
        for (size_t j = 0; j < M; j++)
        {   
            if (i == 1)
            {
                next[0][j] = p->tinit[(i-1) * M + j];
            }
            if (i == N)
            {
                next[N + 1][j] = p->tinit[(i-1) * M + j];
            }
            next[i][j] = p->tinit[(i-1) * M + j];
            cond[i][j] = p->conductivity[(i - 1) * M + j];
            
            if (p->tinit[(i - 1) * j] > max)
                max = p->tinit[(i - 1) * j];
            if (p->tinit[(i) * j] < min)
                min = p->tinit[(i - 1) * j];
        }
    }

    printf("max: %.2f\n", max);
    printf("min: %.2f\n", min);

    /* Get start time */
    if (gettimeofday(&start, 0) != 0)
    {
        fprintf(stderr, "could not do timing\n");
        exit(1);
    }

    /* Start timesteps */
    for (size_t step = 0; step < p->maxiter; ++step)
    {
        /* Copy next into current */
        memcpy(current, next, sizeof(double) * num_bodies);
        /* make movie */
        begin_picture (step + 1 , p->M , p->N , p->io_tmin , p->io_tmax);
        for (size_t i = 1; i < N + 1; i++)
        {
                for (size_t j = 0; j < M; j++)
                {
                    draw_point (j , i - 1, next[i][j]);
                }
            }
        end_picture();

        for (int i = 1; i < N + 1; i++)
        {
            for (int j = 0; j < M; j++)
            {
                next[i][j] = cond[i][j] * current[i][j];
                double inf = (1 - cond[i][j]);

                /* strong neighbors */
                next[i][j] += strong_inf * inf * current[(i + 1) % N][j];
                next[i][j] += strong_inf * inf * current[(i - 1 + N) % N][j];
                next[i][j] += strong_inf * inf * current[i][j + 1];
                next[i][j] += strong_inf * inf * current[i][j - 1];

                /* weak neighbors */
                next[i][j] += weak_inf * inf * current[(i - 1 + N) % N][j - 1];
                next[i][j] += weak_inf * inf * current[(i + 1) % N][j - 1];
                next[i][j] += weak_inf * inf * current[(i - 1 + N) % N][j + 1];
                next[i][j] += weak_inf * inf * current[(i + 1) % N][j + 1];
                
            }
        }
    }
    /* Get end time and print the duration */
    if (gettimeofday(&end, 0) != 0)
    {
        fprintf(stderr, "could not do timing\n");
        exit(1);
    }
    rtime = (end.tv_sec + (end.tv_usec / 1000000.0)) -
            (start.tv_sec + (start.tv_usec / 1000000.0));
    fprintf(stderr, "\n Simulation took %.3f seconds\n", rtime);
    compute_results(&p, r, p->maxiter, M, N, &current, &next, rtime);

}

void compute_results(const struct parameters *p, struct results *r, int k, int M, int N,  double t_array[N][M], double t_array_new[N][M], double rtime) {
    r->niter = k;
    double tmin = 10E10;
    double tmax = -10E10;
    double t_tot = 0;
    double max_diff = 0;

    for (int i = 1; i < N + 1; i++)
    {
        for (int j = 0; j < M; j++)
        {
            double current_value = t_array_new[i][j];
            t_tot += t_array_new[i][j];
            if (current_value > tmax)
                tmax = current_value;
            if (current_value < tmin)
                tmin = current_value;

            /* Logic for max_diff */
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
