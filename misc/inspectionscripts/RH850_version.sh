#!/bin/sh
# Date       / Author / Version  
# 10.2.2020 / SBaucom   / v1.0   
# 12.22.202 / JStinnett / v1.1 Add wait after PID kill
# Gets the RH850 Version 


# Set up rh850 device file
stty -F /dev/ttyLP3 115200 sane

# Set up output file 
FILENAME="/tmp/RH850_output_version.txt"
touch $FILENAME
chmod 777 $FILENAME


# Start reading ttyLP3
cat /dev/ttyLP3 > $FILENAME & PID=$!

# Send the first command 
echo -e "ZV\r" > /dev/ttyLP3
sleep 0.5
kill $PID
wait $PID 2>/dev/null

# Print the version 
result=`cat $FILENAME | grep Version` 

echo ""
echo "Result: $result"

rm $FILENAME

