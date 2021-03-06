#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include "semaphore.h"

sem_t hist_lock[256];
pthread_barrier_t barrier; // the barrier synchronization object

void die(const char *msg){
    if (errno != 0) 
        perror(msg);
    else
        fprintf(stderr, "error: %s\n", msg);
    exit(1);
}   

void generate_image(int num_rows, int num_cols, int * image){
    for (int i = 0; i < num_cols * num_rows; ++i)
    {
        image[i] = rand() % 256; //255 + 1 for num bins
    }
}

void read_image(const char * image_path, int num_rows, int num_cols, int * image){
	char format[3];
    FILE *f;
    unsigned imgw, imgh, maxv, v;
    size_t i;

	printf("Reading PGM data from %s...\n", image_path);

	if (!(f = fopen(image_path, "r"))) die("fopen");

	fscanf(f, "%2s", format);
    if (format[0] != 'P' || format[1] != '2') die("only ASCII PGM input is supported");
    
    if (fscanf(f, "%u", &imgw) != 1 ||
        fscanf(f, "%u", &imgh) != 1 ||
        fscanf(f, "%u", &maxv) != 1) die("invalid input");

    if (imgw != num_cols || imgh != num_rows) {
        fprintf(stderr, "input data size (%ux%u) does not match cylinder size (%zux%zu)\n",
                imgw, imgh, num_cols, num_rows);
        die("invalid input");
    }

    for (i = 0; i < num_cols * num_rows; ++i)
    {
        if (fscanf(f, "%u", &v) != 1) die("invalid data");
        image[i] = ((int)v * 255) / maxv; //255 for num bins
    }
    fclose(f);
}

void print_histo(int * histo){
	for (int i = 0; i < 256; ++i)
	{	
		if(i != 0 && (i % 10 == 0)) {
            printf("\n");
        }
		printf("%d ", histo[i]);
	}
    printf("\n");
}

void print_image(int num_rows, int num_cols, int * image){
	int index = 0;
	for (int i = 0; i < num_rows; ++i){	
		for (int j = 0; j < num_cols; ++j){
	        index = i * num_cols + j;
			printf("%d ", image[index]);
		}
	}
	printf("\n");
}

typedef struct Params{
    int start_element, end_element;
    void * img;
    void * histo;
} Params;

void * do_part(void * params){

    /* Convert void to actual params */
    Params * params_pointer = (Params *) params;
    Params parameters = * params_pointer;
    int (*hist)[256] = (int (*)[256])parameters.histo;
    int start_element = parameters.start_element;
    int end_element = parameters.end_element;
    int (*image_arr)[] = (int (*)[])parameters.img;


    for (int i = start_element; i < end_element; i++){
        sem_wait (&hist_lock[(*image_arr)[i]]);
        (*hist)[(*image_arr)[i]] += 1;
        sem_post(&hist_lock[(*image_arr)[i]]);
    }
    int hist_sum = 0;
    for (int i = 0; i < 256; i++){
    hist_sum += (*hist)[i];
    }
    printf("hist sum: %i\n", hist_sum);
    pthread_barrier_wait(&barrier);
}

void histogram(int * histo, int * image, int threads, int elems){
    /* Create bins and set to zero */
    void * results[threads];
    pthread_t thread_ids [threads];
    for (int i = 0; i < 256; i++)
    {
        sem_init(&hist_lock[i] , 1 , 1);
    }
    pthread_barrier_init (&barrier, NULL, threads + 1);
    int elems_per_thread = (int) (elems/threads) + 1;
    int start_element;
    int end_element;
    /* Barrier */
    pthread_barrier_init (&barrier, NULL, threads + 1);

    Params params[threads];
    for (int i = 0; i < threads; i++){
        //printf("%i\n", i);
        start_element = i * elems_per_thread;
        if (i != threads - 1){
            end_element = (i + 1) * elems_per_thread;  
            //printf("after if statement %i\n", end_element);
        } else {
            end_element = elems;
        }
    
        /* Maybe check if last thread has elements */
        params[i].start_element = start_element;
        params[i].end_element = end_element;
        params[i].img = image;
        params[i].histo = histo;
        void * input_pointer = &params[i];
        
        /* Create Ze Tread */
        pthread_create(&thread_ids[i], NULL, &do_part, input_pointer);
    }

    pthread_barrier_wait (&barrier);
    // for (int i = 0; i < threads; i ++) {
    //     pthread_join(thread_ids[i] , &results[i]);
    // }
    int pixels = 0;
    for (int j = 0; j < 256; j ++){
        pixels += histo[j];
    }

    printf("pixesl: %i\n", pixels);
}

int main(int argc, char *argv[]){
    int c;
    int seed = 42;
    const char *image_path = 0;
    image_path ="../../../../images/pat1_100x150.pgm";
    int gen_image = 0;
    int debug = 0;
    int threads = 4;
    int num_rows = 150;
    int num_cols = 100;

    struct timespec before, after;

    int * histo = (int *) calloc(256, sizeof(int));

    /* Read command-line options. */
    while((c = getopt(argc, argv, "s:i:rp:n:m:g")) != -1) {
        switch(c) {
            case 's':
                seed = atoi(optarg);
                break;
            case 'i':
            	image_path = optarg;
            	break;
            case 'r':
            	gen_image = 1;
            	break;
            case 'p':
                threads = atoi(optarg);
                break;
            case 'n':
            	num_rows = strtol(optarg, 0, 10);
            	break;
            case 'm':
				num_cols = strtol(optarg, 0, 10);
				break;
			case 'g':
				debug = 1;
				break;
            case '?':
                fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                return -1;
            default:
                return -1;
        }
    }
    int * image = (int *) malloc(sizeof(int) * num_cols * num_rows);

    /* Seed such that we can always reproduce the same random vector */
    if (gen_image){
    	srand(seed);
    	generate_image(num_rows, num_cols, image);
    }else{
    	read_image(image_path,num_rows, num_cols, image);
    }

    clock_gettime(CLOCK_MONOTONIC, &before);
    /* Do your thing here */

    histogram(histo, image, threads, num_rows * num_cols);

    /* Do your thing here */

    if (debug){
    	print_histo(histo);
    }

    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    printf("Histo took: % .6e seconds \n", time);
}
