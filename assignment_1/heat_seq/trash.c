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