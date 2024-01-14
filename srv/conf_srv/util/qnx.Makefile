#
#	Makefile for configuration manager utilities
#

DEBUG = -g
# CC = qcc
# LD = qcc

ifndef V2X_ROOT
   V2X_ROOT=../../../..
endif

include ${V2X_ROOT}/v2xcommon.mk

#---Include Paths
export SQL_INC_PATH=../../../lib/mysql

export CONF_INC_PATH=../../lte

#---Library Paths
export SQL_LIB_PATH=../../../lib/mysql

# PLATFORM_QNX SPECIFIC CONFIGURATION SETTINGS, these should really always be on
ifneq (,$(findstring PLATFORM_QNX, $(CFLAGS)))    # PLATFORM_QNX is defined
    CFLAGS += -Wall -DWSU_QNX
    INC=-I../include -I${SRV_INC_PATH} -I${PAL_INC_PATH} -I${SQL_INC_PATH} -I${TPS_INC_PATH} -I${CONF_INC_PATH}
endif

CFLAGS += -DDEBUG -DHAVE_CONFIG_H -DGPS_UPDATE_LINUX_SYSTIME -DLITTLE_ENDIAN

CFLAGS += $(DEBUG) $(TARGET) -w9 -I../../../pal -I../../../lib/mysql -I../../../../../../srv/tps -I../sr -I.   -I../../inc -I../

LDFLAGS+= $(DEBUG) $(TARGET)

# Path to BSP Prebuilt directory for libspi-master.a
#    NOTE:  libspi-master is only used by POC (not MK5 nor VM)
PREBUILT_DIR=$(V2X_ROOT)/${BSP_ROOT}/prebuilt/armle-v7/lib

ALL_EXECUTABLES = get_token_val fw_dl file_val file_upload_save

all:	$(ALL_EXECUTABLES)  
	echo Done $(ALL_EXECUTABLES)
lib: ${ALL_LIBRARIES}

# todo: to be removed 
LIBS = -L$(LIB_PATH) -lsocket -lpal -lm

get_token_val: get_token_val.o
	$(CC) $(CFLAGS) $(LIBS) -o get_token_val get_token_val.o -lmq 
	
file_val: file_val.o
	$(CC) $(CFLAGS) $(LIBS) -o file_val file_val.o
	
fw_dl: fw_dl.o
	$(CC) $(CFLAGS) $(LIBS) -o fw_dl fw_dl.o -lmq 
	
file_upload_save: file_upload_save.o
	$(CC) $(CFLAGS) $(LIBS) -o file_upload_save file_upload_save.o

clean:
	@echo "Cleaning ${ALL_EXECUTABLES} ${ALL_LIBRARIES} ${ALL_LIB_VARIANTS} *.o *.tmp"
	rm -rf ${ALL_EXECUTABLES} ${ALL_LIBRARIES} ${ALL_LIB_VARIANTS} *.o *.tmp 

#---Install into INSTALL_PATH.
lib_install: lib
ifdef INSTALL_PATH
	@echo "SRV/TPS: make lib_install... "
	@test -d $(INSTALL_PATH) || (mkdir $(INSTALL_PATH); echo "*---Creating Installation Directory ($(INSTALL_PATH)).")
	@test -d $(LIB_PATH) || (mkdir $(LIB_PATH); echo "*---Creating LIB Installation Directory ($(LIB_PATH)).")
	cp -d ${ALL_LIBRARIES} ${ALL_LIB_VARIANTS} ${LIB_PATH}
else
	@echo "ERROR: SRV/TPS: Please Define INSTALL_PATH Before Calling make lib_install"
	exit 1
endif


install: all
ifdef INSTALL_PATH
	@echo "SRV/TPS-: make install... "
	@test -d ${INSTALL_PATH} || (mkdir ${INSTALL_PATH}; echo "*---Creating new installation directory (${INSTALL_PATH}).")
	@test -d ${INSTALL_PATH}/v2x || (mkdir ${INSTALL_PATH}/v2x; echo "*---Creating new LIB installation directory (${INSTALL_PATH}/v2x).")
	cp $(ALL_EXECUTABLES) $(INSTALL_PATH)/v2x
	cp bozohttpd $(INSTALL_PATH)/v2x
else
	@echo "ERROR: srv/tps-Makefile: Please define INSTALL_PATH before calling make install !"
	exit 1
endif

