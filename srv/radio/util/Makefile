#-------------------------------------------------------------------------------
# File: Source/C/srv/radio/util/Makefile
#
# Copyright (C) 2019 DENSO International America, Inc.
#
# Description: Makefile for Radio Services Test Programs
#------------------------------------------------------------------------------


ifndef V2X_ROOT
   V2X_ROOT=../../..
endif

include ${V2X_ROOT}/v2xcommon.mk

#---Directories
INC_DIR=$(V2X_ROOT)/inc
PAL_DIR=$(V2X_ROOT)/pal

COMMON_DIR=../common
NS_DIR=../ns
RIS_DIR=../ris
AEROLINK_DIR=../common/Aerolink/include
ALSMI_API_DIR=../alsmi_api
TPS_DIR=../../tps
I2V_UTIL_PATH=$(V2X_ROOT)/app/trunk/i2v/util
DN_TYPES_PATH=$(V2X_ROOT)/inc
I2V_COMMON_PATH=$(V2X_ROOT)/app/trunk/i2v/common
PAL_PATH=$(V2X_ROOT)/pal
I2V_SHM_PATH=$(V2X_ROOT)/app/trunk/i2v/shm_inc
TPS_PATH=$(V2X_ROOT)/srv/tps
CONF_SRV_PATH=$(V2X_ROOT)/srv/conf_srv
AEROLINK_INC_PATH=$(V2X_ROOT)/srv/radio/common/Aerolink/include

INC=-I. -I${INC_DIR} -I${COMMON_DIR} -I${PAL_DIR} -I${RIS_DIR} -I${NS_DIR}
INC+= -I${I2V_UTIL_PATH}
INC+= -I${I2V_COMMON_PATH}
INC+= -I${PAL_PATH}
INC+= -I${I2V_SHM_PATH}
INC+= -I${J2735_UPER_DIR}
INC+= -I${ASN1_DIR}
INC+= -I${CONF_SRV_PATH}

LIBS=-L${PAL_DIR} -L${RIS_DIR} -L${ALSMI_API_DIR} -lris -lalsmi_api -lpal -lpthread -lrt

ALL_EXECUTABLES=nsstats nsconfig smiUpdatePositionAndTime getAerolinkVersion capture_pcap
##PREBUILT_EXECUTABLES=mk5stats
PREBUILT_EXECUTABLES=

CFLAGS+=-Wall
CFLAGS+=-DSECTON      # To enable Autotalks-SECTON definitions in radio/common/ns_cv2x.h
CFLAGS+=$(LDFLAGS)    # For OpenEmbedded GNU_HASH in binary

LIB_DEPENDENCIES=$(PAL_DIR)/libpal.so $(RIS_DIR)/libris.so

#For unit test builds.
I2V_STUB_DIR  = $(V2X_ROOT)/app/trunk/i2v/stubs
CUNIT_FLAGS   +=  -DNO_SECURITY
CUNIT_INCS    = -I$(I2V_STUB_DIR) -I${DN_TYPES_PATH} -I${TPS_PATH} -I${AEROLINK_INC_PATH} -I${COMMON_PATH} -I${NS_DIR} -I${RIS_PATH}
CUNIT_TEST_FILE = unit_test_radio_util.c
#Add source under test.
CUNIT_SOURCE  = capture_pcap.c 
CUNIT_SOURCE  += $(RIS_DIR)/type2str.c 
CUNIT_SOURCE  += $(ALSMI_API_DIR)/alsmi_api.c
CUNIT_SOURCE  += $(V2X_ROOT)/app/trunk/i2v/util/i2v_util.c 
CUNIT_SOURCE  += $(PAL_PATH)/wsu_sharedmem.c 
CUNIT_SOURCE  += $(PAL_PATH)/ipcsock.c 
CUNIT_SOURCE  += $(V2X_ROOT)/app/trunk/i2v/stubs/stubs.c
CUNIT_BINS    = ./capture_pcap.o   
CUNIT_BINS    += ./type2str.o 
CUNIT_BINS    += ./alsmi_api.o
CUNIT_BINS    += ./i2v_util.o 
CUNIT_BINS    += ./wsu_sharedmem.o  
CUNIT_BINS    += ./stubs.o
CUNIT_BINS    += ./ipcsock.o
#Call this to run test.
CUNIT_EXE     = unit_test_radio_util
#Extra test output to clean.
CUNIT_OUTPUT  = *.gcov *.gcda *.gcno

.PHONY: all clean install check

#---Default Rule:
all: ${ALL_EXECUTABLES}
	@echo Done ${ALL_EXECUTABLES}

nsstats: nsstats.c $(RIS_DIR)/type2str.c  ${COMMON_DIR}/ris_struct.h ${COMMON_DIR}/rs.h ${INC_DIR}/dn_types.h ${NS_DIR}/shm_rsk.h ${PAL_DIR}/ipcsock.h ${PAL_DIR}/wsu_sharedmem.h ${LIB_DEPENDENCIES} ${ALSMI_API_DIR}/libalsmi_api.so
	@echo "-----Building nsstats-----"
	$(CC) ${CFLAGS} ${INC} -I${NS_DIR} -I${AEROLINK_DIR} -o nsstats nsstats.c $(RIS_DIR)/type2str.c ${LIBS}

