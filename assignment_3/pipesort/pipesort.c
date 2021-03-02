#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#define SIZE_OF_BUFFER 8

int counter ;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread;

typedef struct Program_params{
    int seed, length;
} Program_params;

typedef struct Params
{
    sem_t * p_lock;
    sem_t * c_lock;
    sem_t * occupied;
    sem_t * empty;
    int length, seed, next_out, next_in;
    Buffer * buffer;
} Params;

typedef struct Buffer
{
    /* Fill initial buffer */
    int numbers[SIZE_OF_BUFFER];
    int nextin;
    int nextout;
} Buffer;

void * pipsesort(void * params_pipe){
    Program_params * params_point = (Program_params *) params_pipe;
    Program_params params = * params_point;

    int end = 0;
    int i;
    int num_threads = 1;
    pthread_t thread_ids[params.length + 2];

    pthread_create (
        &thread , // OUT : thread identifier
        NULL , // IN : thread attributes
        &buffer_maintain , // IN : code to execute
        &params); // IN : for start routine
    
}

buffer_maintain(void * params){

    Program_params * params_pointer = (Program_params *) params;
    Program_params parameters = * params_pointer;  


    /* Create buffer */
    Buffer buffer;
    buffer.nextin = 0;
    buffer.nextout = 0;
    
    /* Locks */
    sem_t p_lock;
    sem_t c_lock;
    sem_t occupied;
    sem_t empty;

    sem_init ( & p_lock , 0 , 1);
    sem_init ( & c_lock , 0 , 1);
    sem_init ( & occupied , 0, 0);
    sem_init ( & empty , 0, SIZE_OF_BUFFER );

    Params parameters_comp;
    parameters_comp.p_lock = &p_lock;
    parameters_comp.c_lock = &c_lock;
    parameters_comp.occupied = &occupied;
    parameters_comp.empty = &empty;
    parameters_comp.buffer = &buffer;

    pthread_create(&thread, NULL, compare, &parameters_comp);
    
    int val;
    for (int i = 0; i < parameters.length ; i++){
        val = rand();
        sem_wait(&empty);

        buffer.nextin = val;
        buffer.nextin = (buffer.nextin + 1) % SIZE_OF_BUFFER;

        sem_post(&occupied);
    }
}

void compare(void * params)
{   
    /* Convert void to actual params */
    Params * params_pointer = (Params *) params;
    Params parameters = * params_pointer;
    
    int temp;
    int max;

    int done = 0;
    int state = 0;
    int val;
    sem_t * occupied = &parameters.occupied;
    sem_t * c_lock = &parameters.c_lock;
    /* Define states */
        while (! done){
            switch (state){
                /* Initial state */
                case 0:
                    sem_wait(&occupied);
                    sem_wait(&c_lock);

                    val = parameters.buffer.numbers[parameters.buffer->nextout];
                    parameters.buffer->nextout = (parameters.buffer->nextout + 1) % SIZE_OF_BUFFER;

                    sem_post(&c_lock);
                    sem_post(&occupied);

                    max = val;
                    state = 1;

                case 1:

                    sem_wait(&occupied);
                    sem_wait(&c_lock);

                    val = parameters.buffer->numbers[parameters.buffer->nextout];
                    parameters.buffer->nextout = (parameters.buffer->nextout + 1) % SIZE_OF_BUFFER;

                    sem_post(&c_lock);
                    sem_post(&occupied);

                    temp = val;

                    if (max > temp) {
                        pthread_create (
                            &thread , // OUT : thread identifier
                            NULL , // IN : thread attributes
                            &buffer_maintain , // IN : code to execute
                            &params); // IN : for start routine

                     
                    state = 1;                    

                    /* if got end, move to state 2 */
                    state = 2;               
                case 2:
                    state = 3;
                case 3:

                    /* Do more stuff */
                    done = 1;
            }
        }

}


int main(int argc, char *argv[]){
    int c;
    int seed = 42;
    long length = 1e4;

    struct timespec before;
    struct timespec  after;



    /* Read command-line options. */
    while((c = getopt(argc, argv, "l:s:")) != -1) {
        switch(c) {
            case 'l':
                length = atol(optarg);
                break;
            case 's':
                seed = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                return -1;
            default:
                return -1;
        }
    }

    /* Seed such that we can always reproduce the same random vector */
    srand(seed);

    clock_gettime(CLOCK_MONOTONIC, &before);
    /* Do your thing here */
    
    Program_params p;
    p.length = length;
    p.seed = seed;
    
    pipsesort(&p);

    /* Do your thing here */
    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    printf("Pipesort took: % .6e seconds \n", time);

}
