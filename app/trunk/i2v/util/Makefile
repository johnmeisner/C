#-------------------------------------------------------------------------------
# File:
#
# Copyright (C) 2021 DENSO International America, Inc.
#
# Description:
#------------------------------------------------------------------------------

####################
# I2V offsets
####################
I2V_DIR =..

#########
# At ./C
#########
ifndef V2X_ROOT
   V2X_ROOT=../../../..
endif

include $(V2X_ROOT)/v2xcommon.mk

########################################
# Directories
########################################

COMMON_DIR      = $(V2X_ROOT)/srv/radio/common
INC_DIR         = $(V2X_ROOT)/inc
PAL_DIR         = $(V2X_ROOT)/pal
RIS_DIR         = $(V2X_ROOT)/srv/radio/ris
TPS_DIR         = $(V2X_ROOT)/srv/tps
I2V_COMMON_DIR  = $(I2V_DIR)/common
I2V_SHM_DIR     = $(I2V_DIR)/shm_inc
I2V_RSEMIB_DIR  = $(I2V_DIR)/ntcip-1218
CONF_SRV_DIR    = $(V2X_ROOT)/srv/conf_srv
LIBHDUTIL_DIR   = $(V2X_ROOT)/lib/hd-target-utils

CFLAGS += -Wall 
CFLAGS += -D_OPTIMIZED 
CFLAGS += -DASN1RT 
CFLAGS += -DGNU 
CFLAGS += -D_GNU_SOURCE 
CFLAGS += -DHAVE_STDINT_H 
CFLAGS += -DCPP

INCS = -I$(INC_DIR)
INCS += -I$(COMMON_DIR)
INCS += -I$(PAL_DIR)
INCS += -I$(RIS_DIR)
INCS += -I$(TPS_DIR)
INCS += -I$(I2V_SHM_DIR)
INCS += -I$(I2V_COMMON_DIR)
INCS += -I$(LIBHDUTIL_DIR)
INCS += -I$(ASN1_DIR)
INCS += -I$(J2735_UPER_DIR)
INCS += -I$(CONF_SRV_DIR)

LIBS += -L$(PAL_DIR)
LIBS += -L$(RIS_DIR)
LIBS += -L$(TPS_DIR)

LIBS += -lpal
LIBS += -lm
LIBS += -lpthread
LIBS += -lris
LIBS += -lrt
LIBS += -ltpsapi

SO_DEPS += $(RIS_DIR)/libris.so
SO_DEPS += $(TPS_DIR)/libtpsapi.so

BINS = i2vmonitor quickspat
TESTS = test_i2v_util

.PHONY: all tests clean

all: $(BINS)

tests: $(TESTS)

quickspat: quickspat.c i2v_util.o
	$(CC) $(CFLAGS) $(INCS) -o $@ $^ $(LDFLAGS) $(LIBS)

i2vmonitor: i2vmonitor.c i2v_util.o
	$(CC) $(CFLAGS) $(INCS) -o $@ $^ $(LDFLAGS) $(LIBS)

i2v_util.o: i2v_util.c
	$(CC) $(CFLAGS) $(INCS) -c $(I2V_DIR)/util/i2v_util.c

spatcfg_util.o: spatcfg_util.c
	$(CC) $(CFLAGS) $(INCS) -c $^

test_i2v_util: test_i2v_util.c i2v_util.o
	$(CC) $(CFLAGS) $(INCS) -o $@ $^ $(LDFLAGS) $(LIBS)

coverage:
	./$(CUNIT_SOURCE)
	gcov *.c

clean:
	rm -f *.o $(BINS) $(TESTS)

install: all
ifdef INSTALL_PATH
	@mkdir -p $(INSTALL_PATH)/bin
	cp $(BINS) $(INSTALL_PATH)/bin
else
	@echo "ERROR: i2v-Makefile: Please define INSTALL_PATH before calling make install !"
	exit 1
endif

