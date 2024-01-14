#-------------------------------------------------------------------------------
# File:    Source/C/lib/hd-target-utils/Makefile
#
# Copyright (C) 2020 DENSO International America, Inc.
#
# Description: Configuration Server and WebGUI
#------------------------------------------------------------------------------

ifndef V2X_ROOT
   V2X_ROOT=../..
endif

include $(V2X_ROOT)/v2xcommon.mk

#-------------------------------------------------------------------------------
#---Paths to project and non-standard system directories:
#-------------------------------------------------------------------------------

PAL_PATH= $(V2X_ROOT)/pal
CONF_AGENT_PATH= $(V2X_ROOT)/srv/conf_srv
I2V_COMMON_PATH= $(V2X_ROOT)/app/trunk/i2v/common
I2V_RSEMIB_PATH= $(V2X_ROOT)/app/trunk/i2v/ntcip-1218
I2V_SHM_PATH= $(V2X_ROOT)/app/trunk/i2v/shm_inc
COMMON_PATH= $(V2X_ROOT)/srv/radio/common
V2X_INC_PATH= $(V2X_ROOT)/inc
TPS_PATH = $(V2X_ROOT)/srv/tps
RIS_PATH = $(V2X_ROOT)/srv/radio/ris
NTCIP_1218_PATH=$(V2X_ROOT)/app/trunk/i2v/ntcip-1218

#-------------------------------------------------------------------------------
#---Make Variables used by Make rules:
#-------------------------------------------------------------------------------

CFLAGS += -Wall

INC_PATHS = -I.
INC_PATHS += -I$(V2X_INC_PATH)
INC_PATHS += -I$(PAL_PATH)
INC_PATHS += -I$(TPS_PATH)
INC_PATHS += -I$(CONF_AGENT_PATH)
INC_PATHS += -I$(I2V_COMMON_PATH)
INC_PATHS += -I$(I2V_RSEMIB_PATH)
INC_PATHS += -I$(COMMON_PATH)
INC_PATHS += -I$(RIS_PATH)
INC_PATHS += -I$(I2V_SHM_PATH)
INC_PATHS += -I$(ASN1_PATH)
INC_PATHS += -I$(J2735_UPER_PATH)
INC_PATHS += -I$(NTCIP_1218_PATH)

CFLAGS += $(INC_PATHS)  # INC_PATHS part of CFLAGS so that implicit make rules will work.

LDLIBS = -lpal -lm -lcfp -lpthread -lrt -lcrypto
LD_PATHS = -L$(PAL_PATH)
#LD_PATHS += -L$(I2V_LIB_PATH) 
LD_PATHS += -L$(TPS_PATH)

LDFLAGS += -Wl,--no-undefined  # Resolve dependencies on other libraries
LDFLAGS += $(LD_PATHS)

#-------------------------------------------------------------------------------
#---Variables specifying Make Rule Targets:
#-------------------------------------------------------------------------------


CFGMGRLIB = libcfgmgr.so

# FULL LIST OF EXECUTABLES IN EQUIVALENT QNX MAKEFILE
# TODO: Finish porting these to Hercules or remove them from this Makefile and svn
ALL_EXECUTABLES = led_srv led_cmd led_status btn_srv health_monitor_srv dcu flash-helper enable_route custctl
OPTIONAL_EXECUTABLES = dncmd

# These executables have been ported to Hercules
ALL_EXECUTABLES = dcu custctl enable_route health_monitor_srv
OPTIONAL_EXECUTABLES = test_tsmapreader
ALL_LIBRARIES = $(CFGMGRLIB)

#-------------------------------------------------------------------------------
#---Makefile Rules:
#-------------------------------------------------------------------------------

.PHONY: all install clean check
all: $(ALL_EXECUTABLES)  $(ALL_LIBRARIES) $(OPTIONAL_EXECUTABLES)
lib: $(ALL_LIBRARIES)

# todo: to be removed 
LIBS = -L$(LIB_PATH) -lsocket -lpal -lm
	
led_srv: led_srv.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ led_srv.c -lmq -L$(PREBUILT_LIB) -lspi-master -lgpio

led_cmd: led_cmd.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ led_cmd.c -lmq -L$(PREBUILT_LIB)
	
led_status: led_status.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ led_status.c -L$(PREBUILT_LIB) -lspi-master -lgpio
	
btn_srv: btn_srv.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ btn_srv.c -lmq -L$(PREBUILT_LIB) -lspi-master -lgpio -lcfgmgr
    
health_monitor_srv: health_monitor_srv.c
	$(CC) $(CFLAGS) -o $@ health_monitor_srv.c

custctl: custctl.c
	$(CC) $(CFLAGS) -o $@ custctl.c

dcu: cfgmgr.c $(PAL_PATH)/libpal.so
	$(CC) $(CFLAGS) -o $@ cfgmgr.c -L$(PAL_PATH) -lpal

flash-helper: flash-helper.c
	$(CC) $(CFLAGS) -o $@ flash-helper.c

$(CFGMGRLIB): cfgmgrapi.c
	$(CC) $(CFLAGS) -fPIC -shared -Wl,-soname,$(CFGMGRLIB) -o $@ $^

dncmd: dncmd.c $(CFGMGRLIB)
	$(CC) $(CFLAGS) $(LIBS) -o $@ dncmd.c -L . -lcfgmgr

enable_route: enable_route.c
	$(CC) $(CFLAGS) -o $@ enable_route.c

test_tsmapreader: test_tsmapreader.c libtsmapreader.c
	$(CC) $(CFLAGS) -o $@ $^
    
$(PAL_PATH)/libpal.so:
	make -C $(PAL_PATH)

clean:
	rm -f $(ALL_EXECUTABLES) $(ALL_LIBRARIES) $(ALL_LIB_VARIANTS) $(OPTIONAL_EXECUTABLES) *.o

install: all
ifdef INSTALL_PATH
	mkdir -p $(INSTALL_PATH)/bin $(INSTALL_PATH)/lib
	cp $(ALL_EXECUTABLES) $(INSTALL_PATH)/bin
	cp $(ALL_LIBRARIES) $(INSTALL_PATH)/lib
else
	@echo "ERROR: srv/hdmgr-Makefile: Please define INSTALL_PATH before calling make install !"
	exit 1
endif

check:
	cppcheck -q --force --inline-suppr led_srv.c
	cppcheck -q --force --inline-suppr led_cmd.c
	cppcheck -q --force --inline-suppr led_status.c
	cppcheck -q --force --inline-suppr btn_srv.c
	cppcheck -q --force --inline-suppr health_monitor_srv.c
	cppcheck -q --force --inline-suppr custctl.c
	cppcheck -q --force --inline-suppr cfgmgr.c
	cppcheck -q --force --inline-suppr flash-helper.c
	cppcheck -q --force --inline-suppr cfgmgrapi.c
	cppcheck -q --force --inline-suppr dncmd.c
	cppcheck -q --force --inline-suppr enable_route.c
