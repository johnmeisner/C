#!/bin/bash
#
# Autotalks SDK helper
#   check - Verifies correct SDK is installed
#   install - Installs SDK
#
# One location to update SDK version number as new
#   versions come in, rather than multiple places in
#   the C/misc Makefile

USAGE="Usage: autotalks_installer.sh (check | install)"

DESIRED_AUTOTALKS_SDK_VERSION="secton-5.17.0-rel"
AUTOTALKS_SDK_TAR_FILE=autotalks_sdk_for_opt.tar.bz2

# check -- Check that our desired version of Autotalks SDK is installed
if [ "$1" == check ]; then
    # Check that there's a directory here
    if [ ! -d /opt/autotalks ]; then
        echo Error: Directory /opt/autotalks does not exist
        exit 1
    fi
    # Check that the desired SDK version is installed
    SDKVER=`egrep HOST_SDK_REV_ID /opt/autotalks/secton-5.17.0/include/atlk/verstring.h | awk '{print $3}' | sed 's/"//g'`
    if [ "$SDKVER" == $DESIRED_AUTOTALKS_SDK_VERSION ]; then
        echo Checked: Autotalks SDK is installed and up to date.
        exit 0
    else
        if [ x"$SDKVER" != x ]; then
            echo Error: Autotalks SDK version $SDKVER installed, please install version $DESIRED_AUTOTALKS_SDK_VERSION
            exit 1
        else
            echo Error: Broken or unknown Autotalks SDK installed, please install version $DESIRED_AUTOTALKS_SDK_VERSION
            exit 1
        fi
    fi

# install -- Install the Autotalks SDK
elif [ "$1" == install ]; then
    echo -n Installing Autotalks SDK ...' '
    # Now untar the SDK tar in /opt
    HERE=`pwd`
    cd /opt
    # Using sudo as /opt is root owned, and also so SDK files stay owned by root
    sudo tar xf $HERE/$AUTOTALKS_SDK_TAR_FILE
    echo done

# Usage
else

    echo "$USAGE"
    exit 1
fi

