#-------------------------------------------------------------------------------
# File: Source/C/srv/radio/ns/Makefile
#
# Copyright (C) 2022 DENSO International America, Inc.
#
# Description: Makefile for Radio Services
#----------------------------------------------------------------------------#
# 20220511: TODO: JJG removed ns_cv2x and cv2x related headers; update for ATLKS
# 20220524: KF: updated for Autotalks

ifndef V2X_ROOT
   V2X_ROOT=../../..
endif

include ${V2X_ROOT}/v2xcommon.mk

#---Directories
COMMON_DIR=../common
INC_DIR=../../../inc
PAL_DIR=../../../pal
RIS_DIR=../ris
AEROLINK_INC_DIR=../common/Aerolink/include
AEROLINK_LIB_DIR=../common/Aerolink/lib
I2V_COMMON_DIR=../../../app/trunk/i2v/common
I2V_SHM_DIR=../../../app/trunk/i2v/shm_inc
I2V_UTIL_DIR=../../../app/trunk/i2v/util
TPS_DIR=../../tps
CONF_SRV_DIR=../../conf_srv
V2XSE_HOST_SW_V2XSE=/home/ivi/work/rsu5940_h7e_imx8_yocto_v0.2.3.1/sources/meta-h71e-B-sample/recipes-derek/sxf1800_RFP8-1/files/hostSw/v2xCrypto/v2xSe/inc

INC=-I. -I${INC_DIR} -I${COMMON_DIR} -I${AEROLINK_INC_DIR} -I${PAL_DIR} -I${RIS_DIR}
INC+= -I${I2V_COMMON_DIR}
INC+= -I${I2V_SHM_DIR}
INC+= -I${TPS_DIR}
INC+= -I${J2735_UPER_DIR}
INC+= -I${ASN1_DIR}
INC+= -I${CONF_SRV_DIR}
INC+= -I${V2XSE_HOST_SW_V2XSE}

OBJS=rsk.o rpsi.o wme.o nscfg.o  type2str.o alsmi.o ns_pcap.o ns_cv2x_at.o
OBJS+= i2v_util.o

LIBS=-lpal -lstdc++ -pthread -lrt
AEROLINK_LIBS=-lviicsec -lmisbehaviorReport -laerolinkPKI -lv2xCrypto -lv2xCryptoPal -lcrypto

# Autotalks-SECTON specific compilation flags
CFLAGS +=-D__ATLK_CV2X -DSECTON -Wall -Werror -Wstrict-prototypes -I/opt/autotalks/secton-5.17.0/include -L/opt/autotalks/secton-5.17.0/lib

LIBS_AT  = -lcv2x      # libcv2x.so includes cv2x_service_get() and friends
LIBS_AT += -lm         # for pow(), log(), log10f() needed by libcv2x.so
LIBS_AT += -latlkremote_linux_u    # Provides ddm_service_get(), atlk_malloc()
LIBS_AT += -lrefsys    # Needs to come before asn1c! Thanks Linux, for this!
LIBS_AT += -lasn1c
LIBS_AT += -lpoti
LIBS_AT += -ltommath

ALL_EXECUTABLES=radioServices  waitForRadioServicesBooted

CFLAGS+=-Wall
CFLAGS += $(LDFLAGS)    # For OpenEmbedded GNU_HASH in binary
CFLAGS += $(INC)  # INC_PATHS part of CFLAGS so that implicit make rules will work.

LIB_DEPENDENCIES=${PAL_DIR}/libpal.so ${AEROLINK_LIB_DIR}/libaerolinkPKI.so.1

.PHONY: all clean install check

