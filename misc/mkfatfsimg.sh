#!/bin/bash
#
# mkfatfsimg - creates a vfat blob of a directory of files
# 
# NOTE: This utility needs /etc/fstab modified to work properly,
#   root is usually needed to mount filesystems, but this utility
#   utilized a "user" entry to allow an non-root users to mount
#   the vfat blob, to copy config files into it.
#
# Last Modified: 20210114

BLOBSIZE_IN_KBYTES=1024
MOUNTABLE_TMP_BLOB=/tmp/test.iso
MOUNTABLE_TMP_MOUNT=/tmp/test.mount

USAGE='Usage: mkfatfsimg.sh DirOfFiles PathToBlobToBeCreated'

FSTAB_MSG="Error: System not set up for mkfatfsimg.sh.

You must create an entry in /etc/fstab for mkfatfsimg.sh to work.
Add these two lines to /etc/fstab:

# Let a non-root user mount a test filesystem
$MOUNTABLE_TMP_BLOB $MOUNTABLE_TMP_MOUNT auto defaults,noauto,user 0 1
"

function echodie()
{
    echo ERROR: mkfatfsimg: "$1"
    exit 1
}


# Verify fstab contains our entry for non-root mount
egrep -q -l $MOUNTABLE_TMP_BLOB /etc/fstab || echodie "$FSTAB_MSG"

# Check usage
if [ $# != 2 ]; then
    echo "$USAGE"
    exit 0
fi

# Test that we can create temporary blob
touch $MOUNTABLE_TMP_BLOB || echodie "Failed to create temporary blob"

# Just in case, unmount our mount point
/bin/umount $MOUNTABLE_TMP_MOUNT 2> /dev/null

# Create our tmp mount point
mkdir -p $MOUNTABLE_TMP_MOUNT || echodie "Failed to create temporary mount point"

# Create Blob
dd if=/dev/zero bs=1k count=$BLOBSIZE_IN_KBYTES of=$MOUNTABLE_TMP_BLOB 2> /dev/null || echodie "Failed to create blob"

# Format vfat onto blob
/sbin/mkfs.vfat $MOUNTABLE_TMP_BLOB > /dev/null || echodie "Failed to format blob"

# Mount blob
/bin/mount $MOUNTABLE_TMP_MOUNT || echodie "Failed to mount blob at temporary mount"

# Copy files into blob
cp -r "$1"/* $MOUNTABLE_TMP_MOUNT || echodie "Failed to copy files into blob"

# Unmount blob
/bin/umount $MOUNTABLE_TMP_MOUNT || echodie "Failed to unmount temporary mount"

# Remove tmp mount point
rmdir $MOUNTABLE_TMP_MOUNT || echodie "Failed to remove temporary mount"

# Move blob to its destination
mv $MOUNTABLE_TMP_BLOB $2 || echodie "Failed to move temporary blob to $2"

# Done
