#!/bin/sh
#
#   build_openssl.sh - Script to build openssl v1.1.1c
#
tar -xvf openssl-1.1.1c.tar.gz 
cd openssl-1.1.1c/
./Configure --prefix=/tmp/openssl --openssldir=/tmp/openssl shared enable-ec_nistp_64_gcc_128 linux-aarch64
make CC="$CC" RANLIB="$RANLIB" AR="$AR" install
$STRIP libcrypto.so.1.1
sudo cp -d libcrypto.so* ..

