#!/bin/sh
# Date       / Author / Version  
# 02.19.2022 / JStinnett   / v1.0 


function usage
{
    echo ""
	echo "set_status_led.sh <option>"
    echo "options: grn; yel; red; off;"
	echo ""
}

if [[ "$1" == "grn" ]]; then
	echo "set_status_led.sh: Turn ON Green LED"
	/usr/src/wnc/scripts/gpiodriver_ctl.sh 0x0100 > /dev/null 2>/dev/null
elif [[ "$1" == "yel" ]]; then
        echo "set_status_led.sh: Turn ON Yellow LED"
        /usr/src/wnc/scripts/gpiodriver_ctl.sh 0x0400 > /dev/null 2>/dev/null
elif [[ "$1" == "red" ]]; then
	echo "set_status_led.sh: Turn ON Yellow LED"
        /usr/src/wnc/scripts/gpiodriver_ctl.sh 0x0080 > /dev/null 2>/dev/null
elif [[ "$1" == "off" ]]; then
	echo "set_status_led.sh: Turn ALL Status LEDs OFF"
        /usr/src/wnc/scripts/gpiodriver_ctl.sh 0x0000 > /dev/null 2>/dev/null
else
	usage
fi

