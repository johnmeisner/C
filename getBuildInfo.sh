#!/bin/bash
#   ^--NOTE:
#     This is needed to be able to run this script from within Makefile;
#     otherwise ./getBuildInfo.sh: 26: ./getBuildInfo.sh: [[: not found
#-------------------------------------------------------------------------------
# File: Source/C/getBuildInfo.sh
#
# Copyright (C) 2021 DENSO International America, Inc.
#
# Description: Creates document to identify source code used to create the build
#-------------------------------------------------------------------------------

# ABS_PATH_TO_THIS_SCRIPTS_DIR:
# In order for svn info to get info, it has to be run inside an SVN tree
# path.  However, during a bitbake, the build is created in an off-svn, tmp
# path, so to get SVN info, we have to hardwire a SVN root path into this
# script, and cd into it to get svn info.   Note this issue is also solved
# in the fullgit bitbake recipe, which has a similar hardwired, host-path
# built into it for copying SVN source.  Similarly, we set ABS_PATH to
# point to our own script's directory, which lets us easily test that the
# path is correct, in case someone builds in an alternate dev setup without
# fixing their path into this script, and we can give that user a sensible error.
ABS_PATH_TO_THIS_SCRIPTS_DIR=/home/ivi/work/trunk/hercules/Software/iMX8/Source/C


# ABS_PATH_TO_BSP_PARENT_DIRECTORY
# In order to read the dnbspver file, we have to know where it is.  During a
# bitbake this is easy, as we are building from inside the BSP directory.
# However, during a local make, we need to know where it is.  This variable
# points us to it.  We will assume the BSP directory starts with "rsu5940_h7e".
ABS_PATH_TO_BSP_PARENT_DIRECTORY=/home/ivi/work


# Now getBuildInfo requires an output path so it can be run inside
# and outside bitbake.  We check for that parameter and show
# a Usage message if it's missing.
if [ $# == 1 ]; then
    OUTPUTFILE=$1
else
    echo Usage: ./getBuildInfo.sh OutputFile
    exit 1
fi

# Verify ABS_PATH_TO_THIS_SCRIPT points to this script
CHECK_PATH=`grep ABS_PATH_TO_THIS_SCRIPTS_DIR= $ABS_PATH_TO_THIS_SCRIPTS_DIR/getBuildInfo.sh | grep -v grep | sed -e 's/.*=//'`
if [ "$CHECK_PATH" != $ABS_PATH_TO_THIS_SCRIPTS_DIR ]; then
    echo ERROR: getBuildInfo.sh:
    echo '  ' I require the absolute path to myself to operate inside bitbake.
    echo '  ' Please update Source/C/getBuildInfo.sh and set variable
    echo '  ' ABS_PATH_TO_THIS_SCRIPTS_DIR to the absolute path to the
    echo '  ' Source/C directory.
    echo Aborting.
    exit 1
fi

# Figure out where the BSP directory is.
# If we in a bitbake, easy! We'll use pwd!
if [ "$PKG_CONFIG_DIR" != "" ]; then
    BSP_DIR=`pwd -P | perl -ne 'if ( /(.*\/rsu5940_h7e_[^\/]+)/ ) { print $1 }'`
else
    BSP_DIR=`/bin/ls -d $ABS_PATH_TO_BSP_PARENT_DIRECTORY/rsu5940_h7e*`
    # Make sure got only one
    COUNT=`echo $BSP_DIR | wc -w`
    if [ $COUNT == 0 ]; then
        echo Unable to find a BSP rsu5940_h7e* directory in $ABS_PATH_TO_BSP_PARENT_DIRECTORY
        exit 1
    elif [ $COUNT -gt 1 ]; then
        echo Found multiple BSP rsu5940_h7e* directories in $ABS_PATH_TO_BSP_PARENT_DIRECTORY
        echo I dont know which one to use
        exit 2
    fi
fi

# Get SVN info.
# Note: we have to be in a SVN-populated directory for svn info to work
pushd $ABS_PATH_TO_THIS_SCRIPTS_DIR > /dev/null
SVN_REVISION=`/usr/bin/svn info | grep Revision: | awk '{print $NF}'`
popd > /dev/null

# Get build-time related info
BUILD_DATE=`date -Iseconds`
BUILD_USER=${USER}
BUILD_MACHINE=`uname -n`
cd `pwd -P`     # Expands symbolic links into a real path to make GIT happy
BUILD_PATH=${PWD}

# Extract official build version from v2v_general.h
I2V_VERSION=`grep I2V_VERSION inc/v2v_general.h | sed -e 's/.*VERSION *//' -e 's/"//g'`

# Capture CFlags
# NOTE: In bitbake, the following make call prints "Entering/exiting directory"
#    as part of its output, which we dont want captured.  This is why we
#    grep -v directory.  Passing make -s didn't suppress this output either.
#    Also, bitbake duplicates the contents of the v2xcommon.mk CFLAGS, and adds
#    its own stuff, so we split output into separate lines, grep for -D for
#    only our v2xcommon stuff, and do a sort -u to remove the duplicates.
TMP=/tmp/xtmp.getBuildInfo.xtmp
make -s getCFLAGS | grep -v directory > $TMP.1
tr ' ' '\n' < $TMP.1 | grep \\-D | sed -e 's/^-D//' | sort -u | tr '\n' ' ' > $TMP.2
I2V_CFLAGS=`cat $TMP.2`
BUILD_CFLAGS=`cat $TMP.1`   # Leave BUILD_CFLAGS out of output; value very
                            #   busy from within bitbake. Add later if wanted
rm -f $TMP.?

# Extract bsp version

cd $BSP_DIR

BSP_REVISION=`/usr/bin/svn info | grep Revision: | awk '{print $NF}'`  

if [ -e $BSP_DIR/dnbspver ]; then
    BSP_VERSION=`grep Version $BSP_DIR/dnbspver | awk '{print $NF}'`
    # indicator that v2x security supported
    BSP_VERSION+="_ATTRIB:S"
else
    BSP_VERSION="pre-A6"
fi

# Extract build target (eliminating new line with tr)
if [ -e $BSP_DIR/buildsetup.txt ]; then
    BUILD_TARGET=`cat $BSP_DIR/buildsetup.txt | tr '
' ' '`
fi  

# Output
echo "BUILD_DATE         = $BUILD_DATE"         >  $OUTPUTFILE  
echo "BUILD_USER         = $BUILD_USER"         >> $OUTPUTFILE
echo "BUILD_MACHINE      = $BUILD_MACHINE"      >> $OUTPUTFILE
echo "BUILD_PATH         = $BUILD_PATH"         >> $OUTPUTFILE
echo "BSP_VERSION        = $BSP_VERSION"        >> $OUTPUTFILE
echo "BSP_REVISION       = $BSP_REVISION"       >> $OUTPUTFILE
echo "BUILD_TARGET       = $BUILD_TARGET"       >> $OUTPUTFILE
echo "SVN_REVISION       = $SVN_REVISION"       >> $OUTPUTFILE
echo "I2V_VERSION        = $I2V_VERSION"        >> $OUTPUTFILE
echo "I2V_CFLAGS         = $I2V_CFLAGS"         >> $OUTPUTFILE

echo Created $OUTPUTFILE

