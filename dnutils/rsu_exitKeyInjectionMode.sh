#!/bin/sh
#
#rsu_exitKeyInjectionMode.sh
# 37: Activate with Security Level 3:connect 5:getAppletVersion
# 38: endKeyInjection 0: Exit
cd /usr/bin
printf "37\n3\n5\n38\n0\n" | /usr/bin/v2xse-example-app > /mnt/rwflash/customer/exitKeyInjection.txt 2>&1
