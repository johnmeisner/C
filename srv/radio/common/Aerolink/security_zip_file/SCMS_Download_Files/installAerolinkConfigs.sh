# This script will remove all previously loaded certificates from the Hercules
# and re-install the base Aerolink Configuration files needed to communicate
# with the SCMS

# Wait for the year to become something other than 1970 (for TPS to set the
# date)
echo "Waiting for tps to set the date and time..."

year=$(date | awk {'print $6'})

while [ "$year" == "1970" ]
do
    sleep 1
    year=$(date | awk {'print $6'})
done

rm -rf /mnt/rwflash/aerolink
cp -r /mnt/rwflash/SCMS_Download_Files/Aerolink/state /mnt/rwflash/aerolink
echo "Base Aerolink configuration files for HSM usage have been successfully installed"
