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

#
# IPv6 OCTETS coming from SNMP CLIENT
# SNMPB client different than linux/QNX local host for some reason
#
# Kept for local level compile
#PLATFORM_FLAGS += -DPACKED_OCTETS

ALSMI_API_DIR   = $(V2X_ROOT)/srv/radio/alsmi_api
COMMON_DIR      = $(V2X_ROOT)/srv/radio/common
INC_DIR         = $(V2X_ROOT)/inc
PAL_DIR         = $(V2X_ROOT)/pal
RIS_DIR         = $(V2X_ROOT)/srv/radio/ris
TPS_DIR         = $(V2X_ROOT)/srv/tps
I2V_COMMON_DIR  = $(I2V_DIR)/common
I2V_SHM_DIR     = $(I2V_DIR)/shm_inc
I2V_RSEMIB_DIR  = $(I2V_DIR)/ntcip-1218
CONF_SRV_DIR    = $(V2X_ROOT)/srv/conf_srv
AEROLINK_DIR    = $(COMMON_DIR)/Aerolink/include
RSUHEALTH_DIR   = $(V2X_ROOT)/utils/rsuhealth

INCS += -I$(PAL_DIR)
INCS += -I$(INC_DIR)
INCS += -I$(I2V_COMMON_DIR)
INCS += -I$(I2V_SHM_DIR)
INCS += -I$(RIS_DIR)
INCS += -I$(COMMON_DIR)
INCS += -I$(TPS_DIR)
INCS += -I$(J2735_UPER_DIR)
INCS += -I$(ASN1_DIR)
INCS += -I$(AEROLINK_DIR)
INCS += -I$(CONF_SRV_DIR)
INCS += -I$(RSUHEALTH_DIR)

LIBS += -L$(PAL_DIR)
LIBS += -lpal
LIBS += -lrt
LIBS += -lpthread
LIBS += -lm

SO_DEPS += $(PAL_DIR)/libpal.so

# if adding different MIBs copy the format below
RSU_MIB      = rsumib.so
#BSP loads MIB references needed but no harm noting here this is the one intended.
#RSU_MIB_FILE = NTCIP-1218-MIB.txt
RSUMIB_SRC   = rsutable.c
RSUMIB_SRC  += ntcip-1218.c
RSUMIB_SRC  += rsuRadioTable.c
RSUMIB_SRC  += rsuGnssStatus.c
RSUMIB_SRC  += rsuGnssOutput.c
RSUMIB_SRC  += rsuWraConfig.c
RSUMIB_SRC  += rsuSysDescription.c
RSUMIB_SRC  += rsuSysSettings.c
RSUMIB_SRC  += rsuSysStatus.c
RSUMIB_SRC  += rsuAsync.c
RSUMIB_SRC  += rsuSecurity.c
RSUMIB_SRC  += rsuAntenna.c
RSUMIB_SRC  += rsuAppConfig.c
RSUMIB_SRC  += rsuService.c
RSUMIB_SRC  += rsuMessageStats.c
RSUMIB_SRC  += rsuMsgRepeat.c
RSUMIB_SRC  += rsuIFM.c
RSUMIB_SRC  += rsuReceivedMsg.c
RSUMIB_SRC  += rsuInterfaceLog.c
RSUMIB_SRC  += rsuWsaConfig.c
RSUMIB_SRC  += rsuSystemStats.c
RSUMIB_SRC  += rsuXmitMsgFwding.c
RSUMIB_SRC  += $(I2V_DIR)/util/i2v_util.c
RSUMIB_SRC  += $(PAL_DIR)/wsu_sharedmem.c 

#For unit test builds.
I2V_STUB_DIR  = $(I2V_DIR)/stubs
CUNIT_FLAGS  += -DUNIT_NET_SNMP
CUNIT_FLAGS  += -DUNIT_PTHREAD
CUNIT_INCS    = -I$(I2V_STUB_DIR)
CUNIT_LIBS   += -lm
#Add source under test.
CUNIT_TEST_FILE = unit_test_rsemib.c
CUNIT_SOURCE  = $(RSUMIB_SRC)
CUNIT_SOURCE  += $(I2V_STUB_DIR)/stubs.c
#CUNIT_SOURCE += $(PAL_DIR)/wsu_sharedmem.c
CUNIT_BINS  = ./stubs.o
CUNIT_BINS += ./rsutable.o 
CUNIT_BINS += ./ntcip-1218.o  
CUNIT_BINS += ./rsuRadioTable.o 
CUNIT_BINS += ./rsuGnssStatus.o 
CUNIT_BINS += ./rsuGnssOutput.o
CUNIT_BINS += ./rsuWraConfig.o
CUNIT_BINS += ./rsuSysDescription.o
CUNIT_BINS += ./rsuSysSettings.o
CUNIT_BINS += ./rsuSysStatus.o
CUNIT_BINS += ./rsuAsync.o
CUNIT_BINS += ./rsuSecurity.o
CUNIT_BINS += ./rsuAntenna.o
CUNIT_BINS += ./rsuAppConfig.o
CUNIT_BINS += ./rsuService.o
CUNIT_BINS += ./rsuMessageStats.o
CUNIT_BINS += ./rsuMsgRepeat.o
CUNIT_BINS += ./rsuIFM.o
CUNIT_BINS += ./rsuReceivedMsg.o
CUNIT_BINS += ./rsuInterfaceLog.o
CUNIT_BINS += ./rsuWsaConfig.o
CUNIT_BINS += ./rsuSystemStats.o
CUNIT_BINS += ./rsuXmitMsgFwding.o
CUNIT_BINS += ./wsu_sharedmem.o
CUNIT_BINS += ./i2v_util.o

#Call this to run test.
CUNIT_EXE     = unit_test_rsemib
#Extra test output to clean.
CUNIT_OUTPUT  = *.gcov *.gcda *.gcno 0.txt sdm.conf

.PHONY: all rsu_mib install clean unit lint cppcheck

#add any new mib targets here
all: rsu_mib 

# strip performed separate to this make
rsu_mib:
	rm -f ${RSU_MIB}
	${CC} -fPIC -shared -g -O2 ${PLATFORM_FLAGS} ${CFLAGS} ${INCS} ${PLATFORM_LIBS} ${LIBS} ${SO_DEPS} -o ${RSU_MIB} ${RSUMIB_SRC}

i2v_util.o: $(I2V_DIR)/util/i2v_util.c
	$(CC) $(CFLAGS) $(INCS) -c $(I2V_DIR)/util/i2v_util.c

# separate targets not included in all
install:
	mkdir -p ${INSTALL_PATH}/lib
	cp ${RSU_MIB} ${INSTALL_PATH}/lib

coverage:
	./$(CUNIT_SOURCE)
	gcov *.c

clean:
	rm -f *.o $(BINS) *.tmp *.gch
	rm -rf ./install
	rm -f $(CUNIT_EXE) $(CUNIT_OUTPUT)

unit:
	gcc -c -fPIC $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INCS) $(CUNIT_SOURCE)
	gcc          $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INCS) $(CUNIT_TEST_FILE) $(CUNIT_BINS) $(CUNIT_LIBS) -o $(CUNIT_EXE)

check:
	cppcheck -q --force --inline-suppr $(CUNIT_SOURCE) $(CUNIT_TEST_FILE)
