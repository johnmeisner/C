#!/bin/sh
#
# quick_services
#   This provides services data for webgui in one script
#

#=== Services Running
TMP=/tmp/zsrv
/bin/rm -f $TMP
/bin/ps ax > $TMP
if [[ ! -z "`/bin/grep amh $TMP`" ]]; then echo Immediate Forward/Active Message , ; else echo , ; fi
if [[ ! -z "`/bin/grep scs $TMP`" ]]; then echo Direct Traffic Signal Controller SPaT ,; else echo , ; fi
if [[ ! -z "`/bin/grep ipb $TMP`" ]]; then echo IPv6 Over DSRC ,; else echo , ; fi
if [[ ! -z "`/bin/grep snmpd $TMP`" ]]; then echo SNMP ,; else echo , ; fi
if [[ ! -z "`/bin/grep srmrx_app $TMP`" ]]; then echo SRM RX ,; else echo , ; fi
if [[ ! -z "`/bin/grep spat16_app $TMP`" ]]; then echo BSM Forwarding ,; else echo , ; fi
rm $TMP

