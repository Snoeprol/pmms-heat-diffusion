#!/bin/bash
rm pretty_results.txt
rm test_results.txt
cp comput.c.bak compute.c
declare -a StringArray=("schedule(static)" "schedule(dynamic)" "schedule(guided,1)")
STR="schedule(static)"
for val in ${StringArray[@]}; do
(
echo "$STR start forloop" 
echo "s/$STR/$val/g" 
sed -i -e "s/$STR/$val/g" compute.c
make clean && make
head -85 compute.c | grep schedule >> test_results.txt
./20000x100test.sh >> test_results.txt
./100x20000test.sh >> test_results.txt 
./2500test.sh >> test_results.txt
./5000test.sh >> test_results.txt
STR=$val
echo "$STR end of for loop"
) done
sed -i -e 's/schedule(guided, 1)/schedule(static)/g' compute.c
python scramble_results.py
cat pretty_results.txt
