#!/bin/sh
# This script runs once at startup based on u-boot env "run_once" flash
# to patition and format emmc/sdcard for rwflash mounting.
# Reboot is required after script complete.
#
# Date       / Author / Version / Description
# 2020.09.23 / JStinnett   / v1.0    / Initial release


if [ "`whoami`" != "root" ]; then
  exit 0
fi

#Check if u-boot env says first run.  If so, test for logging partition
if [[ $(fw_printenv) == *run_once=1* ]]; then
  read -p "Detected first run, create logging partition? [Y/n] " -n 1 -r
  echo #Move to new line
  if [[ $REPLY =~ ^[Nn]$ ]]; then
    fw_setenv run_once 0
    exit 0
  elif [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 0
  fi  
else
  exit 0
fi

MICROSD_BOOTDEV="/dev/mmcblk1"
EMMC_BOOTDEV="/dev/mmcblk0"

#Check which mmcblk device is current boot device
if [[ $(cat /proc/cmdline) == *"$MICROSD_BOOTDEV""p2"* ]]; then
  echo "run_once.sh Boot from microSD"
  BOOTDEV="$MICROSD_BOOTDEV"
elif [[ $(cat /proc/cmdline) == *"$EMMC_BOOTDEV""p2"* ]]; then
  echo "run_once.sh: Boot from eMMC"
  BOOTDEV="$EMMC_BOOTDEV"
else
  echo "run_once.sh: Cannot detect boot device! Exiting."
  exit 1
fi

#Check if logging partition already exists.  Assume already formatted and exit
if [[ $(fdisk -l "$BOOTDEV") == *"$BOOTDEV"p3* ]]; then
  echo "run_once.sh: Logging partition "$BOOTDEV"p3 already exists!  Exiting."
  exit 1
fi

#Find last sector of last partition on mmcblk.  Make logging partition start +1 after last sector
LAST_SECTOR="$(fdisk -l "$BOOTDEV" | grep ^"$BOOTDEV"p2 | awk -F" " '{ print $3 }')"
echo "$LAST_SECTOR"
START_SECTOR=$((LAST_SECTOR+1))
echo "$START_SECTOR"

echo "run_once.sh: Creating logging partition "$BOOTDEV"p3"
#Modify partition table with new logging partition /dev/mmcblkXp3
fdisk $BOOTDEV <<EOF
n
p
3
$START_SECTOR

w
EOF

#Format logging partition with EXT4
echo "run_once.sh: Formatting logging partition "$BOOTDEV"p3"
mkfs.ext4 $BOOTDEV"p3"

#Set u-boot run_once to 0 so script does not run again.
fw_setenv run_once 0
echo "run_once.sh: Done!"
echo "run_once.sh: Reboot to initialize logging partition."
