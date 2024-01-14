#-------------------------------------------------------------------------------
# File: Source/C/app/utils/pshbtnmonitor/Makefile
#
# Copyright (C) 2021 DENSO International America, Inc.
#
# Description: pshbtnmonitor Makefile
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

CFLAGS += -Wall

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

CFLAGS += $(INC_PATHS)  # INC_PATHS part of CFLAGS so that implicit make rules will work.

LD_PATHS = -L$(PAL_PATH)

LDFLAGS += -Wl,--no-undefined  # Resolve dependencies on other libraries
LDFLAGS += $(LD_PATHS)

# Object modules
OBJS  = pshbtnmonitor.o i2v_util.o

# Binaries
BINS = pshbtnmonitor

# Libraries
LIBS += -lpthread -lrt
LIBS += -L$(PAL_PATH)
LIBS += -lpal

SO_DEPS += $(PAL_PATH)/libpal.so

#For unit test builds.
I2V_STUB_DIR  = $(V2X_ROOT)/app/trunk/i2v/stubs
CUNIT_INCS    = -I$(I2V_STUB_DIR)
CUNIT_TEST_FILE = unit_test_pshbtnmonitor.c
#Add source under test.
CUNIT_SOURCE  = pshbtnmonitor.c
CUNIT_SOURCE  += $(V2X_ROOT)/app/trunk/i2v/util/i2v_util.c $(PAL_PATH)/wsu_sharedmem.c $(V2X_ROOT)/app/trunk/i2v/stubs/stubs.c
CUNIT_BINS    = ./pshbtnmonitor.o   ./i2v_util.o ./wsu_sharedmem.o ./stubs.o
#Call this to run test.
CUNIT_EXE     = unit_test_pshbtnmonitor
#Extra test output to clean.
CUNIT_OUTPUT  = *.gcov *.gcda *.gcno

.PHONY: all clean install

all: $(BINS)

pshbtnmonitor: $(OBJS) $(SO_DEPS)
	@echo "INSTALL_PATH=$(INSTALL_PATH)"
	@echo "LIB_PATH=$(LIB_PATH)"
	$(CC) $(CFLAGS) -o pshbtnmonitor $(OBJS) $(LIBS)

pshbtnmonitor.o: pshbtnmonitor.c
	$(CC) $(CFLAGS) $(INCS) $(LIBS) -c pshbtnmonitor.c

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
	@echo "pshbtnmonitor: make install... "
	@test -d $(INSTALL_PATH) || (mkdir $(INSTALL_PATH); echo "*---Creating Installation Directory ($(INSTALL_PATH)).")
	@test -d $(INSTALL_PATH)/local/bin || (mkdir -p $(INSTALL_PATH)/local/bin; echo "*---Creating Installation Directory ($(INSTALL_PATH)/local/bin).")
	cp $(BINS) $(INSTALL_PATH)/local/bin

else
	@echo "ERROR: pshbtnmonitor: Please Define INSTALL_PATH Before Calling make install"
	exit 1
endif

unit:
	rm -rf $(BINS) *.o
	rm -f $(CUNIT_EXE) $(CUNIT_OUTPUT) *.tmp *.gch
	gcc -c -fPIC $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INC_PATHS) $(CUNIT_SOURCE)
	gcc          $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INC_PATHS) $(CUNIT_TEST_FILE) $(CUNIT_BINS) $(CUNIT_LIBS) -o $(CUNIT_EXE)

check:
	cppcheck -q --force --inline-suppr pshbtnmonitor.c
