#-------------------------------------------------------------------------------
# File: Source/C/srv/radio/common/Aerolink/bin/Makefile
#
# Copyright (C) 2020 DENSO International America, Inc.
#
# Description: Makefile for the TLS library
#----------------------------------------------------------------------------#

ifndef V2X_ROOT
   V2X_ROOT=../../../../..
endif

include ${V2X_ROOT}/v2xcommon.mk

.PHONY: install

PREBUILT_BINS=acfBuilder certadm crladm importSymmetricEncryptionKey

install: $(PREBUILT_BINS)
ifdef INSTALL_PATH
	mkdir -p $(INSTALL_PATH)/bin
	cp -d $(PREBUILT_BINS) $(INSTALL_PATH)/bin
else
	@echo "ERROR: srv/radio/common/Aerolink/bin-Makefile: Please define INSTALL_PATH before calling make install !"
	exit 1
endif

