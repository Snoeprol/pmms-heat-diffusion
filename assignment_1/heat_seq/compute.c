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
    printf("%.2f, %.2f", tmin, tmax);
    /* Create arrays for next and current bodies and conductivities */
    double next[N + 2][M];
    double current[N + 2][M];
    double cond[N][M];
    double **output_t;

    /* Fill next and conductivity as 2D matrix */

    double min = 10E10;
    double max = -10E10;
    //printf("%.5f\n", p->tinit[150 * M + 0]);
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
            cond[i][j] = p->conductivity[(i-1) * M + j];
            //printf("%.2f\n", cond[i][j]);
            //printf("%.2f\n", next[i][j]);
            
            if (p->tinit[(i) * j] > max)
                max = p->tinit[(i) * j];
            if (p->tinit[(i) * j] < min)
                min = p->tinit[(i) * j];
        }
    }
    { 
    /*
    for (int i = 0; i < N + 2; i++) { 
        for (int j = 0; j < M; j++) {
                printf("Value for [%d][%d] is %.5f: \n",i,j, next[i][j]); 
        } 
    } 
    */
    } 

    printf("max: %.2f\n", max);
    printf("min: %.2f\n", min);

    /* Do results have to be transformed from range 0-100?
    for (size_t i = 0; i < N; i++)
    {
        for (size_t j = 0; j < M; j++)
        {   
            next[i][j] = tmin + (next[i][j] - min)/ (max - min) * (tmax - tmin);
            //printf("%.2f\n", next[i][j]);
        }
    }
    */

    /* Get a sexy pic */
    begin_picture (0 , p->M , p->N , p->io_tmin , p->io_tmax);
    for (size_t i = 1; i < N; i++)
    {
            for (size_t j = 0; j < M; j++)
            {
                draw_point (j , i - 1, next[i][j]);
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
        /* Check if copy works
        int i;
        int j ;
        for(i=0;i<N+2;i++){
            for(j=0;j<M;j++){
            if(current[i][j]!=next[i][j])
                printf("copy failed %.5f and %.5f\n", current[i][j], next[i][j]);
            }
        }
        */
        //compute_results(&p, r, step, M, N, &current, rtime);
        for (int i = 1; i < N + 1; i++)
        {
            for (int j = 0; j < M; j++)
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
    compute_results(&p, r, p->maxiter, M, N, &current, &next, rtime);

    /* Get a sexy pic */
    begin_picture (42 , p->M , p->N , p->io_tmin , p->io_tmax);
    for (size_t i = 1; i < N; i++)
    {
            for (size_t j = 0; j < M; j++)
            {
                draw_point (j , i - 1, next[i][j]);
            }
        }
    end_picture();

}

void compute_results(const struct parameters *p, struct results *r, int k, int M, int N,  double t_array[N][M], double t_array_new[N][M], double rtime) {
    r->niter = k;
    double tmin = 10E8;
    double tmax = -10E8;
    double t_tot = 0;
    double max_diff = 0;

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            double current_value = t_array[i][j];
            t_tot += t_array[i][j];
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

void check_convergence(int num_bodies, double * t_current, double * t_next, double epsilon){
    for (int i = 0; i < num_bodies; i++)
    {
        for (int j = 1; j < num_bodies; j++)
        {
            if (fabs(t_current[i] - t_next[j]) < epsilon)
                return 1;
        }
    }    
    return 0;
}