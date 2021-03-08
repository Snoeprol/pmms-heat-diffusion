#!/bin/bash

for i in 100,20000 20000,100 2500,2500 5000,5000
do(
    IFS=',' read item1 item2 <<< "${i}"
    B=$( printf '%ix%i.pgm' ${item1} ${item2} )
    for (( c=1; c<=32; c=2*c )); 
    do(
        for((j=0;j<10;j++))
        do( 
            echo params $i, $c
            ./heat_pth -p $c -i 256 -c $B -t $B -n $item1 -m $item2 | grep "256     "
            perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./heat_pth -p $c -i 256 -c $B -t $B -n $item1 -m $item2 | grep 'instructions.*M/sec$'
        ) done
    ) done
) done
