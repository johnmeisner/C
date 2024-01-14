#-------------------------------------------------------------------------------
# File:    Source/C/srv/tps/Makefile
#
# Copyright (C) 2019 DENSO International America, Inc.
#
# Description: V2X Framework Makefile for TPS Service (Hercules)
#------------------------------------------------------------------------------

ifndef V2X_ROOT
   V2X_ROOT=../..
endif

include ${V2X_ROOT}/v2xcommon.mk

#-------------------------------------------------------------------------------
#---Paths to project and non-standard system directories:
#-------------------------------------------------------------------------------

PAL_PATH         = $(V2X_ROOT)/pal
COMMON_DIR       = $(V2X_ROOT)/srv/radio/common
RIS_DIR          = $(V2X_ROOT)/srv/radio/ris
NS_DIR           = $(V2X_ROOT)/srv/radio/ns
AEROLINK_INC_DIR = $(V2X_ROOT)/srv/radio/common/Aerolink/include
CONF_SRV_DIR     = $(V2X_ROOT)/srv/conf_srv
I2V_COMMON_DIR   = $(V2X_ROOT)/app/trunk/i2v/common
I2V_SHM_DIR      = $(V2X_ROOT)/app/trunk/i2v/shm_inc
I2V_UTIL_DIR     = $(V2X_ROOT)/app/trunk/i2v/util

# UBlox Configuration location and file
ifdef YOCTO_BUILD
UBLOX_CONFIG_PATH = ../../../u-blox/configuration
else
UBLOX_CONFIG_PATH = $(V2X_ROOT)/../../../u-blox/configuration
endif
UBLOX_CONFIG_FILE = Hercules_7.1_u-blox_M8030_ADR4.21_10Hz_UBX2c.txt
UBLOX_CONFIG_INSTALL_NAME = u-blox_config.txt

#-------------------------------------------------------------------------------
#---Make Variables used by Make rules:
#-------------------------------------------------------------------------------
#CFLAGS += -DDEBUG
CFLAGS += -DHAVE_CONFIG_H -DGPS_UPDATE_LINUX_SYSTIME
CFLAGS += -D_GNU_SOURCE  #Required for settimeofday,strptime,etc
CFLAGS += -Wno-unused-function -Wall
####Following line not needed if LDFLAGS is used properly.
####NOT NEEDED: CFLAGS += $(LDFLAGS)    # For OpenEmbedded GNU_HASH in binary

INC_PATHS = -I. -I../../inc -I$(PAL_PATH)
INC_PATHS+= -I${I2V_COMMON_DIR}
INC_PATHS+= -I${I2V_SHM_DIR}
INC_PATHS+= -I${J2735_UPER_DIR}
INC_PATHS+= -I${ASN1_DIR}
INC_PATHS+= -I${COMMON_DIR}
INC_PATHS+= -I${RIS_DIR}
INC_PATHS+= -I${NS_DIR}
INC_PATHS+= -I${AEROLINK_INC_DIR}
INC_PATHS+= -I${CONF_SRV_DIR}

CFLAGS += $(INC_PATHS)  # INC_PATHS part of CFLAGS so that implicit make rules will work.

LDLIBS = -lpal -lm -lpthread -lrt -lcrypto
LD_PATHS = -L$(PAL_PATH) 

LDFLAGS += -Wl,--no-undefined  # Resolve dependencies on other libraries
LDFLAGS += $(LD_PATHS)


#-------------------------------------------------------------------------------
#---Variables specifying Make Rule Targets:
#-------------------------------------------------------------------------------

#---TPSAPI Shared Library (No Versioning so TARGET=SONAME=LDNAME)
LIBTPS_FILES  = tps_api.c
LIBTPS_LDNAME = libtpsapi.so     # Shared Library link name
LIBTPS_SONAME = $(LIBTPS_LDNAME) # Shared Library internal SONAME
LIBTPS_TARGET = $(LIBTPS_SONAME) # build target (SO REALNAME)


ALL_LIBRARIES = $(LIBTPS_TARGET)
###ALL_LIB_VARIANTS = $(LIBTPS_LDNAME) $(LIBTPS_LDNAME)   #not used (No .so versioning)
ALL_LIB_VARIANTS = 

##TODO:Port test tools:  ALL_EXECUTABLES = tps tpstestapp ppstest chat_ublox ublox_config zero_ublox_bias
ALL_EXECUTABLES = tps tpstestapp  ppstestx ppswatchx quickubloxvers

