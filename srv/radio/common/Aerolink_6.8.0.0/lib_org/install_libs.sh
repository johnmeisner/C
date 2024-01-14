#!/bin/bash
#
#   install_libs.sh - Script to install libraries on the Hercules
#
export HERCIP=192.168.1.80
make clean install
scp libaerolinkPKI.so.1 root@$HERCIP:/usr/lib
scp libaerolinkPKI_dummy.so.1 root@$HERCIP:/usr/lib
scp libmisbehaviorReport.so.1 root@$HERCIP:/usr/lib
scp libviicsec.so.1 root@$HERCIP:/usr/lib
ssh -n root@$HERCIP "rm -f /usr/lib/libaerolinkPKI.so"
ssh -n root@$HERCIP "rm -f /usr/lib/libaerolinkPKI_dummy.so"
ssh -n root@$HERCIP "rm -f /usr/lib/libmisbehaviorReport.so"
ssh -n root@$HERCIP "rm -f /usr/lib/libviicsec.so"
ssh -n root@$HERCIP "ln -s libaerolinkPKI.so.1 /usr/lib/libaerolinkPKI.so"
ssh -n root@$HERCIP "ln -s libaerolinkPKI_dummy.so.1 /usr/lib/libaerolinkPKI_dummy.so"
ssh -n root@$HERCIP "ln -s libmisbehaviorReport.so.1 /usr/lib/libmisbehaviorReport.so"
ssh -n root@$HERCIP "ln -s libviicsec.so.1 /usr/lib/libviicsec.so"

