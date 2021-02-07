#!/bin/bash
for i in 2500,2500
do(
IFS=',' read item1 item2 <<< "${i}"
B=$( printf '%ix%i.pgm' ${item2} ${item1} )
sleep 5s
prun -v -1 -np 1 -1 heat_seq -n ${item1} -m ${item2} -c $B -t $B -i 100
)
done