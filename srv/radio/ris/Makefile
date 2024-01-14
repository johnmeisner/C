#-------------------------------------------------------------------------------
# File: Source/C/srv/radio/ris/Makefile
#
# Copyright (C) 2020 DENSO International America, Inc.
#
# Description: Makefile for RIS
#------------------------------------------------------------------------------

ifndef V2X_ROOT
   V2X_ROOT=../../..
endif

include ${V2X_ROOT}/v2xcommon.mk

#---Directories
COMMON_DIR=../common
INC_DIR=../../../inc
PAL_DIR=../../../pal
#SR_DIR=../../sr
AEROLINK_DIR=../common/Aerolink/include

INC=-I. -I${INC_DIR} -I${COMMON_DIR} -I${AEROLINK_DIR} -I${PAL_DIR}
OBJS=ris.o  ipcsock.o  libipv6.o type2str.o

#---RIS Shared Library
RIS_SONAME = libris.so

CFLAGS+=-Wall
CFLAGS += $(LDFLAGS)    # For OpenEmbedded GNU_HASH in binary

.PHONY: all clean install check

#---Default Rule:
all: ${RIS_SONAME}

ris.o: ris.c ris.h ris_dbg.h   ${COMMON_DIR}/ris_struct.h ${COMMON_DIR}/rs.h ${INC_DIR}/dn_types.h ${PAL_DIR}/ipcsock.h
	$(CC) ${CFLAGS} ${INC} -fpic -c ris.c
type2str.o: type2str.c ris.h ris_dbg.h   ${COMMON_DIR}/ris_struct.h ${COMMON_DIR}/rs.h ${INC_DIR}/dn_types.h ${PAL_DIR}/ipcsock.h
	$(CC) ${CFLAGS} ${INC} -fpic -c type2str.c

libipv6.o: libipv6.c libipv6.h
	$(CC) ${CFLAGS} ${INC} -fpic -c libipv6.c


# We need to build this ourselves, rather than link in libpal.so, because the
# code needs to be position-independent for a shared object
ipcsock.o: ${PAL_DIR}/ipcsock.c ${PAL_DIR}/ipcsock.h
	$(CC) ${CFLAGS} ${INC} -fpic -c ${PAL_DIR}/ipcsock.c

#---RIS (Radio Services API) Library
${RIS_SONAME}: ${OBJS}
	$(CC) $(CFLAGS) -shared -lc -Wl,-soname,${RIS_SONAME} -o ${RIS_SONAME} ${OBJS}

clean:
	rm -rf ${RIS_SONAME} ${OBJS}

install: all
ifdef INSTALL_PATH
	@test -d $(INSTALL_PATH)/lib || mkdir -p $(INSTALL_PATH)/lib
	cp -pd ${RIS_SONAME} ${INSTALL_PATH}/lib/
else
	echo ERROR: srv/radio/ris: Please define INSTALL_PATH before calling make install !
	exit 1
endif

check:
	cppcheck -q --force --inline-suppr ris.c  ${PAL_DIR}/ipcsock.c libipv6.c type2str.c
