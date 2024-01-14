#-------------------------------------------------------------------------------
# File: Source/C/fac/vod/Makefile
#
# Copyright (C) 2019 DENSO International America, Inc.
#
# Description: Makefile for vod
#------------------------------------------------------------------------------

ifndef V2X_ROOT
   V2X_ROOT=../..
endif

include ${V2X_ROOT}/v2xcommon.mk

#---Directories
INC_DIR=../../inc
PAL_DIR=../../pal
COMMON_DIR=../../srv/radio/common
AEROLINK_DIR=../../srv/radio/common/Aerolink/include
RIS_DIR=../../srv/radio/ris

ALL_EXECUTABLES=vod testvod
ALL_LIBS=libvodapi.so

CFLAGS+=-Wall
CFLAGS+=$(LDFLAGS)    # For OpenEmbedded GNU_HASH in binary

INC=-I. -I${INC_DIR} -I${PAL_DIR}
VODINC=${INC}  -I${COMMON_DIR} -I${AEROLINK_DIR} -I${RIS_DIR} #-I${SR_DIR} 
VODLIB=-L${PAL_DIR}  -lpal  -lm -lpthread -lrt
VODINCDEPENDENCIES=vod.c vod.h vod_api.h cfg_vod.h ${PAL_DIR}/ipcsock.h ${PAL_DIR}/wsu_sharedmem.h ${INC_DIR}/v2x_common.h  ${COMMON_DIR}/rs.h   ${RIS_DIR}/type2str.h #${SR_DIR}/shm_sr.h
VODLIBDEPENDENCIES=${PAL_DIR}/libpal.so 

.PHONY: all clean install

#---Default Rule:
all: ${ALL_EXECUTABLES} ${ALL_LIBS}

vod_api.o: vod_api.c vod_api.h ${INC_DIR}/dn_types.h ${INC_DIR}/v2x_common.h ${PAL_DIR}/wsu_shm_inc.h
	$(CC) ${CFLAGS} ${INC} -fpic -c vod_api.c -o $@

type2str.o: ${RIS_DIR}/type2str.c ${RIS_DIR}/type2str.h  ${COMMON_DIR}/ris_struct.h ${COMMON_DIR}/rs.h ${INC_DIR}/dn_types.h
	$(CC) ${CFLAGS} -I${INC_DIR} -I${RIS_DIR} -I${COMMON_DIR} -c ${RIS_DIR}/type2str.c

libvodapi.so: vod_api.o
	$(CC) $(CFLAGS) -fpic -shared -Wall -lc -Wl,-soname,libvodapi.so -o libvodapi.so vod_api.o

vod: ${VODINCDEPENDENCIES} vod_api.o type2str.o ${VODLIBDEPENDENCIES}
	$(CC) ${CFLAGS} ${VODINC} -o $@ vod.c vod_api.o type2str.o ${VODLIB}

testvod: testvod.c vod_api.h libvodapi.so ${PAL_DIR}/wsu_sharedmem.h ${PAL_DIR}/libpal.so ${INC_DIR}/v2x_common.h
	$(CC) $(CFLAGS) ${INC} -L. -L${PAL_DIR} testvod.c -o $@ -lvodapi -lm -lpal -lpthread -lrt

${PAL_DIR}/libpal.so:
	make -C ${PAL_DIR} libpal.so

clean:
	rm -f ${ALL_EXECUTABLES} $(ALL_LIBS) *.o

install: all
ifdef INSTALL_PATH
	@test -d $(INSTALL_PATH)/bin || mkdir -p $(INSTALL_PATH)/bin
	cp $(ALL_EXECUTABLES) $(INSTALL_PATH)/bin
	@test -d $(INSTALL_PATH)/lib || mkdir -p $(INSTALL_PATH)/lib
	cp $(ALL_LIBS) $(INSTALL_PATH)/lib
else
	@echo "ERROR: fac/vod/Makefile: Please define INSTALL_PATH before calling make install !"
	exit 1
endif

