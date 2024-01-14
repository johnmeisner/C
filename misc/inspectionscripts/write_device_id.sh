#!/bin/sh
# Date       / Author / Version  
# 3.11.2021  / SBaucom   / v1.1   
# 3.17.2022  / SBaucom   / v2.1
# 11.21.2022 / SBaucom   / v2.2


###########################################
# Verify ID
###########################################
ID="$1"
if [[ -z "$ID" ]]; then
    exit 
fi 


###########################################
# Create CSV with the Hex representation of the device ID
###########################################
TEMP_FILE=`mktemp /tmp/idXXXXX.csv`
> "$TEMP_FILE"

# Get the string to program the protected NOR sector
echo "Device ID = $ID"

# Write to CSV (and print to console)
for ((i=0;i<${#ID};i++));do 
	# printf "%02X " \'${ID:$i:1};
	printf "%02X; " \'${ID:$i:1} >> "$TEMP_FILE";
done
echo ""

###########################################
# Unlock the sector and write the id to it
###########################################
denso_nor_unlock
denso_nor_write "$TEMP_FILE"
rm "$TEMP_FILE"

echo ""

