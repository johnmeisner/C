#!/bin/sh
##############################scms-process-install.sh################################
# This script takes care of installing the SCMS on RSU                              #
#####################################################################################

CONTEXTDIR=/mnt/rwflash/aerolink/security-context
CERTDIR=/mnt/rwflash/aerolink/certificates
SCMSDOWNLOADDIR=/mnt/rwflash/SCMS_Download_Files
OERBACKUP=/mnt/rwflash/OERBackup

#Check if SCMS_Download_Files is present in /mnt/rwflash
if [ ! -d $SCMSDOWNLOADDIR ] && [ ! -e $SCMSDOWNLOADDIR/installAerolinkConfigs.sh ]; then
    echo "Copy SCMS_Download_Files from Release directory to /mnt/rwflash."
    exit
fi

#SCMS_Download_Files present. 
echo "Running the Aerolink installation"
chmod 776 $SCMSDOWNLOADDIR/installAerolinkConfigs.sh
err_info=`$SCMSDOWNLOADDIR/installAerolinkConfigs.sh 2>&1`
echo $err_info

#Check if /mnt/rwflash/aerolink directory is present
if [ ! -d $CERTDIR ] && [ ! -d $CONTEXTDIR ]; then
    echo "Aerolink files not installed correctly"
else
    echo "Aerolink files installed correctly"
fi

# Retrieve backed up .oer and .prikey files prior to run the script.
# These files were backed up when scms-createECReq.sh
if [ -e $OERBACKUP/*.oer ] && [ -e $OERBACKUP/*.prikey ]; then
    echo "Copying .oer and .prikey files back to /mnt/rwflash/aerolink/certificates directory"
    cp $OERBACKUP/*.{oer,prikey} $CERTDIR/.
else
    echo "Unable to find .oer and .prikey files in $OERBACKUP for this installation"
fi
