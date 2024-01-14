#!/bin/bash
#
# GET_CONFIG_VALUE -- utility to grep config setting value directly,
#                     used to confirm webgui updates worked
#
# NOTE: This is called only by cgi-bin scripts, but it cannot be placed
#       in cgi-bin; else RSU is vulnerable to attackers calling it
#       directly and passing in hostile input.

CONFIGDIR=/rwflash/configs

if [ $# != 2 ]; then
    echo USAGE: get_conf_val.sh config_file config_setting_name
    exit 0
fi

# When config_file=RSU_INFO .. it's not a true config file, it's
# a magic word to conf_agent/conf_manager, but RSU_INFO's network
# items are in startup.sh, and we can treat startup.sh as a
# config file!  We translate three of the RSU_INFO queries here.
# Add more as needed.
if [ $1 == RSU_INFO ]; then
    if [ $2 == NETWORK_MODE ]; then         # NETWORK_MODE was called DHCP_ENABLE/DHCP_MODE
        SETTINGNAME=DHCP_ENABLE             # Currently called DHCP_ENABLE in startup.sh
        CONFIGFILE=startup.sh
    elif [ $2 == IP_FOR_STATIC_ADDRESS ]; then
        SETTINGNAME=$2
        CONFIGFILE=startup.sh
    elif [ $2 == SC0IPADDRESS ]; then       # Old name used in scripts
        SETTINGNAME=IP_FOR_STATIC_ADDRESS
        CONFIGFILE=startup.sh
    elif [ $2 == NETMASK ]; then            # NETMASK in conf_agent and scripts
        SETTINGNAME=NETMASK_FOR_STATIC_ADDRESS   # startup.sh name
        CONFIGFILE=startup.sh
    elif [ $2 == NETMASK_FOR_STATIC_ADDRESS ]; then
        SETTINGNAME=$2
        CONFIGFILE=startup.sh
    elif [ $2 == MTU ]; then
        SETTINGNAME=$2
        CONFIGFILE=startup.sh
    elif [ $2 == VLAN_ENABLE ]; then
        SETTINGNAME=$2
        CONFIGFILE=startup.sh
    elif [ $2 == VLAN_IP ]; then
        SETTINGNAME=$2
        CONFIGFILE=startup.sh
    elif [ $2 == VLAN_MASK ]; then
        SETTINGNAME=$2
        CONFIGFILE=startup.sh
    elif [ $2 == VLAN_ID ]; then
        SETTINGNAME=$2
        CONFIGFILE=startup.sh
    elif [ $2 == WEBGUI_HTTP_SERVICE_PORT ]; then
        SETTINGNAME=$2
        CONFIGFILE=startup.sh
    elif [ $2 == WEBGUI_HTTPS_SERVICE_PORT ]; then
        SETTINGNAME=$2
        CONFIGFILE=startup.sh
    else 
        echo ERROR:GCVNeedsProgramming
        exit 0
    fi

# For normal configs, the .conf is optional, so figure out if .conf needs to be added
else
    SETTINGNAME=$2
    if [ -f $CONFIGDIR/$1 ]; then
        CONFIGFILE=$1
    else
        if [ -f $CONFIGDIR/$1.conf ]; then
            CONFIGFILE=$1.conf
        else
            echo ERROR: Config file $CONFIGDIR/$1 does not exist!
            exit 1
        fi
    fi
fi

# Grep out the value.

# Config Type: Has Equals or Implied Equals
#    Most configs are of type A=B or A=B;C,D   (ie Key = Value)
#    Config snmpd.conf is different -- it is of type A B (ie Key Value -- implied equals)

if [ $CONFIGFILE != snmpd.conf ]; then

    # Normal case to extract value - has the equals

    # NOTE: Note the '[ =]' in the grep regex adds focus, it handles cases when
    #   the desired config-setting's name is part of a different setting's name.
    #   For example: in amh.conf, ImmediateIP and ImmediateIPFilter, and wanting
    #   the ImmediateIPO value.  Without this addition to the regex, the grep
    #   pulls multiple rows, giving invalid data.
    GREPOUT=`grep '^'$SETTINGNAME'[ =]' $CONFIGDIR/$CONFIGFILE`
    if [ x"$GREPOUT" == x ]; then
        echo ERROR: Setting $SETTINGNAME does not appear in config file $CONFIGFILE
        exit 2
    fi

    # Munch out the value only
    VALUE=`echo "$GREPOUT" | sed -e 's/;.*//' -e 's/#.*//' -e 's/.*=//' -e 's/^  *//' -e 's/  *$//'`

# SNMPD.CONF -- Handle its key-value format separately
else 

    GREPOUT=`grep '^'$SETTINGNAME' ' $CONFIGDIR/$CONFIGFILE`
    if [ x"$GREPOUT" == x ]; then
        echo ERROR: Setting $SETTINGNAME does not appear in config file $CONFIGFILE
        exit 2
    fi

    # Munch out the value only
    VALUE=`echo "$GREPOUT" | awk '{print $2}'`
fi

# And return
echo "$VALUE"
