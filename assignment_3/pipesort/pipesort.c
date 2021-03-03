#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <getopt.h>
#include <ctype.h>
#include <errno.h>

typedef struct thread_parameters{
    int *buffer;
    // add some lock to the buffer
} thread_parameters;

int buffer_size = 5;
pthread_t *threads;

void test_function(int i) {
    printf("Test function printing\n");
}

void generate_numbers(int *length) {
    int buffer_length = length;
    printf("Creating buffer of length = %d\n", buffer_length);

    int *buffer = malloc(sizeof(int) * buffer_length);
    int buffer_location = 0;


    /* Create first comparator */
    // Maybe this should be after number generation but i think it's better to have the thread running and then "send" a number
    int i = 420;
    pthread_create(&threads[1], NULL, test_function, i);

    /*Generate nubmers and send?*/
    for (size_t i = 0; i < buffer_length; i++)
    {
        buffer[buffer_location] = rand();
        buffer_location = ++buffer_location % buffer_length;
    }

    /* Generate ends */
    for (size_t i = 0; i < buffer_length; i++)
    {
        buffer[buffer_location] = -1;
        buffer_location = ++buffer_location % buffer_length;
    }





}

int main(int argc, char *argv[]){
    int c;
    int seed = 42;
    long length = 1e4;

    struct timespec before;
    struct timespec  after;



    /* Read command-line options. */
    while((c = getopt(argc, argv, "b:l:s:")) != -1) {
        switch(c) {
            case 'l':
                length = atol(optarg);
                break;
            case 's':
                seed = atoi(optarg);
                break;
            case 'b':
                buffer_size = atoi(optarg);
            case '?':
            if (optopt == 'b' ||
                optopt == 'l' ||
                optopt == 's')
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

    /*Create array of threads */
    threads = malloc(sizeof(pthread_t) * length);

    clock_gettime(CLOCK_MONOTONIC, &before);

    pthread_create(&threads[0], NULL, generate_numbers, length);


    /* Wait for first thread to finish */
    pthread_join(threads[0], NULL);







    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    printf("Pipesort took: % .6e seconds \n", time);

}