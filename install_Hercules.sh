#!/bin/sh
#-------------------------------------------------------------------------------
# File: Source/C/install_Hercules.sh
#
# Copyright (C) 2020 DENSO International America, Inc.
#
# Description: Install DENSO software on the Hercules
#-------------------------------------------------------------------------------
echo Be sure the Hercules is in 'run rwboot' mode before running this script.
echo Also, put an 'exit' near the top of startup.sh before running this script
echo so that none of the binary executables that are to be copied over are
echo running.
cp -rd bin/* /usr/bin
cp -rd lib/* /usr/lib/
rm -f /usr/lib/libaerolinkPKI.so
ln -s libaerolinkPKI.so.1 /usr/lib/libaerolinkPKI.so
rm -f /usr/lib/libaerolinkPKI_dummy.so
ln -s libaerolinkPKI_dummy.so.1 /usr/lib/libaerolinkPKI_dummy.so
rm -f /usr/lib/libmisbehaviorReport.so
ln -s libmisbehaviorReport.so.1 /usr/lib/libmisbehaviorReport.so
rm -f /usr/lib/libviicsec.so
ln -s libviicsec.so.1 /usr/lib/libviicsec.so
cp /usr/bin/v2xscppalutil.bin /home/root
mkdir -p /usr/services/version
cp services/version/* /usr/services/version
rm -rf /etc/v2x/default
mkdir -p /etc/v2x/default
cp -rd config/* /etc/v2x/default
echo
echo Config files are in /etc/v2x/default. Copy them to /rwflash/configs if you
echo want to use them.

