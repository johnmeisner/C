#-------------------------------------------------------------------------------
# File: Source/C/srv/radio/common/Aerolink/bin/Makefile
#
# Copyright (C) 2020 DENSO International America, Inc.
#
# Description: Makefile for the Aerolink binaries
#----------------------------------------------------------------------------#

ifndef V2X_ROOT
   V2X_ROOT=../../../../..
endif

include ${V2X_ROOT}/v2xcommon.mk

.PHONY: install

install:
ifdef INSTALL_PATH
	@test -d $(INSTALL_PATH)/bin || mkdir -p $(INSTALL_PATH)/bin
	cp -d acfBuilder certadm crladm importSymmetricEncryptionKey ../security_zip_file/us_security_config_files_Hercules-Hardware.zip ../security_zip_file/us_security_config_files_Hercules-Software.zip $(INSTALL_PATH)/bin
	@test -d $(V2X_ROOT)/source/srv/radio/common/Aerolink/bin || mkdir -p $(V2X_ROOT)/source/srv/radio/common/Aerolink/bin
	cp -d acfBuilder certadm crladm importSymmetricEncryptionKey Makefile $(V2X_ROOT)/source/srv/radio/common/Aerolink/bin
	@test -d $(V2X_ROOT)/source/srv/radio/common/Aerolink/security_zip_file || mkdir -p $(V2X_ROOT)/source/srv/radio/common/Aerolink/security_zip_file
	cp ../security_zip_file/us_security_config_files_Hercules-Hardware.zip ../security_zip_file/us_security_config_files_Hercules-Software.zip $(V2X_ROOT)/source/srv/radio/common/Aerolink/security_zip_file
else
	@echo "ERROR: Aerolink/bin: Please define INSTALL_PATH before calling make install !"
	exit 1
endif

