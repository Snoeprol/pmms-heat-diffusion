#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <input.h>
#include <time.h>
#include <output.h>

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
    int num_bodies = M * N;

    /* Create arrays for next and current bodies and conductivities */
    double next[N][M];
    double current[N][M];
    double cond[N][M];
    double **output_t;

    /* Fill next and conductivity as 2D matrix */

    double min = -100000;
    double max = 100000;
    
    for (size_t i = 0; i < N; i++)
    {
        for (size_t j = 0; j < M; j++)
        {   
            next[i][j] = p->tinit[i * M + j];
            cond[i][j] = p->conductivity[i * M + j];
            //printf("%.2f\n", next[i][j]);
            if (p->tinit[(i + 1) * j] > max)
                max = p->tinit[(i + 1) * j];
            if (p->tinit[(i + 1) * j] < min)
                min = p->tinit[(i + 1) * j];
        }
    }
    printf("max: %.2f\n", max);
    printf("min: %.2f\n", min);
    /* Get a sexy pic */
    begin_picture (0 , p->M , p->N , 40 , 100);
    for (size_t i = 0; i < N; i++)
    {
            for (size_t j = 0; j < M; j++)
            {
                draw_point (i , j , next[i][j]);
            }
        }
    end_picture();

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

        for (int i = 0; i < N; i++)
        {
            for (int j = 1; j < M - 1; j++)
            {
                //printf("next \n---->%.2f\n", current[i][j]);
                //printf("---->%.2f\n", next[i][j]);
                /* Make sure were not on top or bottom boundary */
                next[i][j] = cond[i][j] * current[i][j];
                double inf = (1 - cond[i][j]);

                // printf("%f ", next[i][j]);

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

    compute_results(&p, r, p->maxiter, M, N, &current, rtime);

    /* Get a sexy pic */
    begin_picture (42 , p->M , p->N , r->tmin , r->tmax);
    for (size_t i = 0; i < N; i++)
    {
            for (size_t j = 0; j < M; j++)
            {
                draw_point (i , j , current[i][j]);
            }
        }
    end_picture();

}

void compute_results(const struct parameters *p, struct results *r, int k, int M, int N,  double t_array[N][M], double rtime) {
    r->niter = k;
    double tmin = 10E8;
    double tmax = -10E8;
    double t_tot = 0;
    double max_diff = 0;

    for (int i = 0; i < N; i++)
    {
        for (int j = 1; j < M - 1; j++)
        {
            double current_value = t_array[i][j];
            t_tot += t_array[i][j];
            if (current_value > tmax)
                tmax = current_value;
            if (current_value < tmin)
                tmin = current_value;

            /* Logic for max_diff */
            for (int k = 0; k < N; k++)
            {
                for (int l = 1; l < M - 1; l++)
                {
                    double current_value_2 = t_array[k][l];
                    if (abs(current_value - current_value_2) > max_diff)
                        max_diff = abs(current_value - current_value_2);
                }
            }
        }
    }
    r->time = rtime;
    r->niter = k;
    r->tmin = tmin;
    r->tmax = tmax;
    r->tavg = t_tot / (N * M);
    r->maxdiff = max_diff;
}

