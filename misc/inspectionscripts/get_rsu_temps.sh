#!/bin/sh
# Date       / Author / Version  
# 05.05.2021 / SBaucom   / v1.1 


temps=("cpu" "pmic_int" "hsm" "secton" "pluton2" "temp_ic" "ambient" "poe" "dcdc" "pmic_ext")
TEMP_FILE=`mktemp /tmp/idXXXXX.txt`

function usage
{
    temps=("all" "${temps[@]}")    
    echo "device_test.sh <option>"
    IFS=", "
    echo "options: ${temps[*]}"
}

function round ()
{
    num=$1

    if (( $(echo "$num < 0.0 " | bc) )); then 
        echo "($1-0.5)/1" | bc
    else
        echo "($1+0.5)/1" | bc
    fi 
}

function sign () 
{
    num=$1    

    if [[ "$num" == "-0" ]]; then 
        num=0 # edge case; signing -0 ==> +-0
    fi 

    if [[ $((num)) -lt 0 ]]; then 
        echo "$num"
    else
        echo "+$num"
    fi
}

function get_cpu ()
{
    cmd="cat /sys/class/thermal/thermal_zone0/temp"
    temp=`$cmd`
    temp=`echo "scale=2 ; $temp / 1000" | bc`
    temp=`sign $(round $temp)`"C"
    
    echo "cpu: $temp"
}

function get_pmic_int ()
{
    cmd="cat /sys/class/thermal/thermal_zone1/temp"
    temp=`$cmd`   
    temp=`echo "scale=2 ; $temp / 1000" | bc`
    temp=`sign $(round $temp)`"C"
 
    echo "pmic_int: $temp"
}

function get_hsm ()
{
    app="v2xse-example-app -t /dev/spidev0.0 -d 406 -r 488"
    cd /usr/bin
    echo "1
          3
          39
          0" | $app >  $TEMP_FILE
    temp=`cat $TEMP_FILE| grep "Chip Temperature is:"`
    temp=$(echo $temp | cut -d' ' -f5)
    temp=`sign $(round $temp)`"C"

    rm $TEMP_FILE
    echo "hsm: $temp"
}

function get_secton ()
{
    # Alternate method of getting the temperature
    #cd /usr/bin
    #cmd="atlk_temper /dev/spidev1.0"
    #temp=`$cmd 2>/dev/null`    
    #temp=`echo $temp | grep "Get Autotalks Device temperature Successfully"` 
    #temp=$(echo $temp | rev | cut -d' ' -f1 | rev )
    #temp=`sign $(round $temp)`

    app="diag-cli /dev/spidev1.0"
    cd /usr/bin
    echo "ddm status_get
          exit" | $app &> $TEMP_FILE
    temp=`cat $TEMP_FILE| grep "Device temperature"`
    temp=$(echo $temp | rev | cut -d' ' -f1 | rev )
    temp=`sign $(round $temp)`"C"
    
    rm $TEMP_FILE
    echo "secton: $temp"
}

function get_pluton2 ()
{
    app="diag-cli /dev/spidev1.0"
    cd /usr/bin
    echo "wlan rf_status_get
          exit" | $app &> $TEMP_FILE
    temp=`cat $TEMP_FILE| grep "RF temperature"`
    temp=$(echo $temp | rev | cut -d' ' -f1 | rev )
    if [[ $temp =~ [0-9] ]]; then 
        temp=`sign $(round $temp)`"C"
    else
        temp="Not Available"
    fi

    
    rm $TEMP_FILE
    echo "pluton2: $temp"
}

function get_temp_ic ()
{
    cmd="cat /sys/class/hwmon/hwmon0/temp1_input"
    temp=`$cmd` 
    temp=`echo "scale=3; $temp/1000-64" | bc`
    temp=`sign $(round $temp)`"C"

    echo "temp_ic: $temp"
}

function get_ambient ()
{
    cmd="cat /sys/class/hwmon/hwmon0/temp2_input"
    temp=`$cmd`   
    temp=`echo "scale=3; $temp/1000-64" | bc`
    temp=`sign $(round $temp)`"C"
   
    echo "ambient: $temp"
}

function get_poe ()
{
    HW_VER=`/usr/src/wnc/scripts/get_hw_ver.sh | grep 'EVT' | cut -d" " -f 3`
    if [[ "$HW_VER" == "EVT1" ]];then
      cmd="cat /sys/class/hwmon/hwmon0/temp3_input"
      temp=`$cmd`   
      temp=`echo "scale=3; $temp/1000-64" | bc`
      temp=`sign $(round $temp)`"C"
	  echo "poe: $temp"
    else
      echo "poe: N/A (EVT2 HW Detected)"
    fi 
}

function get_dcdc ()
{
    cmd="cat /sys/class/hwmon/hwmon0/temp4_input"
    temp=`$cmd` 
    temp=`echo "scale=3; $temp/1000-64" | bc`
    temp=`sign $(round $temp)`"C"
    
    echo "dcdc: $temp"
}

function get_pmic_ext ()
{
    cmd="cat /sys/class/hwmon/hwmon0/temp5_input"
    temp=`$cmd` 
    temp=`echo "scale=3; $temp/1000-64" | bc`
    temp=`sign $(round $temp)`"C"
    
    echo "pmic_ext: $temp"
}



#########################
# No argument provided 
#########################
if [[ "$1" == "" ]]; then
    usage
    echo

#########################
# All Temps
#########################
elif [[ "$1" == "all" ]]; then 
    for temp in "${temps[@]}"; do 
        result=`get_$temp`
        echo "$result"
    done
    echo ""

#########################
# Single Temp
#########################
elif [[ " ${temps[*]} " =~ (^|[[:space:]])"$1"(^|[[:space:]]) ]]; then 
    result=`get_$1`
    echo "$result"
    echo ""

#########################
# Invalid Temp
#########################
else
    echo "Invalid option"
    usage
fi
    