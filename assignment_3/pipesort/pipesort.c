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
#include <limits.h>

/* States of a comparator thread */
typedef enum
{
    COMPARE,
    COMPARE_NEW,
    END
} comparator_state;

typedef struct thread_parameters
{
    int *buffer;
    sem_t *occupied;
    sem_t *empty;
    int thread_id;

} thread_parameters;

int buffer_length;
int sequence_length;
pthread_t *threads;
sem_t pipesort_done;
char debug = 1;

int send_val(int val, int next_in_outbuf,
             int *buffer_out, sem_t *occupied, sem_t *empty)
{
    sem_wait(empty);
    buffer_out[next_in_outbuf] = val;
    next_in_outbuf = ++next_in_outbuf % buffer_length;
    sem_post(occupied);

    return next_in_outbuf;
}

void *output_thread(void *input)
{
    /* Unpack input thread parameters */
    thread_parameters *thread_param = (thread_parameters *)input;
    int *buffer_in = thread_param->buffer;
    sem_t *in_buffer_occupied = thread_param->occupied;
    sem_t *in_buffer_empty = thread_param->empty;
    int current_val = 0;
    int next_out = 0;
    int previous_val = -2;

    char first_end = 0;
    int correct = 0;

    if (debug) printf("\noutput:\n");

    while (1)
    {
        sem_wait(in_buffer_occupied);
        current_val = buffer_in[next_out];
        next_out = ++next_out % buffer_length;
        sem_post(in_buffer_empty);

        if (current_val == -1)
        {
            if (first_end == 0)
            {
                first_end = 1;
                continue;
            }
            else
            {
                sem_post(&pipesort_done);
                break;
            }
        }
        if (debug)
        {
            printf("%d ", current_val);
        }

        /* Check correctness */
        correct = previous_val > current_val ? 0 : 1;
        if (!correct)
        {
            break;
        }
    }
    printf("\n\nCorrect: %d\n", correct);
    sem_post(&pipesort_done);
}
void *comparator_thread(void *input)
{
    /* Unpack input thread parameters */
    thread_parameters *thread_param = (thread_parameters *)input;
    int *buffer_in = thread_param->buffer;
    int next_out = 0;
    sem_t *in_buffer_occupied = thread_param->occupied;
    sem_t *in_buffer_empty = thread_param->empty;

    /* Two values of comparator instance */
    int previous_val;
    int current_val;
    int temp_val;

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

    comparator_state STATE = COMPARE_NEW;

    sem_wait(in_buffer_occupied);
    previous_val = buffer_in[next_out];
    next_out = ++next_out % buffer_length;
    sem_post(in_buffer_empty);

    while (1)
    {
        // the error starts here were sem_wait cannot proceed
        /* Always read value from buffer_in first */
        sem_wait(in_buffer_occupied);
        current_val = buffer_in[next_out];
        next_out = ++next_out % buffer_length;
        sem_post(in_buffer_empty);

        if (STATE == COMPARE_NEW)
        {
            /* Create new thread parameters */
            thread_parameters new_params;
            new_params.buffer = buffer_out;
            new_params.occupied = out_buffer_occupied;
            new_params.empty = out_buffer_empty;
            new_params.thread_id = thread_param->thread_id + 1;

            /* Check for end state */
            if (current_val == -1)
            {
                /* End but still need to create a thread for printing*/
                STATE = END;
                pthread_create(&threads[0], NULL, output_thread, &new_params);

                /* Forward both values to next thread */
                next_in_outbuf = send_val(current_val, next_in_outbuf, buffer_out, out_buffer_occupied, out_buffer_empty);
                next_in_outbuf = send_val(previous_val, next_in_outbuf, buffer_out, out_buffer_occupied, out_buffer_empty);
                continue;
            }

            STATE = COMPARE;

            /* Create next comperator thread */
            pthread_create(&threads[0], NULL, comparator_thread, &new_params);

            temp_val = previous_val;
            previous_val = current_val < previous_val ? previous_val : current_val;
            current_val = current_val < temp_val ? current_val : temp_val;
            next_in_outbuf = send_val(current_val, next_in_outbuf, buffer_out, out_buffer_occupied, out_buffer_empty);
        }

        else if (STATE == COMPARE)
        {
            /* Check for end state */
            if (current_val == -1)
            {
                /* End but still need to create a thread */
                STATE = END;

                /* Forward both values to next thread */
                next_in_outbuf = send_val(current_val, next_in_outbuf, buffer_out, out_buffer_occupied, out_buffer_empty);
                next_in_outbuf = send_val(previous_val, next_in_outbuf, buffer_out, out_buffer_occupied, out_buffer_empty);
                continue;
            }
            temp_val = previous_val;
            previous_val = current_val < previous_val ? previous_val : current_val;
            current_val = current_val < temp_val ? current_val : temp_val;
            next_in_outbuf = send_val(current_val, next_in_outbuf, buffer_out, out_buffer_occupied, out_buffer_empty);
        }
        else if (STATE == END)
        {
            next_in_outbuf = send_val(current_val, next_in_outbuf, buffer_out, out_buffer_occupied, out_buffer_empty);
            if (current_val == -1)
            {
                break;
            }
        }
    }
}

void generator_thread(int *length)
{
    /* Create local buffer */
    int buffer_length = length;
    int *buffer = malloc(sizeof(int) * buffer_length);
    int buffer_location = 0;

    /* Initialize buffer locks (semaphores) */
    sem_t *occupied = malloc(sizeof(sem_t));
    sem_t *empty = malloc(sizeof(sem_t));
    sem_init(occupied, 0, 0);
    sem_init(empty, 0, buffer_length);

    /* Fill parameter struct and create first comparator */
    thread_parameters thread_param;
    thread_param.buffer = buffer;
    thread_param.occupied = occupied;
    thread_param.empty = empty;
    thread_param.thread_id = 1;
    pthread_create(&threads[0], NULL, (void *)comparator_thread, &thread_param);

    // if (debug) printf("\ninput:\n");
    size_t j;
    /* Generate nubmers and add to buffer */
    for (j = 0; j < sequence_length; j++)
    {
        sem_wait(empty);
        buffer[buffer_location] = rand();
        // if (debug) printf("%d\n", buffer[buffer_location]);
        buffer_location = ++buffer_location % buffer_length;
        sem_post(occupied);
    }

    /* Generate ends */
    for (size_t i = 0; i < 2; i++)
    {
        sem_wait(empty);
        buffer[buffer_location] = -1;
        buffer_location = ++buffer_location % buffer_length;
        sem_post(occupied);
    }
    // printf("\n");
}

int main(int argc, char *argv[])
{
    buffer_length = 1000;
    sequence_length = 2000;
    int c;
    int seed = 42;
    long length = 1;

    struct timespec before;
    struct timespec after;

    /* Read command-line options. */
    while ((c = getopt(argc, argv, "b:l:s:d")) != -1)
    {
        switch (c)
        {
        case 's':
            seed = atoi(optarg);
            break;
        case 'b':
            buffer_length = atoi(optarg);
            break;
        case 'l':
            sequence_length = atoi(optarg);
            break;
        case 'd':
            debug = 1;
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

    /* Create array of threads */
    threads = malloc(sizeof(pthread_t) * length);

    clock_gettime(CLOCK_MONOTONIC, &before);

    /* Create generator thread */
    pthread_create(&threads[0], NULL, (void *)generator_thread, buffer_length);

    /* Wait for generator to finish */
    sem_wait(&pipesort_done);

    clock_gettime(CLOCK_MONOTONIC, &after);

    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;
    printf("Pipesort took: % .6e seconds \n", time);
    return 0;
}