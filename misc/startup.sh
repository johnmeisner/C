#!/bin/sh
#-------------------------------------------------------------------------------
# File: startup.sh
#
# Copyright (C) 2021 DENSO International America, Inc.
#
# Description: Hercules platform startup script
#------------------------------------------------------------------------------
echo "DENSO HD RSU startup.sh running"

# Variables controlling Ethernet port's IP address. Changed if desired.
# these variables are used by rsuhealth; they are no longer directly applied
# in the startup
DHCP_ENABLE=FALSE
ETH_DEVICE=eth0     # Device to configure
IP_FOR_STATIC_ADDRESS=192.168.2.80
NETMASK_FOR_STATIC_ADDRESS=24
WEBGUI_HTTP_SERVICE_PORT=0
WEBGUI_HTTPS_SERVICE_PORT=4934
VLAN_ENABLE=FALSE
VLAN_IP=192.168.3.80
VLAN_MASK=24
VLAN_ID=0
MTU=1500

# Mount the customer's resolv.conf if it exists
if [ -f /mnt/rwflash/customer/resolv.conf ]; then
   mount -o bind /mnt/rwflash/customer/resolv.conf /etc/resolv.conf
fi

# If necessary, create I2V directories
mkdir -p /mnt/rwflash/I2V
mkdir -p /mnt/rwflash/I2V/amh
mkdir -p /mnt/rwflash/I2V/applogs
mkdir -p /mnt/rwflash/I2V/data
mkdir -p /mnt/rwflash/I2V/logretrieval
mkdir -p /mnt/rwflash/I2V/logs
mkdir -p /mnt/rwflash/I2V/syslog
mkdir -p /mnt/rwflash/logs
mkdir -p /mnt/rwflash/security

# wait for rsuhealth to be ready
while [ ! -e /tmp/rsuhealthready ]; do
    sleep 1
done

# tell rsuhealth not to check cv2x antenna status
# 5940 has no inpedance and will always fail check
# default for rsyhealth is to check
rsuhealth -acoff

# Copy scms.sh if needed
if [ -f /mnt/rwflash/customer/scms.sh ]; then
   echo "scms.sh exists"
else
   if [ ! -d /mnt/rwflash/customer ]; then
        echo "customer directory is not found in /mnt/rwflash..creating.."
        mkdir -p /mnt/rwflash/customer
   fi
   echo "Copying scms.sh file..."
   cp /usr/bin/scms.sh      /mnt/rwflash/customer/.
fi

# prepare customer permissions
chown -R rsuuser:rsuuser /rwflash/customer
# end prepare customer permissions

# Source Aerolink related PATHs and enviornment variables
source /mnt/rwflash/customer/scms.sh


# Copy SCMS_Download_Files if needed
if [ -d /mnt/rwflash/SCMS_Download_Files ]; then
   echo SCMS_Download_Files exists
else
   echo "Copying SCMS_Download_Files file..."
   cp -r /usr/share/SCMS_Download_Files /mnt/rwflash/
fi

# Move Aerolink's old log(s) to a different name
if [ -f /mnt/rwflash/aerolink-1.log ]; then
    rm /mnt/rwflash/aerolink-4.log
    mv /mnt/rwflash/aerolink-3.log /mnt/rwflash/aerolink-4.log
    mv /mnt/rwflash/aerolink-2.log /mnt/rwflash/aerolink-3.log
    mv /mnt/rwflash/aerolink-1.log /mnt/rwflash/aerolink-2.log
fi

# Start logging first for all tasks that follow.
logmgr_app -d/rwflash/configs/ -I1 x x &

# cv2x_daemon should be set to start automatically by entering the
# "systemctl enable cv2x.service" command.
sleep 5
echo "starting tps"
# Start Time and Positioning Services
tps &

# Display the current root certificate chain table
#certadm list

currdir=$PWD
# Start Radio Services (the Radio Stack)
# Note: Must start from the /usr/bin directory for Aerolink to start up
# properly.
sleep 5
#echo "starting Radio"
cd /usr/bin
radioServices &
waitForRadioServicesBooted
smiUpdatePositionAndTime &

cd $currdir
# Start vod 
vod &
# Start I2V
sleep 5
#echo "starting I2V"
i2v_app &
#alert rsuhealth i2v is running
rsuhealth -Ir

# SNMPD NTCIP-1218 MIB Startup
MIBS=ALL snmpd  -f -Lf /rwflash/mysnmpd.txt -c /rwflash/configs/snmpd.conf > /dev/null
# debug output. Will grow unchecked.
#MIBS=ALL snmpd -f -Lf /rwflash/mysnmpd.txt -Dntcip_1218,rsu_table -C -c /mnt/rwflash/configs/snmpd.conf
#MIBS=ALL snmptrapd  -Lf /rwflash/mysnmptrapd.txt -c /rwflash/configs/snmptrapd.conf > /dev/null

# Startup SOUS - Shared Opaque Utility Server
sous &

# Startup config manager
conf_manager > /dev/null &
# Startup webgui.  If we are running DHCP, figure out our IP address to give to bozo
if [ $DHCP_ENABLE == TRUE ]; then
    IP_FOR_STATIC_ADDRESS=`ifconfig $ETH_DEVICE | grep inet.addr | sed -e 's/ *Bcast.*//' -e 's/.*inet.addr://'`
fi
# Start up the bozo services

if [ $WEBGUI_HTTP_SERVICE_PORT -gt 0 ]; then
#    bozohttpd -n -f -c /usr/local/www/cgi-bin -I $WEBGUI_HTTP_SERVICE_PORT -i $IP_FOR_STATIC_ADDRESS /usr/local/www/html webgui &
    bozohttpd -n -f -c -d /usr/local/www/cgi-bin -I $WEBGUI_HTTP_SERVICE_PORT -i $IP_FOR_STATIC_ADDRESS /usr/local/www/html webgui &
echo $WEBGUI_HTTP_SERVICE_PORT

fi

if [ $WEBGUI_HTTPS_SERVICE_PORT -gt 0 ]; then
    bozohttpd -n -f -c /usr/local/www/cgi-bin -I $WEBGUI_HTTPS_SERVICE_PORT -i $IP_FOR_STATIC_ADDRESS -Z /usr/local/www/cert/certificate.pem /usr/local/www/cert/key.pem /usr/local/www/html WebGui &
#    bozohttpd -n -f -c -d /usr/local/www/cgi-bin -I $WEBGUI_HTTPS_SERVICE_PORT -i $IP_FOR_STATIC_ADDRESS -Z /usr/local/www/cert/certificate.pem /usr/local/www/cert/key.pem /usr/local/www/html WebGui &
echo $WEBGUI_HTTPS_SERVICE_PORT
fi

# Webgui & configs safety check
sleep 1
if [ "`ps aux | grep conf_manager | grep -v grep`" != "" ]; then
    echo Config Management started.
else
    echo WARNING: Unable to start conf_manager server
fi 
if [ "`ps aux | grep bozohttpd | grep -v grep`" != "" ]; then
    echo Webgui started.
else
    if [ $WEBGUI_HTTP_SERVICE_PORT -gt 0 ]; then
        echo WARNING: Unable to start webgui server
    elif [ $WEBGUI_HTTPS_SERVICE_PORT -gt 0 ]; then
        echo WARNING: Unable to start webgui server
    else
        echo FYI: webgui service not started, disabled by user
    fi
fi 

# turn green led solid
rsuhealth -Sgo

# Enable push button manger
pshbtnmonitor &

# Enable RSU access manager
rsuaccessmgr &

