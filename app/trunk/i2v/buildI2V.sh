#!/bin/bash
#############################################################
# This script will build i2v
# the purpose of this script is to build i2v
# ENHANCEMENT:
# add validation in the future - it currently is a passthru
#
# Running this script:
#   $ buildI2V.sh [OPTIONS] 
#
# you can alternatively build i2v by navigating to the main
# directory and typing make
# that will build all i2v applications
#
# if you want to build i2v with config options
# such as what you'd find in buildWSU.conf
# use this script
#
# [OPTIONS] defined below
#
# Params: -a APP_GROUP     capitalized
#         -b BIN_PATH      binary app directory
#         -c BUILD_DIR     config directory
#         -d | --clean     clean 
#         -f FEATURE_LIST  comma separated
#         -h | --help      help
#         -m FEATURE_FILE  fully qualified path to file 
#                          to save feature list
#         -l LIB_PATH      lib directory
#         -s SOURCE_TAR    source tarball generation
#                          (no build of binaries happens)
#
# all params optional
# no input will be as good as navigating to main and 
# typing make
###########################################################
#set -e 

BASEDIR=$(readlink -f $(dirname $0))
PARENTDIR=$BASEDIR/..
APP_GROUP=
FEATURE_LIST=
BIN_PATH=
LIB_PATH=
BUILD_DIR=
TARFILE=
TARGET=wsu
TMPDIR=
TMPFILE=

# keep the list of APP_GROUP and FEATURE_LIST up to date
# this must match the makefile to be user friendly
usage()
{
  echo
  echo
  echo "buildI2V [OPTIONS]"
  echo "  where supported options are:"
  echo "    -a APP_GROUP     i2v target application group to build"
  echo "    -b BIN_PATH      directory where binary apps are placed after the build"
  echo "    -c BUILD_DIR     directory where config files are placed after the build"
  echo "    -d or --clean    clean build artifacts"
  echo "    -f FEATURE_LIST  comma separated list of optional build features"
  echo "    -h or --help     print this help message"
  echo "    -m FEATURE_FILE  file to save features included in build"
  echo "    -l LIB_PATH      directory where shared libraries are placed after the build"
  echo "    -s TARBALL_NAME  build source tarball ONLY using TARBALL_NAME"
  echo "    -t target        build target (i.e. wsu OR WSU-5001)"
  echo "    -v               validate args ONLY" 
  echo
  echo "  Supported APP_GROUPs:"
  echo "    all_apps        all i2v apps"
  echo "    i2v_base        skeleton i2v"
  echo "    rse_simulation  spat simulator"
  echo 
  echo "  Supported FEATURE_LIST"
  echo "    rse_peagle"
  echo "    rse_euproto"
  echo "    rse_simulator   Use this instead of rse_simulation app_group for now"
  echo "    rse_asc_snmp"
  echo "    gs2_support"
  echo
}

