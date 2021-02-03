            // else
            // {
            //     /* Split up in lower, and upper bounds */
            //     // Top case
            //     int k = 0;
            //     if (j > 0)
            //     {
            //         // strong neighbors
            //         next[i][j] += strong_inf * inf * current[(i + 1) % N][j];
            //         next[i][j] += strong_inf * inf * current[(i - 1 + N) % N][j];
            //         next[i][j] += strong_inf * inf * bound[i][1];
            //         next[i][j] += strong_inf * inf * current[i][j - 1];
            //         // weak neighbors
            //         next[i][j] += weak_inf * inf * current[(i - 1 + N) % N][j - 1];
            //         next[i][j] += weak_inf * inf * current[(i + 1) % N][j - 1];
            //         next[i][j] += weak_inf * inf * bound[(i - 1 + N) % N][1];
            //         next[i][j] += weak_inf * inf * bound[(i + 1) % N][1];
            //     }
            //     // Bottom case
            //     else
            //     {
            //         // strong neighbors
            //         next[i][j] += strong_inf * inf * current[(i + 1) % N][j];
            //         next[i][j] += strong_inf * inf * current[(i - 1 + N) % N][j];
            //         next[i][j] += strong_inf * inf * current[i][j + 1];
            //         next[i][j] += strong_inf * inf * bound[i][0];
            //         // weak neighbors
            //         next[i][j] += weak_inf * inf * bound[(i - 1 + N) % N][0];
            //         next[i][j] += weak_inf * inf * bound[(i + 1) % N][0];
            //         next[i][j] += weak_inf * inf * current[(i - 1 + N) % N][j + 1];
            //         next[i][j] += weak_inf * inf * current[(i + 1) % N][j + 1];
            //     }
/* Trash when using the cond of neighbors
                /*
                next[i][j] += strong_inf * (1 - cond[(i + 1) % N][j]) * current[(i + 1) % N][j];
                next[i][j] += strong_inf * (1 - cond[(i - 1 + N) % N][j]) * current[(i - 1 + N) % N][j];
                next[i][j] += strong_inf * (1 - cond[i][j + 1]) * current[i][j + 1];
                next[i][j] += strong_inf * (1 - cond[i][j - 1]) * current[i][j - 1];
                next[i][j] += weak_inf *     (1 - cond[(i - 1 + N) % N][j - 1]) * current[(i - 1 + N) % N][j - 1];
                next[i][j] += weak_inf * (1 - cond[(i + 1) % N][j - 1])* current[(i + 1) % N][j - 1];
                next[i][j] += weak_inf * (1 - cond[(i - 1 + N) % N][j + 1]) * current[(i - 1 + N) % N][j + 1];
                next[i][j] += weak_inf * (1 - cond[(i + 1) % N][j + 1]) * current[(i + 1) % N][j + 1];
                */
               

    /* Copy check

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

/* linear transform

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

   /* print array


       /*
    for (int i = 0; i < N + 2; i++) { 
        for (int j = 0; j < M; j++) {
                printf("Value for [%d][%d] is %.5f: \n",i,j, next[i][j]); 
        } 
    } 
    */ 