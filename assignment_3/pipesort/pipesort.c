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
#include "semaphore.h"

/* States of a comparator thread */
typedef enum {INIT, COMPARE, COMPARE_NEW, END} comparator_state;

typedef struct thread_parameters{
    int *buffer;
    int *next_in;
    int *next_out;
    sem_t *occupied;
    sem_t *empty;

} thread_parameters;

int buffer_length = 5;
pthread_t *threads;

void comparator(void* input) {
    /* Unpack input thread parameters */
    thread_parameters *thread_param = (thread_parameters*) input;
    int *buffer_in = thread_param->buffer;
    int *next_out = thread_param->next_out;
    sem_t *occupied = thread_param->occupied;
    sem_t *empty = thread_param->empty;

    int previous_val;
    int current_val;

    int *buffer_out = malloc(sizeof(int) * buffer_length);
    sem_t *out_buffer_occupied = malloc(sizeof(sem_t));
    sem_t *out_buffer_empty = malloc(sizeof(sem_t));
    sem_init(out_buffer_occupied, 0, 0);
    sem_init(out_buffer_empty, 0, buffer_length);
    int next_in = 0; // In the slides they have a next in and next out int he buffcer parameter
                     // But i'm not sure if that's necessary because it can be included during sending

    comparator_state STATE = INIT;


}

void generate_numbers(int *length) {
    /* Create local buffer */
    int buffer_length = length;
    printf("Creating buffer of length = %d\n", buffer_length);
    int *buffer = malloc(sizeof(int) * buffer_length);
    int buffer_location = 0;

    /* Initialize locks (we use semaphores) */
    sem_t *occupied = malloc(sizeof(sem_t));
    sem_t *empty = malloc(sizeof(sem_t));
    sem_init(occupied, 0, 0);
    sem_init(empty, 0, buffer_length);

    /* Fill parameter struct and create first comparator */
    thread_parameters thread_param;
    thread_param.buffer = buffer;
    thread_param.occupied = occupied;
    thread_param.empty = empty;
    pthread_create(&threads[1], NULL, comparator, &thread_param);

    /* Generate nubmers and send? */
    for (size_t i = 0; i < buffer_length; i++)
    {
        sem_wait(empty);
        buffer[buffer_location] = rand();
        buffer_location = ++buffer_location % buffer_length;
        sem_post(occupied);
    }

    /* Generate ends */
    for (size_t i = 0; i < buffer_length; i++)
    {
        // sem_wait(empty);
        buffer[buffer_location] = -1;
        buffer_location = ++buffer_location % buffer_length;
        // sem_wait(occupied);
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
            case 's':
                seed = atoi(optarg);
                break;
            case 'l':
                buffer_length = atoi(optarg);
                break;
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

    pthread_create(&threads[0], NULL, generate_numbers, buffer_length);


    /* Wait for first thread to finish */
    pthread_join(threads[0], NULL);


    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    printf("Pipesort took: % .6e seconds \n", time);

}