#For unit test builds.
I2V_STUB_DIR  = $(V2X_ROOT)/app/trunk/i2v/stubs
CUNIT_FLAGS   += -DNO_SECURITY
CUNIT_FLAGS   += -Wno-unused-parameter
#CUNIT_FLAGS   += -Wno-unused-result
CUNIT_INCS    = -I$(I2V_STUB_DIR)
CUNIT_LIBS    += -lm
CUNIT_TEST_FILE = unit_test_rsk.c
#Add source under test.
CUNIT_SOURCE  = rsk.c rpsi.c wme.c  nscfg.c ns_pcap.c ns_cv2x_at.c ../ris/type2str.c 
#CUNIT_SOURCE  += alsmi.c 
CUNIT_SOURCE  += $(PAL_DIR)/ipcsock.c $(PAL_DIR)/wsu_util.c $(I2V_UTIL_DIR)/i2v_util.c $(PAL_DIR)/wsu_sharedmem.c $(I2V_STUB_DIR)/stubs.c
CUNIT_BINS    = ./rsk.o ./rpsi.o ./wme.o ./nscfg.o ./ns_pcap.o ./ns_cv2x_at.o ./wsu_sharedmem.o ./type2str.o 
#CUNIT_BINS   += ./alsmi.o 
CUNIT_BINS    +=./ipcsock.o ./wsu_util.o ./i2v_util.o ./stubs.o

#Call this to run test.
CUNIT_EXE     = unit_test_rsk
#Extra test output to clean.
CUNIT_OUTPUT  = *.gcov *.gcda *.gcno

#---Default Rule:
all: ${ALL_EXECUTABLES}

rsk.o: rsk.c  rpsi.h shm_rsk.h wme.h nscfg.h  ${COMMON_DIR}/ris_struct.h ${COMMON_DIR}/rs.h ${COMMON_DIR}/rsu_defs.h ${INC_DIR}/dn_types.h ${PAL_DIR}/ipcsock.h ${PAL_DIR}/wsu_sharedmem.h ${PAL_DIR}/wsu_util.h ${RIS_DIR}/type2str.h
	$(CC) ${CFLAGS} ${INC} -c rsk.c

rpsi.o: rpsi.c rpsi.h  shm_rsk.h wme.h nscfg.h  ${COMMON_DIR}/ris_struct.h ${COMMON_DIR}/rs.h ${COMMON_DIR}/rsu_defs.h ${INC_DIR}/dn_types.h ${PAL_DIR}/ipcsock.h
	$(CC) ${CFLAGS} ${INC} -c rpsi.c

wme.o: wme.c wme.h rpsi.h nscfg.h  shm_rsk.h  ${COMMON_DIR}/ris_struct.h ${COMMON_DIR}/rs.h ${COMMON_DIR}/rsu_defs.h ${INC_DIR}/dn_types.h ${PAL_DIR}/ipcsock.h ${RIS_DIR}/type2str.h
	$(CC) ${CFLAGS} ${INC} -pthread -c wme.c

nscfg.o: nscfg.c nscfg.h  wme.h ${RIS_DIR}/type2str.h  ${COMMON_DIR}/ris_struct.h  ${INC_DIR}/dn_types.h ${COMMON_DIR}/rsu_defs.h
	$(CC) ${CFLAGS} ${INC} -c nscfg.c

ns_pcap.o: ns_pcap.c ns_pcap.h  wme.h  ${COMMON_DIR}/ris_struct.h  ${INC_DIR}/dn_types.h ${COMMON_DIR}/rsu_defs.h
	$(CC) ${CFLAGS} ${INC} -c ns_pcap.c

ns_cv2x.o: ns_cv2x.cpp ${COMMON_DIR}/ns_cv2x.h ${INC_DIR}/dn_types.h
	${CXX} ${CFLAGS} ${INC} -std=c++11 -c ns_cv2x.cpp

ns_cv2x_at.o: ns_cv2x_at.c
	$(CC) ${CFLAGS} ${INC} -c $^

