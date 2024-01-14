#!/bin/sh
# Date       / Author / Version  
# 11.9.2020 / SBaucom   / v1.0   
# 02.16.2022 / JStinnett / v1.1 Updated RST and DAV I/O mappings for RSU-5940


# Change the working directory to /usr/bin where secur channel key is located
cd /usr/bin/

# Upgrade the JCOP 
# note: /dev/spidev0.0 = mainboard HSM
#       /dev/spidev2.0 = DSRC daughter board HSM           
v2xse-jcop-update /usr/src/wnc/sxf1800_firmware/R10.3_R12.1_KUD_NCK/ -t /dev/spidev0.0 -d 406 -r 488
v2xse-jcop-update /usr/src/wnc/sxf1800_firmware/R10.3_R12.1_KUD_NCK/ -t /dev/spidev2.0 -d 416 -r 384


# Upgrade V2X Applets (US/EU)
v2xse-applet-upgrade /usr/src/wnc/sxf1800_firmware/v2xApplet_2.12.3_NCK.apdu -t /dev/spidev0.0 -d 406 -r 488
v2xse-applet-upgrade /usr/src/wnc/sxf1800_firmware/v2xApplet_2.12.3_NCK.apdu -t /dev/spidev2.0 -d 416 -r 384

# Enable FIPS self-test at start-up and correct I2C settings
v2xse-jcop-config-power kf7 -t /dev/spidev0.0 -d 406 -r 488
v2xse-jcop-config-power kf7 -t /dev/spidev2.0 -d 416 -r 384

# To verify: 
# v2xse-se-info -t /dev/spidev0.0 -d 477 -r 206 # to check Mainboard HSM
# v2xse-se-info -t /dev/spidev2.0 -d 416 -r 401 # to check DSRC DB HSM