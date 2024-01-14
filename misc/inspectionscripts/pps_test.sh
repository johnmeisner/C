#!/bin/sh
# Date       / Author / Version  
# 10.2.2020 / SBaucom   / v1.0   

RESULTS=PASS


# Set up output file:
FILENAME="/tmp/pps_test_output.txt"
touch $FILENAME
chmod 777 $FILENAME

# Get pps info: 
ppstest /dev/pps0 > $FILENAME & PID=$!
sleep 2
kill $PID
wait $PID 2>/dev/null


# Read seconds
lines=()
while IFS= read -r line || [[ "$line" ]]; do 
    if [[ "$line" == *"assert"* ]]; then 
	line=$(echo $line | cut -d' ' -f5 | cut -d',' -f1)
	lines+=("$line")
    fi
done < $FILENAME

# Print seconds
for i in ${!lines[@]}; do 
	echo ${lines[$i]}	
done


# Check that times are increasing by one second at a time
dt=$(bc -l <<< "${lines[1]}-${lines[0]}")
var1=`echo "$dt < 1.001" | bc` 
var2=`echo "$dt > 0.999" | bc`
if [[ ($var1 -eq 0 || $var2 -eq 0) ]]; then 
	echo "Error: Not increasing per second. "
	RESULTS=FAIL
fi


# Check that fractions of a second are close to zero 
frac="0."$(echo ${lines[0]} | cut -d '.' -f2)
var1=`echo "$frac < 0.1" | bc` 
var2=`echo "$frac > 0.9" | bc`
if [[ $var1 -eq 0 && $var2 -eq 0 ]]; then 
	echo "Error: fractions not close to zero"
	RESULTS=FAIL
fi


rm $FILENAME

# Print results
echo "Result: $RESULTS"