nsconfig: nsconfig.c $(RIS_DIR)/type2str.c  ${COMMON_DIR}/ris_struct.h ${COMMON_DIR}/rs.h ${INC_DIR}/dn_types.h ${NS_DIR}/shm_rsk.h ${PAL_DIR}/ipcsock.h ${PAL_DIR}/wsu_sharedmem.h ${LIB_DEPENDENCIES} ${ALSMI_API_DIR}/libalsmi_api.so
	@echo "-----Building nsconfig-----"
	$(CC) ${CFLAGS} ${INC} -I${NS_DIR} -o nsconfig nsconfig.c $(RIS_DIR)/type2str.c ${LIBS}

smiUpdatePositionAndTime: smiUpdatePositionAndTime.c ${RIS_DIR}/type2str.c ${COMMON_DIR}/rs.h  ${INC_DIR}/dn_types.h ${PAL_DIR}/wsu_sharedmem.h ${COMMON_DIR}/alsmi_api.h ${TPS_DIR}/tps_api.h ${RIS_DIR}/type2str.h ${LIB_DEPENDENCIES} ${ALSMI_API_DIR}/libalsmi_api.so ${TPS_DIR}/libtpsapi.so
	@echo "-----Building smiUpdatePositionAndTime-----"
	$(CC) ${CFLAGS} ${INC} -I${TPS_DIR} -I${AEROLINK_DIR} -o smiUpdatePositionAndTime smiUpdatePositionAndTime.c ${RIS_DIR}/type2str.c ${LIBS} -L${TPS_DIR} -ltpsapi

getAerolinkVersion: getAerolinkVersion.c ${RIS_DIR}/type2str.c ${COMMON_DIR}/rs.h ${INC_DIR}/dn_types.h  ${COMMON_DIR}/alsmi_api.h ${LIB_DEPENDENCIES} ${ALSMI_API_DIR}/libalsmi_api.so
	@echo "-----Building getAerolinkVersion-----"
	$(CC) ${CFLAGS} ${INC} -I${TPS_DIR} -I${AEROLINK_DIR} -o getAerolinkVersion getAerolinkVersion.c ${RIS_DIR}/type2str.c ${LIBS}

capture_pcap: capture_pcap.c $(RIS_DIR)/type2str.c  ${COMMON_DIR}/ris_struct.h ${COMMON_DIR}/rs.h ${INC_DIR}/dn_types.h ${NS_DIR}/shm_rsk.h ${NS_DIR}/ns_pcap.h ${PAL_DIR}/ipcsock.h ${PAL_DIR}/wsu_sharedmem.h ${LIB_DEPENDENCIES} ${ALSMI_API_DIR}/libalsmi_api.so
	@echo "-----Building nsstats-----"
	$(CC) ${CFLAGS} ${INC} -I${NS_DIR} -I${AEROLINK_DIR} -o capture_pcap capture_pcap.c $(RIS_DIR)/type2str.c ${LIBS}

${PAL_DIR}/libpal.so:
	make -C ${PAL_DIR}

${RIS_DIR}/libris.so:
	make -C ${RIS_DIR}

${ALSMI_API_DIR}/libalsmi_api.so:
	make -C ${ALSMI_API_DIR}

${TPS_DIR}/libtpsapi.so:
	make -C ${TPS_DIR}

coverage:
	./$(CUNIT_SOURCE)
	gcov *.c

clean:
	@echo "Cleaning ${ALL_EXECUTABLES}"
	rm -rf ${ALL_EXECUTABLES}
	rm -f $(CUNIT_EXE) $(CUNIT_OUTPUT) *.tmp *.gch *.o

install: all ${PREBUILT_EXECUTABLES}
ifdef INSTALL_PATH
	@test -d $(INSTALL_PATH)/bin || mkdir -p $(INSTALL_PATH)/bin
	cp $(ALL_EXECUTABLES) $(PREBUILT_EXECUTABLES) $(INSTALL_PATH)/bin
else
	@echo "ERROR: srv/radio/test-Makefile: Please define INSTALL_PATH before calling make install !"
	exit 1
endif

unit:
	rm -rf $(BINS) *.o
	rm -f $(CUNIT_EXE) $(CUNIT_OUTPUT) *.tmp *.gch
	gcc -c -fPIC $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INC) $(CUNIT_SOURCE)
	gcc          $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INC) $(CUNIT_TEST_FILE) $(CUNIT_BINS) $(CUNIT_LIBS) -o $(CUNIT_EXE)

check:
	cppcheck -q --force --inline-suppr nsstats.c ${RIS_DIR}/type2str.c
	cppcheck -q --force --inline-suppr nsconfig.c ${RIS_DIR}/type2str.c
	cppcheck -q --force --inline-suppr smiUpdatePositionAndTime.c ${RIS_DIR}/type2str.c
	cppcheck -q --force --inline-suppr getAerolinkVersion.c ${RIS_DIR}/type2str.c
	cppcheck -q --force --inline-suppr capture_pcap.c ${RIS_DIR}/type2str.c
