#-------------------------------------------------------------------------------
# File: Source/C/app/utils/densonor/Makefile
#
# Copyright (C) 2021 DENSO International America, Inc.
#
# Description: densonor Makefile
#------------------------------------------------------------------------------

V2X_ROOT=../..
include $(V2X_ROOT)/v2xcommon.mk

INC_PATH=$(V2X_ROOT)/inc
PAL_PATH=$(V2X_ROOT)/pal
RSUHEALTH_PATH=$(V2X_ROOT)/utils/rsuhealth
I2V_UTIL_PATH=$(V2X_ROOT)/app/trunk/i2v/util
I2V_COMMON_PATH=$(V2X_ROOT)/app/trunk/i2v/common
I2V_SHM_PATH=$(V2X_ROOT)/app/trunk/i2v/shm_inc
RIS_PATH=$(V2X_ROOT)/srv/radio/ris
COMMON_PATH=$(V2X_ROOT)/srv/radio/common
TPS_PATH=$(V2X_ROOT)/srv/tps
CONF_SRV_PATH=$(V2X_ROOT)/srv/conf_srv

CFLAGS += -Wall

INCS += -I.
INCS += -I$(INC_PATH)
INCS += -I$(PAL_PATH)
INCS += -I$(RSUHEALTH_PATH)
INCS += -I$(I2V_UTIL_PATH)
INCS += -I$(I2V_COMMON_PATH)
INCS += -I$(I2V_SHM_PATH)
INCS += -I$(RIS_PATH)
INCS += -I$(COMMON_PATH)
INCS += -I$(TPS_PATH)
INCS += -I$(CONF_SRV_PATH)
INCS += -I$(J2735_UPER_DIR)
INCS += -I$(ASN1_DIR)

CFLAGS += $(INCS)

LD_PATHS = -L$(PAL_PATH)

LDFLAGS += -Wl,--no-undefined  # Resolve dependencies on other libraries
LDFLAGS += $(LD_PATHS)

# Object modules
NOROBJS  = densonor.o i2v_util.o

OBJS = $(NOROBJS)

# Binaries
BINS = densonor

# Libraries
LIBS += -lpthread -lrt
LIBS += -L$(PAL_PATH)
LIBS += -lpal

SO_DEPS += $(PAL_PATH)/libpal.so

.PHONY: all clean install check

all: $(BINS)

densonor: $(NOROBJS) $(SO_DEPS)
	@echo "INSTALL_PATH=$(INSTALL_PATH)"
	@echo "LIB_PATH=$(LIB_PATH)"
	$(CC) $(CFLAGS) -o densonor $(NOROBJS) $(LIBS)

densonor.o: densonor.c
	$(CC) $(CFLAGS) $(INCS) $(LIBS) -c densonor.c

i2v_util.o: $(I2V_UTIL_PATH)/i2v_util.c
	$(CC) $(CFLAGS) $(INCS) -c $(I2V_UTIL_PATH)/i2v_util.c

$(PAL_PATH)/libpal.so:
	make -C $(PAL_PATH) libpal.so

coverage:
	./$(CUNIT_SOURCE)
	gcov *.c

clean:
	rm -rf $(BINS) *.o

install: all

ifdef INSTALL_PATH
	@echo "densonor: make install... "
	@test -d $(INSTALL_PATH) || (mkdir $(INSTALL_PATH); echo "*---Creating Installation Directory ($(INSTALL_PATH)).")
	@test -d $(INSTALL_PATH)/local/bin || (mkdir -p $(INSTALL_PATH)/local/bin; echo "*---Creating Installation Directory ($(INSTALL_PATH)/local/bin).")
	cp $(BINS) $(INSTALL_PATH)/local/bin

else
	@echo "ERROR: densonor: Please Define INSTALL_PATH Before Calling make install"
	exit 1
endif

check:
	cppcheck -q --force --inline-suppr densonor.c
