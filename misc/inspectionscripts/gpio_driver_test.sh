#!/bin/sh
# Date       / Author / Version  
# 10.2.2020 / SBaucom   / v1.0   
# 2.16.2022 / JStinnett	/ v1.1 Updated script for RSU-5940.  Note: GPO4 cannot be tested with this script.


RESULT=PASS


# Set all four GPO/GPIO pins low 
echo "Setting pins low..."
gpiodriver_ctl.sh 0x0380

# Check that all three GPIO pins went low
pin1=$(echo `gpio_ctl.sh GPIO1_IMX8 r`| rev | cut -d' ' -f1 | rev)
pin2=$(echo `gpio_ctl.sh GPIO2_IMX8 r`| rev | cut -d' ' -f1 | rev)
pin3=$(echo `gpio_ctl.sh GPIO3_IMX8 r`| rev | cut -d' ' -f1 | rev)

echo "GPIO1: $pin1"
echo "GPIO2: $pin2"
echo "GPIO3: $pin3"

if [[ $pin1 == 0 && $pin2 == 0 && $pin3 == 0 ]]; then
   echo "Pins set low"
else
    echo "Error setting pins low"
    RESULT=FAIL
fi

# Set all four GPO/GPIO pins high 
echo "Setting pins high..."
gpiodriver_ctl.sh 0x079e

# Check that all three GPIO pins went high
pin1=$(echo `gpio_ctl.sh GPIO1_IMX8 r`| rev | cut -d' ' -f1 | rev)
pin2=$(echo `gpio_ctl.sh GPIO2_IMX8 r`| rev | cut -d' ' -f1 | rev)
pin3=$(echo `gpio_ctl.sh GPIO3_IMX8 r`| rev | cut -d' ' -f1 | rev)

echo "GPIO1: $pin1"
echo "GPIO2: $pin2"
echo "GPIO3: $pin3"

if [[ $pin1 == 1 && $pin2 == 1 && $pin3 == 1 ]]; then
   echo "Pins set high"
else
    echo "Error setting pins high"
    RESULT=FAIL
fi

echo "Result: $RESULT"
