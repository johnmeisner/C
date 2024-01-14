#!/bin/sh
# Date       / Author / Version  
# 11.5.2020 / SBaucom   / v1.0   

function usage
{
    echo "status_led.sh <option>"
    echo "options: blink; on; off"
}

if [[ "$1" == "" ]]; then
    usage

else
    stty -F /dev/ttyLP3 115200 sane

    if [[ "$1" == "blink" ]]; then 
	echo "Status LED now blinking... "
        
        for ((num=0; num < 10; num++))
        do
            echo -e "Z5\r" > /dev/ttyLP3
            sleep 0.5
            echo -e "Z4\r" > /dev/ttyLP3
            sleep 0.5
        done
        echo "Status LED finished blinking..."
        

    elif [[ "$1" == "on" ]]; then 
	echo "Status LED on. "
	echo -e "Z5\r" > /dev/ttyLP3

    elif [[ "$1" == "off" ]]; then 
	echo "Status LED off. "
	echo -e "Z4\r" > /dev/ttyLP3
	
    else 
	echo "Error: $1 is not a valid option. "
	usage
    fi
fi
