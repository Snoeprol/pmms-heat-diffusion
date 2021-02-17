#!/bin/bash
./gen_im.o 1000 1000
./gen_im.o 2500 2500
./gen_im.o 100 20000
./gen_im.o 20000 100
./gen_im.o 5000 5000
rm pretty_results.txt
rm test_results.txt
cp comput.c.bak compute.c
declare -a StringArray=("schedule(static)" "schedule(dynamic)" "schedule(guided,1)")
STR="schedule(static)"
for i in ${!StringArray[@]}; do
(

echo "${StringArray[$i]} ${StringArray[$i+1]}}"
make clean && make
head -85 compute.c | grep schedule >> test_results.txt
./20000x100test.sh >> test_results.txt
./100x20000test.sh >> test_results.txt 
./2500test.sh >> test_results.txt
./5000test.sh >> test_results.txt
sed -i -e "s/${StringArray[$i]}/${StringArray[$i+1]}/g" compute.c
) done
sed -i -e 's/schedule(guided, 1)/schedule(static)/g' compute.c
python scramble_results.py
cat pretty_results.txt

rm 5000x5000.pgm
rm 100x20000.pgm
rm 20000x100.pgm
rm 2500x2500.pgm
rm 1000x1000.pgm
