#-------------------------------------------------------------------------------
# File: Source/C/app/utils/rsuhealth/Makefile
#
# Copyright (C) 2021 DENSO International America, Inc.
#
# Description: rsuhealth Makefile
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
NTCIP_1218_PATH=$(V2X_ROOT)/app/trunk/i2v/ntcip-1218

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
INCS += -I$(NTCIP_1218_PATH)

CFLAGS += -Wall

# Object modules
OBJS  = rsuhealth.o rsudiagnostic.o i2v_util.o

# Binaries
BINS = rsuhealth

# Libraries
LIBS += -lpthread -lrt
LIBS += -L$(PAL_PATH)
LIBS += -lpal

SO_DEPS += $(PAL_PATH)/libpal.so

#For unit test builds.
I2V_STUB_DIR  = $(V2X_ROOT)/app/trunk/i2v/stubs
CUNIT_INCS    = -I$(I2V_STUB_DIR)
CUNIT_TEST_FILE = unit_test_rsuhealth.c
#Add source under test.
CUNIT_SOURCE  = rsuhealth.c rsudiagnostic.c
CUNIT_SOURCE  += $(V2X_ROOT)/app/trunk/i2v/util/i2v_util.c $(PAL_PATH)/wsu_sharedmem.c $(PAL_PATH)/ipcsock.c $(V2X_ROOT)/app/trunk/i2v/stubs/stubs.c
CUNIT_BINS    = ./rsuhealth.o  ./rsudiagnostic.o ./i2v_util.o ./wsu_sharedmem.o ./ipcsock.o ./stubs.o
#Call this to run test.
CUNIT_EXE     = unit_test_rsuhealth
#Extra test output to clean.
CUNIT_OUTPUT  = *.gcov *.gcda *.gcno

.PHONY: all clean install

all: $(BINS)

rsuhealth: $(OBJS) $(SO_DEPS)
	@echo "INSTALL_PATH=$(INSTALL_PATH)"
	@echo "LIB_PATH=$(LIB_PATH)"
	$(CC) $(CFLAGS) -o rsuhealth $(OBJS) $(LIBS)

rsuhealth.o: rsuhealth.c
	$(CC) $(CFLAGS) $(INCS) $(LIBS) -c rsuhealth.c

rsudiagnostic.o: rsudiagnostic.c
	$(CC) $(CFLAGS) $(INCS) $(LIBS) -c rsudiagnostic.c

i2v_util.o: $(I2V_UTIL_PATH)/i2v_util.c
	$(CC) $(CFLAGS) $(INCS) -c $(I2V_UTIL_PATH)/i2v_util.c

$(PAL_PATH)/libpal.so:
	make -C $(PAL_PATH) libpal.so

coverage:
	./$(CUNIT_SOURCE)
	gcov *.c

clean:
	rm -rf $(BINS) *.o
	rm -f $(CUNIT_EXE) $(CUNIT_OUTPUT) *.tmp *.gch

install: all
ifdef INSTALL_PATH
	@echo "rsuhealth: make install... "
	@test -d $(INSTALL_PATH) || (mkdir $(INSTALL_PATH); echo "*---Creating Installation Directory ($(INSTALL_PATH)).")
	@test -d $(INSTALL_PATH)/local/bin || (mkdir -p $(INSTALL_PATH)/local/bin; echo "*---Creating Installation Directory ($(INSTALL_PATH)/local/bin).")
	cp $(BINS) $(INSTALL_PATH)/local/bin

else
	@echo "ERROR: rsuhealth: Please Define INSTALL_PATH Before Calling make install"
	exit 1
endif

unit:
	rm -rf $(BINS) *.o
	rm -f $(CUNIT_EXE) $(CUNIT_OUTPUT) *.tmp *.gch
	gcc -c -fPIC $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INCS) $(CUNIT_SOURCE)
	gcc          $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INCS) $(CUNIT_TEST_FILE) $(CUNIT_BINS) $(CUNIT_LIBS) -o $(CUNIT_EXE)

check:
	cppcheck -q --force --inline-suppr rsuhealth.c rsudiagnostic.c
