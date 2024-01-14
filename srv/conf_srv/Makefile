#-------------------------------------------------------------------------------
# File:    Source/C/srv/conf_srv/Makefile
#
# Copyright (C) 2020 DENSO International America, Inc.
#
# Description: Configuration Server and WebGUI
#------------------------------------------------------------------------------

ifndef V2X_ROOT
   V2X_ROOT=../..
endif

include ${V2X_ROOT}/v2xcommon.mk

#-------------------------------------------------------------------------------
#---Paths to project and non-standard system directories:
#-------------------------------------------------------------------------------

PAL_PATH = $(V2X_ROOT)/pal

# Paths to RADIO directories (for quickradio)
RADIO_COMMON_DIR=../radio/common
RADIO_NS_DIR=../radio/ns
RADIO_RIS_DIR=../radio/ris
AEROLINK_DIR=../radio/common/Aerolink/include
I2V_UTIL_DIR=../../app/trunk/i2v/util
I2V_COMMON_DIR=../../app/trunk/i2v/common
I2V_SHM_DIR=../../app/trunk/i2v/shm_inc

#---Library Paths
I2V_LIB_PATH = ../../app/trunk/i2v/bin/usr/lib
RIS_LIB_PATH = ../Radio/ris
TPS_PATH = ../tps
CFG_MGR_PATH = ../../lib/hd-target-utils
NTCIP_1218_PATH=$(V2X_ROOT)/app/trunk/i2v/ntcip-1218

#-------------------------------------------------------------------------------
#---Make Variables used by Make rules:
#-------------------------------------------------------------------------------

CFLAGS += -Wall

INC_PATHS = -I. -I../../inc -I$(PAL_PATH)  -I$(TPS_PATH) -I$(CFG_MGR_PATH)
INC_PATHS += -I$(I2V_COMMON_DIR)
INC_PATHS += -I$(I2V_UTIL_DIR)
INC_PATHS += -I$(I2V_SHM_DIR)
INC_PATHS += -I$(RADIO_COMMON_DIR)
INC_PATHS += -I$(RADIO_RIS_DIR)
INC_PATHS += -I$(J2735_UPER_DIR)
INC_PATHS += -I$(ASN1_DIR)
INC_PATHS += -I$(AEROLINK_DIR)
INC_PATHS += -I$(NTCIP_1218_PATH)

CFLAGS += $(INC_PATHS)  # INC_PATHS part of CFLAGS so that implicit make rules will work.

LD_PATHS = -L$(PAL_PATH)
LD_PATHS += -L${I2V_LIB_PATH} -L${RIS_LIB_PATH} -L${TPS_PATH}

LDFLAGS += -Wl,--no-undefined  # Resolve dependencies on other libraries
LDFLAGS += $(LD_PATHS)

#For unit test builds.
I2V_STUB_DIR  = ../../app/trunk/i2v/stubs
CUNIT_INCS    = -I$(I2V_STUB_DIR)
#Extra test output to clean.
CUNIT_OUTPUT  = *.gcov *.gcda *.gcno *.tmp *.gch *.txt
#conf_manager
CUNIT_TEST_FILE = unit_test_conf_manager.c
CUNIT_SOURCE  = conf_manager.c conf_agent.c ../../lib/hd-target-utils/cfgmgrapi.c ../../app/trunk/i2v/util/i2v_util.c $(PAL_PATH)/wsu_sharedmem.c  ../../app/trunk/i2v/stubs/stubs.c
CUNIT_BINS    = ./conf_manager.o ./conf_agent.o  ./cfgmgrapi.o  ./i2v_util.o ./wsu_sharedmem.o ./stubs.o
CUNIT_EXE     = unit_test_conf_manager

#-------------------------------------------------------------------------------
#---Variables specifying Make Rule Targets:
#-------------------------------------------------------------------------------

ALL_BIN_EXECUTABLES = quickpos quickradio quickuptime runagain conf_manager conf_agent quickrh850 quicksecurity parse_tscbm_conf sous sous_report
ALL_CGIBIN_EXECUTABLES = cgi-login
ALL_LIBRARIES = $(CFG_MGR_PATH)/libcfgmgr.so
ALL_BIN_SCRIPTS = get_conf_val.sh quickservices.sh wsm_util_watchdog.sh


#-------------------------------------------------------------------------------
#---Makefile Rules:
#-------------------------------------------------------------------------------
.PHONY: all clean install check

all: $(ALL_BIN_EXECUTABLES) $(ALL_CGIBIN_EXECUTABLES)

cgi-login: cgi_login.c libcgipost.c libtimefgets.c
	$(CC) $(CFLAGS) -o $@ $^ -lpthread -lrt -lcrypt

