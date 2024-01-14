#!/bin/sh
##########################scms-install.sh############################################
# This script takes care of installing the SCMS on RSU and                          #
# generates HSM keys.                                                               #
#####################################################################################

AEROLINKDIR=/mnt/rwflash/aerolink
SCMSDOWNLOADDIR=/mnt/rwflash/SCMS_Download_Files

#Check if SCMS_Download_Files is present in /mnt/rwflash
if [ ! -d $SCMSDOWNLOADDIR ] && [ ! -e $SCMSDOWNLOADDIR/installAerolinkConfigs.sh ]; then
    echo "Copy SCMS_Download_Files from Release directory to /mnt/rwflash."
    exit
fi

#SCMS_Download_Files_HSM present. 
echo "Running the Aerolink installation"
chmod 776 $SCMSDOWNLOADDIR/installAerolinkConfigs.sh
err_info=`$SCMSDOWNLOADDIR/installAerolinkConfigs.sh 2>&1`
echo $err_info

#Check if /mnt/rwflash/aerolink directory is present
if [ ! -d $AEROLINKDIR/certificates ] && [ ! -d $AEROLINKDIR/security-context ]; then
    echo "Aerolink files not installed correctly"
else
    echo "Aerolink files installed correctly"
fi

sleep 1
