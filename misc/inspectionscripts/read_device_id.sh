#!/bin/sh
# Date       / Author / Version  
# 3.11.2021 / SBaucom   / v1.1   

#########################################
# Read the NOR protected sector
########################################
str=`denso_nor_read`

#########################################
# Split the string in to an array 
# where each element is line of bytes
########################################
arr=()
while IFS= read -r line || [[ "$line" ]]; do 
    IFS=":" read -ra line_array <<< "$line"
    arr+=("${line_array[1]}")
done <<< "$str"

##########################################
# Split the array so each element is one byte
##########################################
hex_id=()
for elem in "${arr[@]}"; do
    while IFS= read -r -d " " byte || [[ "$byte" ]]; do 
	if [[ "$byte" != "00" ]]; then
	    hex_id+=("$byte")
	fi
    done <<< "$elem"
done

##########################################
# Convert the hex_id to ascii and print
##########################################

for i in ${hex_id[@]}; do
    if [[ "$i" != "ff" ]]; then
	printf "\x$i"
    fi
done

echo ""
echo ""
