#!/bin/bash

c="2"
n="1000"
m="1000"
B=$( printf '%ix%i.pgm' ${n} ${m} )

for (( c=1; c<=32; c=2*c )); 
do(
    printf "c = $c\n"   
    for i in 10 100 1000 2000 2500 5000

    do( 
        printf "i = $i\n"   
        for ((l=0;l<1;l++))
        do(
        printf "0, "
        ./histo/histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -p $c -n $i -m $i -r | grep "seconds" | awk '{print $3}'
        printf "1, "
        ./histo/with_mutual_ex/histo_atomic/histo_atomic -p $c -n $i -m $i -r | grep "seconds" | awk '{print $3}'
        printf "2, "
        ./histo/with_mutual_ex/histo_mutex/histo_mutex -p $c -n $n -m $i -r | grep "seconds" | awk '{print $3}'
        printf "3, "
        ./histo/with_mutual_ex/histo_semaphores/histo_semaphores -p $c -n $i -m $i -r | grep "seconds" | awk '{print $3}'
        printf "4, "
        ./histo/with_mutual_ex/histo_sw_transactional/histo_sw_transactional -p $c -n $i -m $i -r | grep "seconds" | awk '{print $3}'
        )done
    )done
)done