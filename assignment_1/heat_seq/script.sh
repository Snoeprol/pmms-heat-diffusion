#!/bin/sh
# Square grids
for i in 100 1000 2000 ; do(
	B=$( printf '%ix%i.pmg' $i $i )
	../../images/gen_im.o $i $i
	sleep 1s
	./heat_seq -n $i -m $i -c $B -t $B
)
done
wait

# Rectangles M > N
for i in 100,50 100,200 100,1000 100,2000; ; do(
    IFS=',' read item1 item2 <<< "${i}"
	B=$( printf '%ix%i.pmg' ${item1} ${item2} )
	../../images/gen_im.o ${item1} ${item2}
	sleep 1s
	./heat_seq -n ${item1} -m ${item2} -c $B -t $B
)
done
wait

# Rectangles N > M
for i in  50,100 200,100 1000,100 2000,100; do(
    IFS=',' read item1 item2 <<< "${i}"
	../../images/gen_im.o ${item1} ${item2}
	B=$( printf '%ix%i.pmg' ${item1} ${item2} )
	sleep 1s
	./heat_seq -n ${item1} -m ${item2} -c $B -t $B
)
done
wait
