#------------------------------------------------------------------------------
# File:    Source/pal/Makefile
#
# Copyright (C) 2019 DENSO International America, Inc.
#
# Description: V2X Framework Makefile for PAL api
#------------------------------------------------------------------------------


V2X_ROOT=..

include ${V2X_ROOT}/v2xcommon.mk

COMMON_DIR = ../srv/radio/common
AEROLINK_INCLUDE_DIR = ../srv/radio/common/Aerolink/include

#---CFLAG options from TPS:
#CFLAGS += -DDEBUG
CFLAGS += -DHAVE_CONFIG_H
CFLAGS += -I../pal -I../inc -I$(COMMON_DIR) -I$(AEROLINK_INCLUDE_DIR)
CFLAGS += $(LDFLAGS)    # For OpenEmbedded GNU_HASH in binary


#--- PAL Shared Library
LIBPAL_FILES  = ipcsock.c wsu_sharedmem.c  wsu_util.c
LIBPAL_INCS   = ipcsock.h wsu_sharedmem.h  wsu_util.h
LIBPAL_SONAME = libpal.so
LIBPAL_OBJS = $(LIBPAL_FILES:.c=.o)

ALL_LIBRARIES = ${LIBPAL_SONAME}

TEST_UTILS =

.PHONY: all everything clean install check

all:  $(LIBPAL_SONAME)    # For production
# all:  $(ALL_LIBRARIES)  # For Dev

everything: $(LIBPAL_SONAME) $(TEST_UTILS)

clean:
	rm -rf ${ALL_LIBRARIES} *.o *.a $(TEST_UTILS)
	
#--- PAL Shared Library
#    NOTE: The "cc || exit 1" is to force the for loop to exit on the first
#          error, passing the error back to the Makefile, aborting compilation.
#          Otherwise the error is lost and if the last cc succeeds, make will
#          continue along not knowing there were problems.
${LIBPAL_SONAME}: ${LIBPAL_FILES} ${LIBPAL_INCS}
	for srcfile in ${LIBPAL_FILES}; do \
		$(CC) $(CFLAGS) ${INC} -fpic -c $$srcfile || exit 1; \
	done
	$(CC) $(CFLAGS) -shared -Wall -lc  -Wl,-soname,${LIBPAL_SONAME} -o ${LIBPAL_SONAME} ${LIBPAL_OBJS} -lm -lrt -pthread

# Unused but in here in case we need to debug libpal ever
libpal.a: ${LIBPAL_FILES} ${LIBPAL_INCS}
	for srcfile in ${LIBPAL_FILES}; do \
		$(CC) -pthread $(CFLAGS) ${INC} -fpic -c $$srcfile || exit 1; \
	done
	$(AR) rcs libpal.a ${LIBPAL_OBJS}

install: all
ifdef INSTALL_PATH
	@test -d $(INSTALL_PATH)/lib || mkdir -p $(INSTALL_PATH)/lib
	cp -pd ${LIBPAL_SONAME} ${INSTALL_PATH}/lib/
else
	echo ERROR: pal: Please define INSTALL_PATH before calling make install !
	exit 1
endif

check:
	cppcheck -q --force --inline-suppr ${LIBPAL_FILES}
