#!/bin/sh
# This script runs once at startup based on u-boot env "run_once" flash
# to patition and format emmc/sdcard for rwflash mounting.
# Reboot is required after script complete.
#
# Date       / Author / Version / Description
# 2020.09.23 / JStinnett   / v1.0    / Initial release
# 2020.12.04 / / v1.1 / NOR boot adjustment; added ssh key generation


#Check if u-boot env says first run.  If so, test for logging partition
# using user space partition (/dev/mtd4; bs=4 skip=2 count=2)

USERPART=/dev/mtd8
TMPFILE=/tmp/runonce
CMPFILE=/tmp/cmpfile
# need to skip first 128 KB
DDARGS="bs=4 skip=32768 count=2"
WDDARGS="bs=4 seek=32768 count=2"
SETSTATE="runonce"

# SSH keygen -- this will need to be reexamined when the NOR goes R/O
DEFSSHDIR=/etc/ssh/
SSHDIR=/home/root/.ssh
SSHSAVEDIR=/mnt/rwflash/root/
SSHFILE=id_rsa
#[ ! -e $SSHSAVEDIR ] && mkdir -p $SSHSAVEDIR && cp $DEFSSHDIR/ssh_host* $SSHSAVEDIR
#if [[ -e $SSHDIR$SSHFILE && ! -z "`ls -al $SSHDIR$SSHFILE | grep ^-`" ]]; then
#  cp $SSHDIR$SSHFILE $SSHSAVEDIR
#  cp $SSHDIR$SSHFILE.pub $SSHSAVEDIR
#  rm $SSHDIR$SSHFILE $SSHDIR$SSHFILE.pub
#  cd $SSHDIR
#  ln -s $SSHSAVEDIR$SSHFILE $SSHFILE
#  ln -s $SSHSAVEDIR$SSHFILE.pub $SSHFILE.pub
#fi
#if [ ! -e $SSHSAVEDIR$SSHFILE ]; then
#  ssh-keygen -t rsa -f $SSHSAVEDIR$SSHFILE -q -P ""
#fi
# force every time
#cd $SSHDIR
#rm -f $SSHFILE $SSHFILE.pub
#ln -s $SSHSAVEDIR$SSHFILE $SSHFILE
#ln -s $SSHSAVEDIR$SSHFILE.pub $SSHFILE.pub
#cd - >/dev/null
#cp -f $SSHSAVEDIR/ssh_host* $DEFSSHDIR

if [ "`whoami`" != "root" ]; then
  exit 0
fi

echo $SETSTATE > $CMPFILE
dd if=$USERPART of=$TMPFILE $DDARGS 2>/dev/null
diff $CMPFILE $TMPFILE > /dev/null
if [ 0 -ne $? ]; then
  read -p "Detected first run, create logging partition? [Y/n] " -n 1 -r
  echo #Move to new line
  if [[ $REPLY =~ ^[Nn]$ ]]; then
    flash_erase $USERPART 0x20000 1 >/dev/null
    dd if=$CMPFILE of=$USERPART $WDDARGS 2>/dev/null
    rm -f $TMPFILE $CMPFILE
    exit 0
  elif [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 0
  fi  
else
  rm -f $TMPFILE $CMPFILE
  exit 0
fi

EMMC_BOOTDEV="/dev/mmcblk0"

#Check which mmcblk device is current boot device
BOOTDEV="$EMMC_BOOTDEV"

#Check if logging partition already exists.  Assume already formatted and exit
if [[ $(fdisk -l "$BOOTDEV") == *"$BOOTDEV"p3* ]]; then
  echo "run_once.sh: Logging partition "$BOOTDEV"p3 already exists!  Exiting."
  flash_erase $USERPART 0x20000 1 >/dev/null
  dd if=$CMPFILE of=$USERPART $WDDARGS 2>/dev/null
  rm -f $TMPFILE $CMPFILE
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
flash_erase $USERPART 0x20000 1 >/dev/null
dd if=$CMPFILE of=$USERPART $WDDARGS 2>/dev/null
rm -f $TMPFILE $CMPFILE
echo "run_once.sh: Done!"
echo "run_once.sh: Reboot to initialize logging partition."
