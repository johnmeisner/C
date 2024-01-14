#!/bin/sh
# Date       / Author / Version  
# 05.05.2021 / SBaucom   / v1.1 
# 03.16.2022 / JStinnett / v1.2 Removed microSD test option for RSU-5940, add support for new rwflash partition number

RWFLASH_MNT1="/dev/mmcblk0p1 on /mnt/rwflash type ext4 (rw,relatime,data=ordered" #w/o recovery image in eMMC
RWFLAHS_MNT2="/dev/mmcblk0p3 on /mnt/rwflash type ext4 (rw,relatime,data=ordered" #w/ recovery image in eMMC

function usage
{
    echo "device_test.sh <option>"
    echo "options: rwflash; usbdrv"
}

function verify_rw ()
{
    # Create random binary
    dd if=/dev/urandom of=/var/tmp/sample.bin bs=1M count=1 	
    
    # Copy to device
    cp /var/tmp/sample.bin /mnt/$1/sample.bin 	

    # Verify files are the same 	
    diff /var/tmp/sample.bin /mnt/$1/sample.bin &> /var/tmp/fdiff.txt
    filediff=`cat /var/tmp/fdiff.txt`
    rm /var/tmp/fdiff.txt
    #echo "FILEDIFF: $filediff"
    if [[ -n "$filediff" ]]; then 
        echo "Error copying file to $1"
    else
	echo "File copied successfully to $1"
    fi
    
    # Edit file    
    sed -i '1s/^/0000 /' /mnt/$1/sample.bin 		

    # Verify file is changed
    filediff=`diff /var/tmp/sample.bin /mnt/$1/sample.bin`	
    if [[ "$filediff" != "Binary files /var/tmp/sample.bin and /mnt/$1/sample.bin differ" ]]; then 
        echo "Error editing file in $1"
    else
	echo "File in $1 successfully edited"
    fi

    # Remove files
    rm /var/tmp/sample.bin /mnt/$1/sample.bin 	
}

results=PASS



if [[ "$1" == "" ]]; then
    usage
    
else    
    
    ##### RWFLASH/EMMC #####
    if [[ "$1" == "rwflash" ]]; then
        
    # Test that rwflash is mounted
	rwflash_mounted=`mount | grep mmcblk0`
	if [[ $rwflash_mounted == *$RWFLASH_MNT1* ]] || [[ $rwflash_mounted == *$RWFLASH_MNT2* ]]; then 
        #if [[ $rwflash_mounted == "/dev/mmcblk0p3 on /mnt/rwflash type ext4 (rw,relatime,data=ordered,x-systemd.automount)" ]]; then 
	    echo "rwflash is mounted"
	else
	    echo "rwflash is NOT mounted as expected"
	    echo "$rwflash_mounted"
	    results=FAIL
	fi
	
    # Test that RWFLASH is readable/writeable
	rwflash_rw=$(verify_rw "rwflash")
	echo "$rwflash_rw"
	if [[ "$rwflash_rw" == *"Error"* ]]; then 
	    results=FAIL
	fi

    # Print results
	echo ""
	echo "Result: $results"

        
    ##### USBDRV #####
    elif [[ "$1" == "usbdrv" ]]; then
        
	# Test that usbdrv is mounted
	usbdrv_mounted=`mount | grep usbdrv`
	if [[ $usbdrv_mounted == *"/dev/sda1 on /mnt/usbdrv"* ]]; then 
	    echo "usbdrv is mounted"
	else
	    echo "usbdrv is NOT mounted"
	    echo "$usbdrv_mounted"
	    results=FAIL
	fi

    # Test that usbdrv is readable/writeable
	usbdrv_rw=$(verify_rw $1)
	echo "$usbdrv_rw"
	if [[ "$usbdrv_rw" == *"Error"* ]]; then 
	    results=FAIL
	fi

    # Print results
	echo ""
	echo "Result: $results"

    
    else 
	echo "invalid device. "
	results=FAIL	
	usage
    echo "" 
    echo "Result: $results"
    fi
fi 