type2str.o: ${RIS_DIR}/type2str.c ${RIS_DIR}/type2str.h  ${COMMON_DIR}/ris_struct.h ${COMMON_DIR}/rs.h ${INC_DIR}/dn_types.h
	$(CC) ${CFLAGS} ${INC} -c ${RIS_DIR}/type2str.c

i2v_util.o: ${I2V_UTIL_DIR}/i2v_util.c
	$(CC) ${CFLAGS} ${INC} -c ${I2V_UTIL_DIR}/i2v_util.c

alsmi.o: alsmi.c alsmi.h  rpsi.h wme.h ${COMMON_DIR}/alsmi_struct.h  ${COMMON_DIR}/ris_struct.h ${COMMON_DIR}/rs.h ${INC_DIR}/dn_types.h
	$(CC) ${CFLAGS} ${INC} -c alsmi.c

${PAL_DIR}/libpal.so:
    # NOTE: CFLAGS='' is so we don't pass our warning-as-errors, otherwise libpal goes boom
	CFLAGS='' make -C ${PAL_DIR}

${AEROLINK_LIB_DIR}/libaerolinkPKI.so.1:
	make -C ${AEROLINK_LIB_DIR}

#---Radio Services Application
radioServices: $(OBJS) $(LIB_DEPENDENCIES)
	$(CC) $(CFLAGS) $(CFLAGS_AT) -L$(AEROLINK_LIB_DIR) -L$(PAL_DIR) -o $@ $(OBJS) $(LIBS) $(AEROLINK_LIBS)  $(LIBS_AT) -lm

#---Wait for Radio Services Booted Application
waitForRadioServicesBooted: waitForRadioServicesBooted.c rpsi.h shm_rsk.h wme.h nscfg.h ${COMMON_DIR}/ris_struct.h ${COMMON_DIR}/rs.h ${COMMON_DIR}/rsu_defs.h ${INC_DIR}/dn_types.h ${PAL_DIR}/ipcsock.h ${PAL_DIR}/wsu_sharedmem.h ${PAL_DIR}/wsu_util.h ${RIS_DIR}/type2str.h
	$(CC) $(CFLAGS) ${INC} -L${PAL_DIR} -o waitForRadioServicesBooted waitForRadioServicesBooted.c -lpal

coverage:
	./$(CUNIT_SOURCE)
	gcov *.c

clean:
	rm -rf ${ALL_EXECUTABLES} *.o
	rm -f $(CUNIT_EXE) $(CUNIT_OUTPUT)  *.tmp *.gch *.csv

install: all
ifdef INSTALL_PATH
	@test -d $(INSTALL_PATH)/bin || mkdir -p $(INSTALL_PATH)/bin
	cp $(ALL_EXECUTABLES) $(INSTALL_PATH)/bin
	@test -d $(INSTALL_PATH)/config || mkdir -p $(INSTALL_PATH)/config
	cp *.conf $(INSTALL_PATH)/config
    # Copy files needed for running Secton.  They need to be in same dir as their
    # executable for the Secton cv2x libraries to find them.  Thanks Autotalks!
	#cp -p /opt/autotalks/secton-5.15/package/* $(INSTALL_PATH)/bin
else
	@echo "ERROR: srv/radio/ns-Makefile: Please define INSTALL_PATH before calling make install !"
	exit 1
endif

unit:
	rm -rf ${ALL_EXECUTABLES} *.o
	rm -f $(CUNIT_EXE) $(CUNIT_OUTPUT)  *.tmp *.gch *.csv
	gcc -c -fPIC $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INCS) $(CUNIT_SOURCE)
	gcc          $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INCS) $(CUNIT_TEST_FILE) $(CUNIT_BINS) $(CUNIT_LIBS) -o $(CUNIT_EXE)

check:
	cppcheck -q --force --inline-suppr rsk.c rpsi.c wme.c nscfg.c ns_pcap.c type2str.c alsmi.c ns_cv2x_at.c
##	cppcheck -q --force --inline-suppr waitForRadioServicesBooted.c