#For unit test builds.
I2V_STUB_DIR  = $(V2X_ROOT)/app/trunk/i2v/stubs
CUNIT_INCS    = -I$(I2V_STUB_DIR)
CUNIT_LIBS    += -lm
CUNIT_TEST_FILE = unit_test_tps.c
#Add source under test.
CUNIT_SOURCE  = main.c gps.c tps.c strl_utils.c $(PAL_PATH)/ipcsock.c $(PAL_PATH)/wsu_util.c $(PAL_PATH)/wsu_sharedmem.c $(I2V_UTIL_DIR)/i2v_util.c $(I2V_STUB_DIR)/stubs.c
CUNIT_BINS    = ./main.o ./gps.o ./tps.o ./i2v_util.o ./strl_utils.o ./ipcsock.o ./wsu_util.o ./wsu_sharedmem.o ./stubs.o
#Call this to run test.
CUNIT_EXE     = unit_test_tps
#Extra test output to clean.
CUNIT_OUTPUT  = *.gcov *.gcda *.gcno

OBJS = gps.o tps.o main.o strl_utils.o
OBJS+= i2v_util.o

#-------------------------------------------------------------------------------
#---Makefile Rules:
#-------------------------------------------------------------------------------
.PHONY: all clean install check

all: $(ALL_EXECUTABLES) ${LIBTPS_TARGET}

tps: $(OBJS) $(PAL_PATH)/libpal.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o tps $(OBJS) ${LDLIBS}

tpstestapp: tpstestapp.c tps_api.c tps_enum_to_str.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

quickubloxvers: quickubloxvers.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lpal 

ppstestx: ppstestx.c timepps.h
	@###$(CC) $(CFLAGS) $(INC_PATHS) -o $@ $^ $(LD_PATHS) ${LDLIBS} 
	$(CC) $(CFLAGS)  -D_GNU_SOURCE -fPIC  -o $@ $^  -lm

ppswatchx: ppswatchx.c timepps.h
	$(CC) $(CFLAGS) -D_GNU_SOURCE -fPIC  -o $@ $^  -lm

i2v_util.o: ${I2V_UTIL_DIR}/i2v_util.c
	$(CC) ${CFLAGS} ${INC} -c ${I2V_UTIL_DIR}/i2v_util.c

$(PAL_PATH)/libpal.so:
	make -C $(PAL_PATH)

#---TPSAPI Shared Library
${LIBTPS_TARGET}: ${LIBTPS_FILES} $(PAL_PATH)/libpal.so
	for srcfile in ${LIBTPS_FILES}; do \
		$(CC) $(CFLAGS)  -fpic -c $$srcfile; \
	done
	$(CC) $(LDFLAGS) -shared -Wall -lc  -Wl,-soname,${LIBTPS_SONAME} -o ${LIBTPS_TARGET}  $(LIBTPS_FILES:c=o) $(LDLIBS) 
	#aarch64-poky-linux-readelf -d $(LIBTPS_TARGET)   |grep [FSN][iOE][lNE][eAD][:ME]   #SO info for shared library

    #---Not used since not doing .so versioning:
    ##ln -sf $(LIBTPS_TARGET) $(LIBTPS_SONAME)
    ##ln -sf $(LIBTPS_SONAME) $(LIBTPS_LDNAME)

coverage:
	./$(CUNIT_SOURCE)
	gcov *.c

clean:
	rm -rf $(ALL_EXECUTABLES) $(ALL_LIBRARIES) $(ALL_LIB_VARIANTS)  *.o *.tmp 
	rm -f $(CUNIT_EXE) $(CUNIT_OUTPUT)  *.tmp *.gch *.csv

#---Install into INSTALL_PATH.
install: all
ifdef INSTALL_PATH
	@test -d $(INSTALL_PATH)/bin || mkdir -p $(INSTALL_PATH)/bin
	cp $(ALL_EXECUTABLES) $(INSTALL_PATH)/bin
	@test -d $(INSTALL_PATH)/lib || mkdir -p $(INSTALL_PATH)/lib
	cp ${LIBTPS_TARGET} ${INSTALL_PATH}/lib
	@test -d $(INSTALL_PATH)/config || mkdir -p $(INSTALL_PATH)/config
	cp *.conf $(INSTALL_PATH)/config
	cp $(UBLOX_CONFIG_PATH)/$(UBLOX_CONFIG_FILE) $(INSTALL_PATH)/config/$(UBLOX_CONFIG_INSTALL_NAME)
else
	@echo "ERROR: srv/tps-Makefile: Please define INSTALL_PATH before calling make install !"
	exit 1
endif

unit:
	gcc -c -fPIC $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INCS) $(CUNIT_SOURCE)
	gcc          $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INCS) $(CUNIT_TEST_FILE) $(CUNIT_BINS) $(CUNIT_LIBS) -o $(CUNIT_EXE)

check:
	cppcheck -q --force --inline-suppr gps.c tps.c main.c strl_utils.c 
	cppcheck -q --force --inline-suppr tpstestapp.c tps_api.c
	cppcheck -q --force --inline-suppr quickubloxvers.c
	cppcheck -q --force --inline-suppr ppstestx.c 
	cppcheck -q --force --inline-suppr ppswatchx.c 
	cppcheck -q --force --inline-suppr ubx_track.c
	cppcheck -q --force --inline-suppr ${LIBTPS_FILES}
