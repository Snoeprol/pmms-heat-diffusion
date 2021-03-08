#!/bin/bash

c="2"
n="1000"
m="1000"
B=$( printf '%ix%i.pgm' ${n} ${m} )
i="1000"
c="16"

B=$( printf '1000x1000.pgm')
B2=$( printf '1000x1000_2.pgm')

printf "c = $c\n"   
for value in $B $B2
do( 
    printf "image: ${value}\n"   
    for ((l=1;l<5;l++))
    do(
    printf "0, "
    prun -v -1 -np 1 -1 ./histo/histo_avoiding_mutual_ex/histo_avoiding_mutual_ex -p $c -n $i -m $i -i $B | grep "seconds" | awk '{print $3}'
    printf "1, "
    prun -v -1 -np 1 -1 ./histo/with_mutual_ex/histo_atomic/histo_atomic  -p $c -n $i -m $i -i $B  | grep "seconds" | awk '{print $3}'
    printf "2, "
    prun -v -1 -np 1 -1 ./histo/with_mutual_ex/histo_mutex/histo_mutex  -p $c -n $i -m $i -i $B | grep "seconds" | awk '{print $3}'
    printf "3, "
    prun -v -1 -np 1 -1 ./histo/with_mutual_ex/histo_semaphores/histo_semaphores  -p $c -n $i -m $i -i $B  | grep "seconds" | awk '{print $3}'
    printf "4, "
    prun -v -1 -np 1 -1 ./histo/with_mutual_ex/histo_sw_transactional/histo_sw_transactional  -p $c -n $i -m $i -i $B  | grep "seconds" | awk '{print $3}'
    )done
)done