# called by makefile
list_source()
{
  while [ $# -gt 0 ];
  do
    echo $1 >> $TMPFILE
    shift
  done
}
export -f list_source

# arg 1 must be tarball name (assummed to have extension)
save_source()
{
  TARBALL=$1
  rm -rf $TARBALL
  for i in `cat $TMPFILE`; do
    for j in ${!i}; do
      fd=`dirname $j`
      mkdir -p $TMPDIR/target/$fd
      cp -rf $PARENTDIR/$j $TMPDIR/target/$fd
    done
  done
  echo Creating source tarball $TARBALL in $BUILD_DIR
  cd $TMPDIR/target
  rm -rf `find . -type d | grep .svn`
  tar czf $BUILD_DIR/$TARBALL ./*
  cd $BASEDIR
  rm -rf $TMPDIR
}


while [ $# -gt 0 ]
do
  case "$1" in 
    '-a')
      APP_GROUP=$2
      if [ "`echo $APP_GROUP | grep ^-`" ]; then unset APP_GROUP; fi
      shift
      ;;
    '-b')
      BIN_PATH=$2
      if [ "`echo $BIN_PATH | grep ^-`" ]; then unset BIN_PATH; fi
      shift
      ;;
    '-c')
      BUILD_DIR=$2
      if [ "`echo $BUILD_DIR | grep ^-`" ]; then unset BUILD_DIR; fi
      shift
      ;;
    '-d' | '--clean')
      DELETE="true"
      shift
      ;;
    '-f')
      FEATURE_LIST=$2
      if [ "`echo $FEATURE_LIST | grep ^-`" ]; then unset FEATURE_LIST; fi
      shift
      ;;
    '-h' | '--help')
      usage && exit 0
      ;;
    '-m')
      SAVE_MANIFEST=$2
      shift
      ;;
    '-l')
      LIB_PATH=$2
      if [ "`echo $LIB_PATH | grep ^-`" ]; then unset LIB_PATH; fi
      shift
      ;;
    '-s')
      BUILD_SRC_TAR="true"
      SRC_TARBALL=$2
      shift
      ;;
    '-t')
      TARGET=$2
      shift
      ;;
    '-v')
      VALIDATE_ARGS="true"
      shift
      ;;
    *)
      shift
      ;;
  esac
done

export I2V_DEPLOY_LIB_PATH=$LIB_PATH
export FEATURE_LIST=`echo $FEATURE_LIST | tr a-z A-Z` # feature list is all upper
export BUILD_DIR=$BUILD_DIR
export BIN_PATH=$BIN_PATH
export APP_GROUP=`echo $APP_GROUP | tr A-Z a-z` # app group is all lower
export SRC_TARBALL=$SRC_TARBALL

# validation is performed by the makefile for app_group and features
# invalid features in feature list ignored
# directories garbage in garbage out (dirs not validated)
cd $BASEDIR/main
if [ ! -z "$VALIDATE_ARGS" ]; then
  echo I2V argument validation ...
  make app_check APP_GROUPS=$APP_GROUP FEATURES=$FEATURE_LIST
  [ $? -eq 0 ] && exit 0 || exit 1
fi
if [ ! -z "$BUILD_SRC_TAR" ]; then
  . ${BASEDIR}/main/source_matrix
  [ -z "$BUILD_DIR" ] && export BUILD_DIR=/tftpboot
  export TMPDIR=`mktemp -d /tmp/i2vbldXXXX`
  export TMPFILE=$TMPDIR/contents
  make srctar APP_GROUPS=$APP_GROUP FEATURES=$FEATURE_LIST
  save_source $SRC_TARBALL
  exit 0
fi
if [ -z "$DELETE" ]; then
  [ ! -z "$SAVE_MANIFEST" ] && [ -e "$SAVE_MANIFEST" ] && echo I2V:$FEATURE_LIST >> $SAVE_MANIFEST
  make target=$TARGET APP_GROUPS=$APP_GROUP FEATURES=$FEATURE_LIST
  [ $? -eq 0 ] && echo reminder: all conf files copied to config directory - only use the ones needed to match app group you selected 
else
  make clean target=$TARGET APP_GROUPS=$APP_GROUP FEATURES=$FEATURE_LIST
fi

# wsu 1.0/1.5 only required cleanup
if [ -z $TARGET ] || [ "$TARGET" = "wsu" ] || [ "$TARGET" = "WSU" ]; then
  if [ ! -z $ROOTFS_DIR ]; then
    # jpg removal - no gui for i2v (these take up /mnt/rwfs space)
    rm -f $ROOTFS_DIR/rootfs/etc/blackM*
    rm -f $ROOTFS_DIR/rootfs/etc/WSU_C*
    # unused directory (on /mnt/rwfs) cleanup
    rm -rf $ROOTFS_DIR/rootfs/var/www
    # log message cleanup observed
    mkdir -p $ROOTFS_DIR/rootfs/var/spool/cron/crontabs
    mkdir -p $ROOTFS_DIR/rootfs/usr/bin
    cp $BASEDIR/main/crontabs/root $ROOTFS_DIR/rootfs/var/spool/cron/crontabs/
    cp $BASEDIR/main/crontabs/klogcln $ROOTFS_DIR/rootfs/usr/bin/
  fi
fi
