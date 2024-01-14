#!/bin/sh
# This script triggers the reboot signal to the RH850
# to test the reboot override timeout function. If
# M_BUSY toggles to 1 and does not toggle to 0 within
# the default 45s timeout, RH850 will cold reset the 
# system.
#
# Date       / Author      / Version / Description
# 2020.12.08 / JStinnett   / v1.0    / Initial release
# 2022.02.16 / JStinnett   / v1.1    / Updated M_BUSY mapping for RSU-5940


trap deactivate SIGINT

M_BUSY=473
TIMEOUT=45

function deactivate
{
    echo "RH850_reboot_timeout.sh: Deactivating reboot signal (M_BUSY=0)"
    if [ ! -d /sys/class/gpio/gpio$M_BUSY ] ; then
        echo $M_BUSY > /sys/class/gpio/export
    fi
    echo out > /sys/class/gpio/gpio$M_BUSY/direction
    echo 0 > /sys/class/gpio/gpio$M_BUSY/value
    exit 0
}


echo ""
echo "RH850_reboot_timeout.sh: Activating reboot signal (M_BUSY=1)"
echo "RH850_reboot_timeout.sh:"
if [ ! -d /sys/class/gpio/gpio$M_BUSY ] ; then
    echo $DSRC_RF_SEL > /sys/class/gpio/export
fi
echo out > /sys/class/gpio/gpio$M_BUSY/direction
echo 1 > /sys/class/gpio/gpio$M_BUSY/value
COUNT=$TIMEOUT
while [ $COUNT -gt 0 ]; do
    echo "WARNING: System reset in $COUNT seconds!"
    let COUNT=COUNT-1
    sleep 1
done
