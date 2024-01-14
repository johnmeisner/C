############################################
# RSU SDK Example Makefile
############################################
CC=qcc
LD=qcc

TARGET = -Vgcc_ntoarmv7le

SDK_ROOT=..
BUILD_DIR=${SDK_ROOT}/bin

CFLAGS = -DPLATFORM_QNX $(DEBUG) $(TARGET) -w9  -I. -Wno-pointer-sign

SDK_INCLUDES=-I${SDK_ROOT} -I.

SPAT_EX_SRC_DIR=spat_example
SDK_INCLUDES+=-I${SPAT_EX_SRC_DIR}
SPAT_EX_SRC=${SPAT_EX_SRC_DIR}/spat_example.c


.PHONY: all prep spat_example clean

all: prep spat_example 

prep:
	mkdir -p ${BUILD_DIR}

spat_example:
	${CC} $(CFLAGS) -Wall -W -g -O2 ${SDK_INCLUDES} ${SPAT_EX_SRC} -o ${BUILD_DIR}/spat_example

clean:
	rm -rf ${BUILD_DIR}
