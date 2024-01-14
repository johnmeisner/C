#-------------------------------------------------------------------------------
# File: Source/C/app/utils/lpwrmonitor/Makefile
#
# Copyright (C) 2021 DENSO International America, Inc.
#
# Description: lpwrmonitor Makefile
#------------------------------------------------------------------------------

V2X_ROOT=../..
include $(V2X_ROOT)/v2xcommon.mk

#-------------------------------------------------------------------------------
#---Paths to project and non-standard system directories:
#-------------------------------------------------------------------------------

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
NTCIP-1218_PATH=$(V2X_ROOT)/app/trunk/i2v/ntcip-1218



INC_PATHS = -I.
INC_PATHS += -I$(I2V_UTIL_PATH)
INC_PATHS += -I$(DN_TYPES_PATH)
INC_PATHS += -I$(I2V_COMMON_PATH)
INC_PATHS += -I$(PAL_PATH)
INC_PATHS += -I$(I2V_SHM_PATH)
INC_PATHS += -I$(RIS_PATH)
INC_PATHS += -I$(COMMON_PATH)
INC_PATHS += -I$(TPS_PATH)
INC_PATHS += -I$(J2735_UPER_DIR)
INC_PATHS += -I$(ASN1_DIR)
INC_PATHS += -I$(CONF_SRV_PATH)
INC_PATHS += -I$(AEROLINK_INC_PATH)
INC_PATHS += -I$(RSU_HEALTH_PATH)
INC_PATHS += -I$(NTCIP-1218_PATH)

CFLAGS += $(INC_PATHS) -Wall # INC_PATHS part of CFLAGS so that implicit make rules will work.

LD_PATHS = -L$(PAL_PATH)

LDFLAGS += -Wl,--no-undefined  # Resolve dependencies on other libraries
LDFLAGS += $(LD_PATHS)

# Object modules
OBJS  = lpwrmonitor.o
OBJS += i2v_util.o
# Binaries
BINS = lpwrmonitor

# Libraries
LIBS += -lpthread -lrt
LIBS += -L$(PAL_PATH)
LIBS += -lpal

SO_DEPS += $(PAL_PATH)/libpal.so

#For unit test builds.
I2V_STUB_DIR  = $(V2X_ROOT)/app/trunk/i2v/stubs
CUNIT_INCS    = -I$(I2V_STUB_DIR)
CUNIT_TEST_FILE = unit_test_lpwrmonitor.c
#Add source under test.
CUNIT_SOURCE  = lpwrmonitor.c
CUNIT_SOURCE  += $(V2X_ROOT)/app/trunk/i2v/util/i2v_util.c $(PAL_PATH)/wsu_sharedmem.c $(PAL_PATH)/ipcsock.c $(V2X_ROOT)/app/trunk/i2v/stubs/stubs.c
CUNIT_BINS    = ./lpwrmonitor.o   ./i2v_util.o ./wsu_sharedmem.o ./ipcsock.o ./stubs.o
#Call this to run test.
CUNIT_EXE     = unit_test_lpwrmonitor
#Extra test output to clean.
CUNIT_OUTPUT  = *.gcov *.gcda *.gcno

.PHONY: all clean install check

all: $(BINS)

lpwrmonitor: $(OBJS) $(SO_DEPS)
	@echo "INSTALL_PATH=$(INSTALL_PATH)"
	@echo "LIB_PATH=$(LIB_PATH)"
	$(CC) $(CFLAGS) -o lpwrmonitor $(OBJS) $(LIBS)

lpwrmonitor.o: lpwrmonitor.c
	$(CC) $(CFLAGS) $(INCS) -c lpwrmonitor.c

i2v_util.o: $(I2V_UTIL_PATH)/i2v_util.c
	$(CC) $(CFLAGS) $(INCS) -c $(I2V_UTIL_PATH)/i2v_util.c

# Library Dependecies
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
	@echo "lpwrmonitor: make install... "
	@test -d $(INSTALL_PATH) || (mkdir $(INSTALL_PATH); echo "*---Creating Installation Directory ($(INSTALL_PATH)).")
	@test -d $(INSTALL_PATH)/local/bin || (mkdir -p $(INSTALL_PATH)/local/bin; echo "*---Creating Installation Directory ($(INSTALL_PATH)/local/bin).")
	cp $(BINS) $(INSTALL_PATH)/local/bin

else
	@echo "ERROR: lpwrmonitor: Please Define INSTALL_PATH Before Calling make install"
	exit 1
endif

unit:
	rm -rf $(BINS) *.o
	rm -f $(CUNIT_EXE) $(CUNIT_OUTPUT) *.tmp *.gch
	gcc -c -fPIC $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INC_PATHS) $(CUNIT_SOURCE)
	gcc          $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INC_PATHS) $(CUNIT_TEST_FILE) $(CUNIT_BINS) $(CUNIT_LIBS) -o $(CUNIT_EXE)

check:
	cppcheck -q --force --inline-suppr lpwrmonitor.c

