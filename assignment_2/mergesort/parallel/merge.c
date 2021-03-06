#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <omp.h>
#include <string.h>

/* Ordering of the vector */
typedef enum Ordering
{
    ASCENDING,
    DESCENDING,
    RANDOM
} Order;

int debug = 0;

void topDownMerge(int *vector, long low, long mid, long high, int *vector_work)
{
    long i = low;
    long j = mid;

    for (long k = low; k < high; k++)
    {
        if (i < mid && (j >= high || vector[i] <= vector[j]))
        {
            vector_work[k] = vector[i];
            i++;
        }
        else
        {
            vector_work[k] = vector[j];
            j++;
        }
    }
}

void sequential_topDownSplitMerge(int *vector_work, long low, long high, int *vector)
{
    if (high - low <= 1)
        return;

    long mid = (high + low) / 2;
    sequential_topDownSplitMerge(vector, low, mid, vector_work);  // Sort left part (recursively)
    sequential_topDownSplitMerge(vector, mid, high, vector_work); // And right part

    /* Merge after recursive calls for left and right part */
    topDownMerge(vector_work, low, mid, high, vector);
}

void topDownSplitMerge(int *vector_work, long low, long high, int *vector)
{
    if (high - low <= 2000)
    {
        sequential_topDownSplitMerge(vector_work, low, high, vector);
        return;
    }

    long mid = (high + low) / 2;
#pragma omp task shared(vector_work, vector) firstprivate(low, mid)
    topDownSplitMerge(vector, low, mid, vector_work); // Sort left part (recursively)

#pragma omp task shared(vector_work, vector) firstprivate(mid, high)
    topDownSplitMerge(vector, mid, high, vector_work); // And right part

#pragma omp taskwait
    /* Merge left and right part */
    topDownMerge(vector_work, low, mid, high, vector);
}

/* Sort vector v of l elements using mergesort */
void msort(int *vector, long length, int num_threads)
{
    /* Create a work array vector_work and copy v into it*/
    int *vector_work = (int *)malloc(length * sizeof(int));
    memcpy(vector_work, vector, length * sizeof(int));
// I'm not sure if we need to specify the number of threads in the pragma below
// As we already do this in main "towards openmp"
#pragma omp parallel shared(vector, vector_work, length) num_threads(num_threads)
    {
#pragma omp master
        {
            topDownSplitMerge(vector_work, 0, length, vector);
        };
    }
    topDownMerge(vector_work, 0, (int) length/2, length, vector);
}

void print_v(int *v, long l)
{
    printf("\n");
    for (long i = 0; i < l; i++)
    {
        if (i != 0 && (i % 10 == 0))
        {
            printf("\n");
        }
        printf("%d ", v[i]);
    }
    printf("\n");
}

int main(int argc, char **argv)
{

    int c;
    int seed = 42;
    long length = 1e4;
    int num_threads = 1;
    Order order = ASCENDING;
    int *vector;

    struct timespec before, after;

    /* Read command-line options. */
    while ((c = getopt(argc, argv, "adrgp:l:s:")) != -1)
    {
        switch (c)
        {
        case 'a':
            order = ASCENDING;
            break;
        case 'd':
            order = DESCENDING;
            break;
        case 'r':
            order = RANDOM;
            break;
        case 'l':
            length = atol(optarg);
            break;
        case 'g':
            debug = 1;
            break;
        case 's':
            seed = atoi(optarg);
            break;
        case 'p':
            num_threads = atoi(optarg);
            break;
        case '?':
            if (optopt == 'l' || optopt == 's')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            }
            else if (isprint(optopt))
            {
                fprintf(stderr, "Unknown option '-%c'.\n", optopt);
            }
            else
            {
                fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
            }
            return -1;
        default:
            return -1;
        }
    }

    /* Seed such that we can always reproduce the same random vector */
    srand(seed);

    /* Allocate vector. */
    vector = (int *)malloc(length * sizeof(int));
    if (vector == NULL)
    {
        fprintf(stderr, "Malloc failed...\n");
        return -1;
    }

    /* Fill vector. */
    switch (order)
    {
    case ASCENDING:
        for (long i = 0; i < length; i++)
        {
            vector[i] = (int)i;
        }
        break;
    case DESCENDING:
        for (long i = 0; i < length; i++)
        {
            vector[i] = (int)(length - i);
        }
        break;
    case RANDOM:
        for (long i = 0; i < length; i++)
        {
            vector[i] = rand();
        }
        break;
    }

    if (debug)
    {
        print_v(vector, length);
    }

    /* override the value of the environment variable OMP_NUM_THREADS */
    omp_set_num_threads(num_threads);

    clock_gettime(CLOCK_MONOTONIC, &before);

    /* Sort */
    msort(vector, length, num_threads);

    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    printf("Mergesort took: % .6e seconds \n", time);

    if (debug)
    {
        print_v(vector, length);
    }

    return 0;
}
