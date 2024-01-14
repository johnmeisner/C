#!/bin/sh
#
# collect_onceperboot_data.sh
#   This script gets the collect-once-per-boot device properties and versions
#
# PARAMETERS: OUTPUTFILE
#

TMP=/tmp/ztmp.oncedatas.$$

# Needed for read_device_id.sh to find denso_nor_read
export PATH=/usr/local/dnutils:$PATH

# Verify user is actually logged in - conf_agent returns client ip for authorized clients, 0.0.0.0 if not
LOGGED_IP=`/usr/bin/conf_agent SESSION_CHK $REMOTE_ADDR`
if [ "$LOGGED_IP" != "$REMOTE_ADDR" ]; then 
  # Unauthorized users get an error message.  This script is run from within
  #   a cgi-bin script, not called via http, so any error is plain text too
  echo 'ERROR: You are not authorized to call this script.'
  exit 3
fi

# Check usage
if [ $# == 0 ]; then
    echo Usage: '<this_script>' OUTPUTFILE
    exit 1
fi

OUTFILE=$1
if [ -e $OUTFILE ]; then
    echo ERROR: File $OUTFILE already exists';' aborting!
    exit 2
fi

#=== DEVICE ID via denso_nor_read - Trac 2571 ===
    DEVID=`denso_nor_read | dnr2str 0 -`
    # If not a printable string, show the hex dump instead
    if [ x"$DEVID" == x ]; then
        DEVID=`denso_nor_read | head -2 | cut -c 9-`
    fi
    echo Device ID: $DEVID >> $OUTFILE

#=== CPU UniqueID - Trac 2560 ===
    CPUID=`/usr/src/wnc/scripts/get_unique_id.sh | grep UNIQUE_ID_HEX | awk '{print $NF}'`
    echo CPU Unique ID: $CPUID >> $OUTFILE

#=== RH850 Version - Trac 2562 ===
    RHVER=`/usr/bin/quickrh850 fwver`
    if [ x"$RHVER" == x ]; then
        echo RH850 Version: Unavailable >> $OUTFILE
    else
        echo RH850 Version: "$RHVER" >> $OUTFILE
    fi

#=== UBlox Versions - Trac 2565 ===
    /usr/bin/quickubloxvers >> $OUTFILE

#=== Ethernet MAC address - Trac 2566 ===
    # NOTE: We have to capture MAC in a variable and echo it; we cant do an
    # echo -n followed by the ifconfig | grep , because if there's no grep
    # output, awk won't add a newline to end the echo -n, and our following
    # output will be messed up.
    MAC=`/sbin/ifconfig eth0 | grep HWaddr | awk '{print $NF}'`
    echo Ethernet MAC address: $MAC >> $OUTFILE

    # Formats the five version lines to be indented under one HSM header line
    # The JCOP and Platform lines have leading tabs, hence the \s* in their indent seds
    v2xse-se-info > $TMP
    TESTSTR=`grep Platform.configuration $TMP`
    if [ x"$TESTSTR" == x ]; then
        echo $HSM HSM info unavailable >> $OUTFILE
    else
        echo -n $HSM HSM JCOP version:' ' >> $OUTFILE
        grep JCOP.version $TMP | awk '{print $NF}' >> $OUTFILE
        echo -n $HSM HSM Platform configuration:' ' >> $OUTFILE
        grep Platform.config $TMP | awk '{print $NF}' >> $OUTFILE
        echo -n $HSM HSM US Applet version:' ' >> $OUTFILE
        grep US.Applet $TMP | awk '{print $NF}' >> $OUTFILE
        echo -n $HSM HSM EU Applet version:' ' >> $OUTFILE
        grep EU.Applet $TMP | awk '{print $NF}' >> $OUTFILE
        echo -n $HSM HSM GSA version:' ' >> $OUTFILE
        grep of.GSA $TMP | awk '{print $NF}' >> $OUTFILE
    fi

#=== DSRC SDR Firmware version - Trac 2568 ===
    llc version > $TMP
    grep SDR.Firmware $TMP | sed -e's/.*Firmware\s*:/DSRC SDR version:/' >> $OUTFILE

#=== CV2X Firmware version - Trac 2570 ===
    diff_file=`/usr/bin/diff /usr/src/wnc/CV2X_ES3_Config/SECTON.packed_bin.rom /tftpboot/SECTON.packed_bin.rom`
    if [ $? == 0 ]; then
        VERSION="CV2X_ES3_Config"
    else
        VERSION="CV2X_ES2_Config"
    fi
    echo C-V2X Firmware version: $VERSION >> $OUTFILE

#=== BSP Version - Trac 2569 ===
    # NOTE: Needs egrep not grep for the '|''s
    echo -n BSP Build Version:' ' >> $OUTFILE
    egrep 'BSP_VERSION|SVN_REVISION' /usr/share/BuildInfo.txt \
        | awk '{print $NF}' | tr \\012 / | sed -e 's!/$!!' >> $OUTFILE
    echo '' >> $OUTFILE

#=== BSP Revision - Trac 3161 ===
    # NOTE: Needs egrep not grep for the '|''s
     echo -n BSP Build Revision:' ' >> $OUTFILE
     egrep 'BSP_REVISION|BSP_REVISION' /usr/share/BuildInfo.txt \
         | awk '{print $NF}' | tr \\012 / | sed -e 's!/$!!' >> $OUTFILE
     echo '' >> $OUTFILE


#=== eMMC Utilization - Trac 2580
    echo -n eMMC Utilization:' ' >> $OUTFILE
    df -h | grep mnt.emmc | awk '{print $5 " (" $3 " of " $2 " used)"}' >> $OUTFILE

# Cleanup
rm $TMP

