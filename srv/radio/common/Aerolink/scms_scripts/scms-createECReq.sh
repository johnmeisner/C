#!/bin/sh
##################################scms-createECReq.sh################################
# This script takes care of creating Request for Enrollment Certificate
#####################################################################################

function usage {
    echo "Usage: ./scms-createECReq -d 15y -s 2021-02-01 -p 0x82+0x83+0x204097"
    echo ""
    echo "All options and values are MANDATORY"
    echo ""
    echo "duration is option -d.    Example: -d 15y"
    echo "startDate is option -s.   Example: -s 2021-02-01 (Date alone will start at 2020-03-08+00:00:00 UTC)"
    echo "                          Example: -s 2021-02-01+13:00:00 (Date and Time UTC)"
    echo "certReqPermissions is option -p(PSIDs+SSPs for Apps)."
    echo "                          Example: -p 0x82:0x00-80-01-30-40+0x83:0x00-80-01-F0-40+0x204097:0x00-80-01-20-40"  # Add bitmasks here
    echo ""
    echo "Usage: ./scms-createECReq -h"
    echo "Help option is -h."
    exit 1
}

export AEROLINK_STATE_DIR=/rwflash/aerolink
export AEROLINK_CONFIG_DIR=/rwflash/configs/aerolink_conf

CERTDIR=/mnt/rwflash/aerolink/certificates
CONTEXTDIR=/mnt/rwflash/aerolink/security-context
OERBACKUP=/mnt/rwflash/OERBackup

#Check the number of arguments
if [ $# -lt 1 ]; then
    usage
fi

#Get the values of each option 
while getopts ":d:h:p:s:" arg; do
    case $arg in
        d)  duration=$OPTARG
            flagd=true
            ;;
        h)  usage 
            ;;
        p)  certReqPermissions=$OPTARG 
            flagp=true 
            ;;
        s) 
            startDate=$OPTARG 
            flags=true 
            ;;
        ?)  usage 
            ;;
    esac
done

#Check if all four option values are present
if [ "$flagd" != "true" ] || [ "$flagp" != "true" ] || [ "$flags" != "true" ]; then
    usage 
fi

shift "$(($OPTIND - 1))"   

#echo ""
# Make sure scms-install.sh was run prior to running this script.
#if [  -d /mnt/rwflash/aerolink/certificates ] && [  -d /mnt/rwflash/aerolink/security-context ] && [ -e /mnt/rwflash/aerolink/security-context/denso.wsc ]; then
if [  -d $CERTDIR ] && [  -d $CONTEXTDIR ] && [ -e $CONTEXTDIR/denso.wsc ]; then
    echo "Ready to run Request for EC"
    echo ""
    else
    echo "Please run scms-install.sh before running scms-createECReq"
    exit 1
fi

#Get the Unique Id
certId=`get_unique_id.sh | grep UNIQUE_ID_HEX | awk '{print $3}'`

# Command should be run from /usr/bin
cd /usr/bin/

create_cmd="createECRequest --contextName denso.wsc --lcmName rsuLcm --certId n:$certId --duration $duration --start $startDate --certReqPermissions $certReqPermissions --region  countryOnly:840"

echo $create_cmd

create_result=`eval "$create_cmd" 2>&1`

echo $create_result

echo ""
echo ""
echo "createECRequest Results:"
#Check if the command was successful. Useful if this script is run from GUI
if [[ $create_result == *"successfully."* ]] && [[ $create_result == *"/rwflash/aerolink/certificates/"* ]]; then
    echo "Enrollment certificate request completed successfully"
else
    echo "Enrollment certificate request NOT completed successfully"
    exit 1
fi

if [ -e $CERTDIR/*.oer ] && [ -e $CERTDIR/*.prikey ]; then
    echo "Enrollment keys are found in $CERTDIR directory"
    echo "Backing up *.oer and .prikey for retrival during process EC Response"
    if [ ! -d $OERBACKUP ]; then
        # Error check is skipped here since /mnt/rwflash should be accessible always
        mkdir -p $OERBACKUP
    fi
    # Delete any previous key backup
    rm -f $OERBACKUP/*
    cp $CERTDIR/*.{oer,prikey} $OERBACKUP
    echo "The .oer and .prikey are backed up on $OERBACKUP"
else 
    echo "Enrollment keys are NOT found in $CERTDIR directory"
fi
