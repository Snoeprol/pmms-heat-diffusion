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
typedef enum
{
    INIT,
    COMPARE,
    COMPARE_NEW,
    END
} comparator_state;

typedef struct thread_parameters
{
    int *buffer;
    int *next_in;
    int *next_out;
    sem_t *occupied;
    sem_t *empty;

} thread_parameters;

int buffer_length = 5;
int sequence_length = 10;
pthread_t *threads;
sem_t pipesort_done;

int send_largest_value(int current_val, int previous_val, int next_in_outbuf,
                       int *buffer_out, sem_t *occupied, sem_t *empty)
{
    int forward_value = current_val > previous_val ? current_val : previous_val;
    printf("Sending value %d right now\n", forward_value);
    sem_wait(empty);
    buffer_out[next_in_outbuf] = forward_value;
    next_in_outbuf = ++next_in_outbuf % buffer_length;
    sem_post(occupied);
    printf("Sent value %d \n", forward_value);

    return next_in_outbuf;
}

void *pipeline_print(void *p) {
    int x = (int*) p;
    printf("Pipeline finished, received %d\n", x);
}

void *comparator(void *input)
{
    /* Unpack input thread parameters */
    thread_parameters *thread_param = (thread_parameters *)input;
    int *buffer_in = thread_param->buffer;
    // TODO or get nextout from struct ?
    int next_out = 0;
    sem_t *in_buffer_occupied = thread_param->occupied;
    sem_t *in_buffer_empty = thread_param->empty;

    /* Two values of comparator instance */
    int previous_val;
    int current_val;

    /* Create output buffer and its semaphores */
    int *buffer_out = malloc(sizeof(int) * buffer_length);
    sem_t *out_buffer_occupied = malloc(sizeof(sem_t));
    sem_t *out_buffer_empty = malloc(sizeof(sem_t));
    if (sem_init(out_buffer_occupied, 0, 0))
        printf("Seminit error 1");
    if (sem_init(out_buffer_empty, 0, buffer_length))
        printf("Seminit error 2");
    int next_in_outbuf = 0; // In the slides they have a next in and next out int he buffcer parameter
                            // But i'm not sure if that's necessary because it can be included during sending

    comparator_state STATE = INIT;

    while (STATE != END)
    {
        /* Always read value from buffer_in first */
        sem_wait(in_buffer_occupied);
        current_val = buffer_in[next_out];
        printf("Current val = %d\n", current_val);
        next_out = ++next_out % buffer_length;
        sem_post(in_buffer_empty);

        if (STATE == INIT)
        {
            previous_val = current_val;
            STATE = COMPARE_NEW;
            printf("Set state from INIT to COMPARE_NEW with new value %d\n", current_val);
        }

        else if (STATE == COMPARE_NEW)
        {
            /* Check for end state */
            if (current_val == -1)
            {
                /* End but still need to create a thread */
                STATE = END;
                pipeline_print((void*) 5);
                pthread_create(&threads[0], NULL, pipeline_print, current_val);


                /* Forward both values to next thread */
                next_in_outbuf = send_largest_value(current_val, current_val, next_in_outbuf, buffer_out, out_buffer_occupied, out_buffer_empty);
                next_in_outbuf = send_largest_value(previous_val, previous_val, next_in_outbuf, buffer_out, out_buffer_occupied, out_buffer_empty);
                continue;

            }
            printf("Setting state to COMPARE \n");
            STATE = COMPARE;
            /* Create new thread parameters */
            thread_parameters new_params;
            new_params.buffer = buffer_out;
            new_params.occupied = out_buffer_occupied;
            new_params.empty = out_buffer_empty;
            new_params.next_in = next_in_outbuf;

            /* Create next comperator thread */
            pthread_create(&threads[0], NULL, comparator, &new_params);

            next_in_outbuf = send_largest_value(current_val, previous_val, next_in_outbuf, buffer_out, out_buffer_occupied, out_buffer_empty);
            previous_val = current_val > previous_val ? previous_val : current_val;
        }

        else if (STATE == COMPARE) {
            /* Check for end state */
            if (current_val == -1)
            {
                /* End but still need to create a thread */
                STATE = END;

                /* Forward both values to next thread */
                next_in_outbuf = send_largest_value(current_val, current_val, next_in_outbuf, buffer_out, out_buffer_occupied, out_buffer_empty);
                next_in_outbuf = send_largest_value(previous_val, previous_val, next_in_outbuf, buffer_out, out_buffer_occupied, out_buffer_empty);
                continue;
            }
            next_in_outbuf = send_largest_value(current_val, previous_val, next_in_outbuf, buffer_out, out_buffer_occupied, out_buffer_empty);

        }
        else if (STATE == END) {
            printf("Reached end state");
            pthread_create(&threads[0], NULL, pipeline_print, current_val);
        }

    }
    sem_destroy(in_buffer_empty);
    sem_destroy(in_buffer_occupied);
    // free(empty);
    // free(occupied);
    printf("While loop finsihed\n");
    sem_post(&pipesort_done);
}

void generate_numbers(int *length)
{
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
    pthread_create(&threads[0], NULL, (void *)comparator, &thread_param);

    /* Generate nubmers and send? */
    for (size_t i = 0; i < sequence_length; i++)
    {
        printf("Generator: I've put %d in buffer\n", i);
        sem_wait(empty);
        buffer[buffer_location] = rand();
        buffer_location = ++buffer_location % buffer_length;
        sem_post(occupied);
    }

    /* Generate ends */
    for (size_t i = 0; i < 2; i++)
    {
        sem_wait(empty);
        buffer[buffer_location] = -1;
        buffer_location = ++buffer_location % buffer_length;
        sem_wait(occupied);
        printf("added END to buffer\n");
    }
}

int main(int argc, char *argv[])
{
    int c;
    int seed = 42;
    long length = 1e4;

    struct timespec before;
    struct timespec after;

    /* Read command-line options. */
    while ((c = getopt(argc, argv, "b:l:s:")) != -1)
    {
        switch (c)
        {
        case 's':
            seed = atoi(optarg);
            break;
        case 'b':
            sequence_length = atoi(optarg);
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

    sem_init(&pipesort_done, 0, 0);
    /* Seed such that we can always reproduce the same random vector */
    srand(seed);

    /*Create array of threads */
    threads = malloc(sizeof(pthread_t) * length);

    clock_gettime(CLOCK_MONOTONIC, &before);

    pthread_create(&threads[0], NULL, (void *)generate_numbers, buffer_length);

    // pthread_join(threads[0], NULL);
    // /* Wait for first thread to finish */
    // for (int id = 1; id < 2; id++)
    // {
    //     pthread_join(threads[id], NULL);
    // }
    // Check if join is better than another semaphore
    sem_wait(&pipesort_done);

    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;
    printf("Pipesort took: % .6e seconds \n", time);
    return 0;
}