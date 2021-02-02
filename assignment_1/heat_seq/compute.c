#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <input.h>

    // assume cond is a 2-D array filled with conductivities of the lattice
    // assume next is a 2-D array filled with heats at the next iteration
    // assume current is a 2-D array filled with current heat configuration
    // assume bound is the boundary value array (size (N, 2)) bound[x][0] is bottom

    // size_t M = p->M;
    // printf("--> %ld\n", M);

    // printf("%f ", next[(i+1)*j] );
    // printf("hey\n");

/* Define weak strong smth ??? */
const double weak_inf = 1 / 4 * (sqrt(2) + 1);
const double strong_inf = sqrt(2) / 4 * (sqrt(2) + 1);



void do_compute(const struct parameters* p, struct results *r)
{
    int M = p->M;
    int N = p->N;
    int num_bodies = M * N;

    /* Create arrays for next and current bodies and conductivities */
    double next[N][M];
    double current[N][M];
    double cond[N][M];

    /* Fill next and conductivity as 2D matrix */
    for (size_t i = 0; i < N; i++)
    {
        for (size_t j = 0; j < M; j++)
        {
            next[i][j] = p->tinit[(i + 1) * j];
            cond[i][j] = p->conductivity[(i + 1) * j];
        }

    }


    /* Start timesteps */

    /* Copy next into current */
    memcpy(current, next, sizeof(double) * num_bodies);

    for (int i = 0; i < N; i++)
    {
        for (int j = 1; j < M - 1; j++)
        {
            /* Make sure were not on top or bottom boundary */
            next[i][j] = cond[i][j] * current[i][j];
            double inf = 1 - cond[i][j];

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



/* final reporting */
struct results {
    size_t niter; /* effective number of iterations */
    double tmin; /* minimum temperature in last state */
    double tmax; /* maximum temperature in last state*/
    double maxdiff; /* maximum difference during last update */
    double tavg; /* average temperature */
    double time; /* compute time in seconds */
};