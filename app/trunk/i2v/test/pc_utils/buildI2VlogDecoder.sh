#!/bin/bash

set -e

export BASEDIR=`dirname $(readlink -f $0)`
export BUILDDIR=.build_log_decoder
export TARGETDIR=deploy
export BUILDD=$BASEDIR/$BUILDDIR
export TARGETD=$BASEDIR/$TARGETDIR

if [ '' = `echo $BASEDIR | grep I2V` ]; then
  echo Error: you need to build this within your build tree where you found the code
  exit -1
fi

export TOPDIR=`dirname $(echo $BASEDIR | sed 's/\(.*\/I2V\).*/\1/')`
export ASN1DIR=$TOPDIR/WSUServices/Common/asn1_util
export J2735=$TOPDIR/WSUServices/Common/asn1_util/J2735
echo removing old work
rm -rf $BUILDD
rm -rf $TARGETD

mkdir $BUILDD
mkdir $TARGETD

tar xzf $ASN1DIR/tools/*ppc* --directory $BUILDD 2>&1 1>/dev/null
cp build_local.sh $BUILDD/ppclib/c/build_lib
cd $BUILDD/ppclib/c/build_lib
echo building asn1 libraries
echo this will take a few moments...
./build_local.sh >/dev/null 2>&1
cp $BUILDD/ppclib/c/lib/*.so $TARGETD
cd $BUILDD/ppclib/c/lib
cp $BASEDIR/logdecode.c .
echo building utility 
gcc -I $ASN1DIR -L. -lasn1rt -lm -c $J2735/*.c >/dev/null 2>&1
gcc -I $ASN1DIR -I $J2735 logdecode.c *.o -o i2vlogdecoder -L. -lasn1per -lasn1ber -lasn1rt
cp i2vlogdecoder $TARGETD
cd $BASEDIR
rm -rf $BUILDD
echo build completed successfully - artifacts in $TARGETD