conf_manager: conf_manager.c i2v_util.o $(CFG_MGR_PATH)/libcfgmgr.so $(TPS_PATH)/libtpsapi.so $(PAL_PATH)/libpal.so
	$(CC) $(CFLAGS) $(LDFLAGS) -L$(CFG_MGR_PATH) -o  $@ conf_manager.c  i2v_util.o -lcfgmgr -ltpsapi -lpal -pthread -lrt -lm

conf_agent: conf_agent.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lpthread -lrt

i2v_util.o: ../../app/trunk/i2v/util/i2v_util.c
	$(CC) $(CFLAGS) $(INC_PATHS) -c ../../app/trunk/i2v/util/i2v_util.c

quickuptime: quickuptime.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

runagain: runagain.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

quickradio: quickradio.c
	$(CC) ${CFLAGS} $(LDFLAGS) -I$(RADIO_COMMON_DIR) -I$(RADIO_RIS_DIR) -I$(RADIO_NS_DIR) -I$(AEROLINK_DIR) -o $@ $^ $(RADIO_RIS_DIR)/type2str.c -lpal

quickpos: quickpos.c $(PAL_PATH)/libpal.so
	$(CC) $(CFLAGS) -L$(PAL_PATH) -o $@ $^ -lpal

quickrh850: quickrh850.c $(PAL_PATH)/libpal.so
	$(CC) $(CFLAGS) -o $@ $^

quicksecurity: quicksecurity.c
	$(CC) ${CFLAGS} $(LDFLAGS) -I$(RADIO_COMMON_DIR) -I$(RADIO_RIS_DIR) -I$(RADIO_NS_DIR) -I$(AEROLINK_DIR) -o $@ $^ -lpal

parse_tscbm_conf: parse_tscbm_conf.c $(CFG_MGR_PATH)/libtsmapreader.c
	$(CC) $(CFLAGS) -o $@ $^ -lpthread -lrt

sous: sous.c i2v_util.o $(PAL_PATH)/libpal.so sous_rh850.c libtimefgets.c
	$(CC) $(CFLAGS) -L$(PAL_PATH) -o $@ $^ -lpal -lpthread -lrt

sous_report: sous_report.c $(PAL_PATH)/libpal.so
	$(CC) $(CFLAGS) -L$(PAL_PATH) -o $@ $^ -lpal

# Library Dependecies
$(PAL_PATH)/libpal.so:
	make -C $(PAL_PATH)

$(CFG_MGR_PATH)/libcfgmgr.so:
	make -C $(CFG_MGR_PATH) libcfgmgr.so

$(TPS_PATH)/libtpsapi.so:
	make -C $(TPS_PATH) libtpsapi.so

coverage:
	./$(CUNIT_SOURCE)
	gcov *.c

clean:
	rm -f $(ALL_BIN_EXECUTABLES) $(ALL_CGIBIN_EXECUTABLES) *.o
	rm -f $(CUNIT_EXE) $(CUNIT_OUTPUT)

#---Install into INSTALL_PATH.
install: all
ifdef INSTALL_PATH
	@mkdir -p $(INSTALL_PATH)/bin
	cp $(ALL_BIN_EXECUTABLES) $(INSTALL_PATH)/bin
	cp $(ALL_BIN_SCRIPTS) $(INSTALL_PATH)/bin
	@mkdir -p $(INSTALL_PATH)/lib
	cp $(ALL_LIBRARIES) $(INSTALL_PATH)/lib
	@mkdir -p $(INSTALL_PATH)/local/www
	cp -r cert $(INSTALL_PATH)/local/www
	cp -r html $(INSTALL_PATH)/local/www
	cp -r cgi-bin $(INSTALL_PATH)/local/www
	chmod a+x $(INSTALL_PATH)/local/www/cgi-bin/*
	cp $(ALL_CGIBIN_EXECUTABLES) $(INSTALL_PATH)/local/www/cgi-bin
else
	$(error Please define INSTALL_PATH before calling make install)
endif

unit:
	gcc -c -fPIC $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INCS) $(CUNIT_SOURCE)
	gcc          $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INCS) $(CUNIT_TEST_FILE) $(CUNIT_BINS) $(CUNIT_LIBS) -o $(CUNIT_EXE)

check:
	cppcheck -q --force --inline-suppr $(CUNIT_SOURCE)
	cppcheck -q --force --inline-suppr cgi_login.c libcgipost.c libtimefgets.c
	cppcheck -q --force --inline-suppr conf_manager.c
	cppcheck -q --force --inline-suppr conf_agent.c
	cppcheck -q --force --inline-suppr quickuptime.c
	cppcheck -q --force --inline-suppr runagain.c
	cppcheck -q --force --inline-suppr quickradio.c
	cppcheck -q --force --inline-suppr quickpos.c
	cppcheck -q --force --inline-suppr quickrh850.c
	cppcheck -q --force --inline-suppr libtimefgets.c
	cppcheck -q --force --inline-suppr quicksecurity.c
	cppcheck -q --force --inline-suppr parse_tscbm_conf.c ${CFG_MGR_PATH}/libtsmapreader.c
