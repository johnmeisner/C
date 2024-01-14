#-------------------------------------------------------------------------------
# File: Source/C/app/utils/dnnetcfg/Makefile
#
# Copyright (C) 2021 DENSO International America, Inc.
#
# Description: dnnetcfg Makefile
#------------------------------------------------------------------------------

V2X_ROOT=../..
include $(V2X_ROOT)/v2xcommon.mk

I2V_UTIL_PATH=$(V2X_ROOT)/app/trunk/i2v/util
DN_TYPES_PATH=$(V2X_ROOT)/inc
I2V_COMMON_PATH=$(V2X_ROOT)/app/trunk/i2v/common
PAL_PATH=$(V2X_ROOT)/pal
I2V_SHM_PATH=$(V2X_ROOT)/app/trunk/i2v/shm_inc
RIS_PATH=$(V2X_ROOT)/srv/radio/ris
COMMON_PATH=$(V2X_ROOT)/srv/radio/common
TPS_PATH=$(V2X_ROOT)/srv/tps
CONF_SRV_PATH=$(V2X_ROOT)/srv/conf_srv
AEROLINK_INC_PATH=$(V2X_ROOT)/srv/radio/common/Aerolink/include
RSU_HEALTH_PATH=$(V2X_ROOT)/utils/rsuhealth

INCS = -I$(I2V_UTIL_PATH)
INCS += -I$(DN_TYPES_PATH)
INCS += -I$(I2V_COMMON_PATH)
INCS += -I$(PAL_PATH)
INCS += -I$(I2V_SHM_PATH)
INCS += -I$(RIS_PATH)
INCS += -I$(COMMON_PATH)
INCS += -I$(TPS_PATH)
INCS += -I$(J2735_UPER_DIR)
INCS += -I$(ASN1_DIR)
INCS += -I$(CONF_SRV_PATH)
INCS += -I$(AEROLINK_INC_PATH)
INCS += -I$(RSU_HEALTH_PATH)

CFLAGS += -Wall

# Object modules
OBJS  = dnnetcfg.o i2v_util.o

# Binaries
BINS = dnnetcfg

LIBS += -pthread -lrt
LIBS += -L$(PAL_PATH)
LIBS += -lpal

SO_DEPS += $(PAL_PATH)/libpal.so

.PHONY: all clean install check

all: $(BINS)

dnnetcfg: $(OBJS) $(SO_DEPS)
	@echo "INSTALL_PATH=$(INSTALL_PATH)"
	@echo "LIB_PATH=$(LIB_PATH)"
	$(CC) $(CFLAGS) -o dnnetcfg $(OBJS) $(LIBS)

dnnetcfg.o: dnnetcfg.c
	$(CC) $(CFLAGS) $(INCS) -c dnnetcfg.c

i2v_util.o: $(I2V_UTIL_PATH)/i2v_util.c
	$(CC) $(CFLAGS) $(INCS) -c $(I2V_UTIL_PATH)/i2v_util.c

$(PAL_PATH)/libpal.so:
	make -C $(PAL_PATH) libpal.so

clean:
	rm -rf $(BINS) *.o

install: all

ifdef INSTALL_PATH
	@echo "dnnetcfg: make install... "
	@test -d $(INSTALL_PATH) || (mkdir $(INSTALL_PATH); echo "*---Creating Installation Directory ($(INSTALL_PATH)).")
	@test -d $(INSTALL_PATH)/local/bin || (mkdir -p $(INSTALL_PATH)/local/bin; echo "*---Creating Installation Directory ($(INSTALL_PATH)/local/bin).")
	cp $(BINS) $(INSTALL_PATH)/local/bin

else
	@echo "ERROR: dnnetcfg: Please Define INSTALL_PATH Before Calling make install"
	exit 1
endif

check:
	cppcheck -q --force --inline-suppr dnnetcfg.c
