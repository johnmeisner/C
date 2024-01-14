#-------------------------------------------------------------------------------
# File: Source/C/srv/radio/common/Aerolink/lib/Makefile
#
# Copyright (C) 2020 DENSO International America, Inc.
#
# Description: Makefile for the TLS library
#----------------------------------------------------------------------------#

ifndef V2X_ROOT
   V2X_ROOT=../../../../..
endif

include ${V2X_ROOT}/v2xcommon.mk

.PHONY: all clean install

LIBS=libaerolinkPKI.so.1
SYMLINKS=libaerolinkPKI.so
#PREBUILT_LIBS=libaerolinkPKI_dummy.so.1 libmisbehaviorReport.so.1 libviicsec.so.1 libcrypto.so.1.1 libv2xCryptoPal.so.2.5 libv2xCryptoPal.so libv2xCrypto.so.2.5 libv2xCrypto.so
PREBUILT_LIBS=libaerolinkPKI_dummy.so.1 libmisbehaviorReport.so.1 libviicsec.so.1 libcrypto.so.1.1 libv2xCrypto.so.2.5 libv2xCryptoPal.so.2.5

all: $(LIBS) $(SYMLINKS)

libaerolinkPKI.so: libaerolinkPKI.so.1
	ln -s libaerolinkPKI.so.1 libaerolinkPKI.so

libaerolinkPKI.so.1: libaerolinkPKI_dummy.so.1
	cp libaerolinkPKI_dummy.so.1 libaerolinkPKI.so.1

clean:
	rm -rf $(LIBS) $(SYMLINKS)

install: all $(PREBUILT_LIBS)
ifdef INSTALL_PATH
	@test -d $(INSTALL_PATH)/lib || mkdir -p $(INSTALL_PATH)/lib
	cp -d $(LIBS) $(PREBUILT_LIBS) $(INSTALL_PATH)/lib
else
	@echo "ERROR: srv/radio/ns-Makefile: Please define INSTALL_PATH before calling make install !"
	exit 1
endif

