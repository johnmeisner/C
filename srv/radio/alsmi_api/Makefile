#-------------------------------------------------------------------------------
# File: Source/C/srv/radio/alsmi_api/Makefile
#
# Copyright (C) 2019 DENSO International America, Inc.
#
# Description: Makefile for ALSMI API
#----------------------------------------------------------------------------#

ifndef V2X_ROOT
   V2X_ROOT=../../..
endif

include ${V2X_ROOT}/v2xcommon.mk

export INC_DIR=../../../inc
export PAL_DIR=../../../pal
export RIS_DIR=../ris
export COMMON_DIR=../common
export AEROLINK_DIR=../common/Aerolink/include

CFLAGS += -Wall -Werror
INC=-I${AEROLINK_DIR} -I${PAL_DIR} -I${RIS_DIR} -I${COMMON_DIR} -I${INC_DIR}

#---ALSMI API Shared Library
ALSMI_API_TARGET = libalsmi_api.so

#
#---Specify targets for make all (default is only dynamic libraries):
#
ALL_LIBRARIES=${ALSMI_API_TARGET}

.PHONY: all clean install check

#---Default Rule:
all: ${ALL_LIBRARIES}

alsmi_api.o: alsmi_api.c ${COMMON_DIR}/alsmi_api.h
	$(CC) $(CFLAGS) ${INC} -fpic -c alsmi_api.c

#---ALSMI_API (Radio Stack API) Library
${ALSMI_API_TARGET}: alsmi_api.o
	$(CC) $(CFLAGS) -shared -lc  -Wl,-soname,${ALSMI_API_TARGET} -o ${ALSMI_API_TARGET} alsmi_api.o

clean:
	rm -rf ${ALL_LIBRARIES} *.o

#---Install into INSTALL_PATH.
install: all
ifdef INSTALL_PATH
	@test -d $(INSTALL_PATH)/lib || mkdir -p $(INSTALL_PATH)/lib
	cp -pd $(ALL_LIBRARIES) $(INSTALL_PATH)/lib/
else
	echo ERROR: srv/radio/alsmi_api: Please define INSTALL_PATH before calling make install!
	exit 1
endif

check:
	cppcheck -q --force --inline-suppr alsmi_api.c 
