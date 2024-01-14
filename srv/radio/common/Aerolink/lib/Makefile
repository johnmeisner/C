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

LDNAME  = libaerolinkPKI.so
SONAME1  = $(LDNAME).1

# Source Directories
INC_DIR = ${V2X_ROOT}/inc
PAL_DIR = ${V2X_ROOT}/pal
FAC_DIR = ${V2X_ROOT}/fac
OTAP_DIR = ${FAC_DIR}/otap 
SXF1800_DIR = ${V2X_ROOT}/${SXF1800_ROOT}/install

LIBS=libaerolinkPKI.so.1
SYMLINKS=libaerolinkPKI.so
PREBUILT_LIBS=libaerolinkPKI.so.1 libmisbehaviorReport.so.1 libviicsec.so.1 libcrypto.so.1.1

PREBUILT_INC=/opt/fsl-imx-xwayland/4.14-sumo/sysroots/aarch64-poky-linux/usr/include

# Compiler Includes
CFLAGS += -I.
CFLAGS += -I$(INC_DIR)
CFLAGS += -I$(PAL_DIR)
#CFLAGS += -I$(FAC_DIR)/vod
#CFLAGS += -I$(FAC_DIR)/wmh
CFLAGS += -I$(V2X_ROOT)/srv/radio/ris
CFLAGS += -I$(V2X_ROOT)/srv/radio/common
CFLAGS += -I$(V2X_ROOT)/srv/radio/common/Aerolink/include
CFLAGS += -I$(OTAP_DIR) 
CFLAGS += -I$(PREBUILT_INC)

#CFLAGS_APP = -Vgcc_ntoarmv7le -w9 -Wall -g -O2 -I$(V2X_ROOT)/C/srv/radio/common/Aerolink/include
CFLAGS_APP = -Wall -g -O2 -I$(V2X_ROOT)/srv/radio/common/Aerolink/include

# Package Sources
SRCS = tlsImpl.c tlsSocket.c
HDRS = tlsImpl.h $(V2X_ROOT)/srv/radio/common/Aerolink/include/aerolinkPKIConnection.h 

# Binaries
LIBS = $(LDNAME)

LIB_PATH_APP = -L. -L${PAL_DIR}
LIBS_APP = -laerolinkPKI -lpal 

export ALL_LIBRARIES=./libaerolinkPKI.so.1
export ALL_LIB_VARIANTS=./libaerolinkPKI.so

#---Rules:

.PHONY: all clean install 

# Default Rule
all: clean $(LIBS)

# Library Rule
$(LDNAME): $(SRCS) $(HDRS) ${PAL_DIR}/libpal.so
	${CC} $(CFLAGS) -shared -Wall -fPIC -lpthread -lpal -lm -lssl -lcrypto ${SRCS} -Wl,-soname,${SONAME1} -o ${SONAME1} -L${LIB_PATH} -L${PAL_DIR} 
	ln -sf $(SONAME1) $(LDNAME)

libaerolinkPKI_test: libaerolinkPKI_test.c $(LDNAME) ${PAL_DIR}/libpal.so
	${CC} $(CFLAGS_APP) -o libaerolinkPKI_test libaerolinkPKI_test.c $(LIB_PATH_APP) $(LIBS_APP)

# Cleanup Rule:
clean:
	rm -rf $(LDNAME) $(SONAME1) libaerolinkPKI_test

# Library Install Rule:
install: all $(PREBUILT_LIBS)
ifdef INSTALL_PATH
	@test -d $(INSTALL_PATH)/lib || mkdir -p $(INSTALL_PATH)/lib
	cp -d $(LIBS) $(PREBUILT_LIBS) $(INSTALL_PATH)/lib
	cp -d tls.conf $(INSTALL_PATH)/config
	mkdir -p $(INSTALL_PATH)/share/SCMS_Download_Files/Aerolink
	cp -r ../security_zip_file/SCMS_Download_Files/Aerolink/state $(INSTALL_PATH)/share/SCMS_Download_Files/Aerolink/
	cp -r ../security_zip_file/SCMS_Download_Files/installAerolinkConfigs.sh $(INSTALL_PATH)/share/SCMS_Download_Files/
	cp -d ../security_zip_file/SCMS_Download_Files/Misc_Files/* $(INSTALL_PATH)/bin/
	cp -r ../aerolink_conf/ $(INSTALL_PATH)/config/
	cp -d ../scms_scripts/* $(INSTALL_PATH)/bin/
else
	cp -d libaerolinkPKI_test $(INSTALL_PATH)/bin
	@echo "ERROR: Aerolink/lib: Please define INSTALL_PATH before calling make install !"
	exit 1
endif
