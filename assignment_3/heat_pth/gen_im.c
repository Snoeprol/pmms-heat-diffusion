// Write .pgm images
#include <string.h>
#include <stdio.h>
#include <time.h> 
#include <stdlib.h>
#include <string.h>

int get_random_int(int lower, int upper)
{ 
    return (rand() % (upper - lower + 1)) + lower;
} 

int main(int argc, char **argv)
{ 
    char * n = argv[1];
    char * m = argv[2];
    int n_int = atoi(n);
    int m_int = atoi(m);
    int max_val = 65535;   
    // Suppose the 2D Array to be converted to Image is as given below 
    int image[n_int][m_int];
    int i, j;
    for (i = 0; i < n_int; i++)
    {
        for (j = 0; j < m_int; j++)
        {
            image[i][j] = get_random_int(0, max_val);
        }
    }
  
    FILE* pgmimg;
    char file_name[15]; 
    sprintf(file_name, "%ix%i.pgm", m_int, n_int);
    pgmimg = fopen(file_name, "wb"); 
  
    // Writing Magic Number to the File 
    fprintf(pgmimg, "P2\n");  
  
    // write n and m
    fprintf(pgmimg, "%d %d\n", n_int, m_int);  
  
    // Writing the maximum gray value 
    fprintf(pgmimg, "65535\n");  
    int count = 0; 
    int temp;
    for (i = 0; i < n_int; i++) { 
        for (j = 0; j < m_int; j++) { 
            temp = image[i][j]; 
  
            // Writing the gray values in the 2D array to the file 
            fprintf(pgmimg, "%d ", temp); 
        } 
        fprintf(pgmimg, "\n"); 
    } 
    fclose(pgmimg); 
} 
