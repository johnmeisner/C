#!/bin/sh
##################################scms-processECRes.sh###############################
# This script takes care of processing Enrollment Request Response from SCMS server
#####################################################################################

#This script doesn't take any input. It checks for the files that should be present 
#in /rwflash/root/aerolink/certificates directory before running the EC Response
# No arrays supported in /bin/sh. So check some files and make sure they are present before executing the script

RWFLASH=/mnt/rwflash
CERTDIR=/mnt/rwflash/aerolink/certificates

#Check if ISS_CMS_RESPONSE_*.zip is present in /mnt/rwflash
filep=`ls $RWFLASH/ISS_CMS_RESPONSE_*.zip 2>/dev/null | wc -w` 
echo $filep
if [ 0 -eq $filep ]; then
        echo "There in no ISS_CMS_RESPONSE_*.zip file in /mnt/rwflash"
        exit 1
elif [ 1 -lt $filep ]; then
        echo "More than one ISS_CMS_RESPONSE_*.zip files present in /mnt/rwflash."
        echo "Only copy the latest to /mnt/rwflash."
        exit 1
fi

cd /rwflash/aerolink/certificates

unzip -o -j $RWFLASH/ISS_CMS_RESPONSE_*.zip

# Delete the current ISS_CMS_RESPONSE_*.zip after unzipping the one
rm -f $RWFLASH/ISS_CMS_RESPONSE_*.zip

if [ -e $CERTDIR/RA.oer ] && [ -e $CERTDIR/ECA.oer ] && [ -e $CERTDIR/enrollment.oer ] && [ -e $CERTDIR/enrollment.s ] && [ -e $CERTDIR/CRL.oer ] && [ -e $CERTDIR/LCCF.oer ] && [ -e $CERTDIR/root.oer ]; then
    echo "Files are available to process EC Response"
else 
    echo "Copy the ISS_CMS_RESPONSE_*.zip to RSU's /mnt/rwflash/ before running processECRes.sh"
    exit 1
fi

# Command should be run from /usr/bin
cd /usr/bin/

export AEROLINK_STATE_DIR=/rwflash/aerolink
export AEROLINK_CONFIG_DIR=/rwflash/configs/aerolink_conf

process_cmd="processECResponse --contextName denso.wsc --lcmName rsuLcm --enrollmentResponse aerolink/certificates/enrollment.oer --lccf aerolink/certificates/LCCF.oer --crl aerolink/certificates/CRL.oer --lpf aerolink/certificates/LPF.oer --ra aerolink/certificates/RA.oer --root aerolink/certificates/root.oer --eca aerolink/certificates/ECA.oer --enrollmentReconstruction aerolink/certificates/enrollment.s"

echo $process_cmd

process_result=`eval "$process_cmd" 2>&1`

echo $process_result

echo ""
# Check if the process output is success
echo "processECResponse results"
if [[ $process_result == *"root.oer"* ]] && [[ $process_result == *"LCCF.oer"* ]] && [[ $process_result == *"CRL.oer"* ]]  && [[ $process_result == *"ECA.oer"* ]] && [[ $process_result == *"LPF.oer"* ]] && [[ $process_result == *"RA.oer"* ]] && [[ $process_result == *"enrollment.oer"* ]] && [[ $process_result == *"enrollment.s"* ]]; then
    echo "Process EC response done successfully"
else
    echo "Process EC response failed"
    exit 1
fi

# Copy the ws_ca.db.new to /mnt/rwflash for later retriveal
cp /rwflash/aerolink/wscs/ws_ca.* /mnt/rwflash/.
echo "ws_ca.db.new file is backed up to /mnt/rwflash"

echo ""
#Check the files are present after process EC Response
if [ -e $CERTDIR/enrollment.cert ] && [ -e $CERTDIR/local_policy_*.oer ]; then
    echo "Process EC Response created files"
else 
    echo "Process EC Response files NOT created"
fi
