#!/bin/bash
for i in 5000,5000
do(
IFS=',' read item1 item2 <<< "${i}"
B=$( printf '%ix%i.pgm' ${item2} ${item1} )
prun -v -1 -np 1 -1 heat_omp -n ${item1} -m ${item2} -c $B -t $B -i 100 -p 8
)
done
