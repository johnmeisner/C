#
# Makefile for tlsTest
#

ifndef V2X_ROOT
   V2X_ROOT=../../../../../..
endif

#---Include Paths

include ${V2X_ROOT}/v2xcommon.mk

# PAL include files
export PAL_INC_PATH=$(V2X_ROOT)/C/pal

# INC include files
export INC_INC_PATH=$(V2X_ROOT)/C/inc

# AEROLINK include files
export AEROLINK_INC_PATH=../../include

# Binary executables directory
export BIN_PATH=.

# Library paths
#export LIBPATH=$(V2X_ROOT)/C/pal
export LIBPATH=.
export SXF1800_DIR=$(V2X_ROOT)/../Drivers/HSM/SXF1800_REL_EAR6.0.0/install
export SSL_DIR=../openssl-1.1.1c/
export TLSLIBPATH=../
export SSL_INCLUDE=../openssl-1.1.1c/include/openssl

CFLAGS += -DDEBUG -DHAVE_CONFIG_H -DWSU_LITTLE_ENDIAN -I.
CFLAGS += -I${PAL_INC_PATH}
CFLAGS += -I${INC_INC_PATH}
CFLAGS += -I${AEROLINK_INC_PATH}
CFLAGS += -I${SSL_INCLUDE}

BINS = ${BIN_PATH}/tlsTest

all:	$(BINS)

# tlsTest
OBJS = tlsTest.o # udpTest.o

tlsTest.o: tlsTest.c
	$(CC) $(CFLAGS) -c tlsTest.c

$(BINS): $(OBJS)
	$(CC) $(CFLAGS) -L$(LIBPATH) -L$(SXF1800_DIR) -L$(SSL_DIR) -L$(TLSLIBPATH) -Wall -lcrypto -lpal -lm -o ${BINS} $(OBJS) -laerolinkPKI

clean:
	rm -f *.o $(BINS) *.tmp

#---Install into INSTALL_PATH.
install: all
ifdef INSTALL_PATH
	cp ${BINS} ${INSTALL_PATH}/v2x
else
	echo "ERROR: srv/Radio/ns: Please define INSTALL_PATH before calling make install !"
	exit 1
endif

