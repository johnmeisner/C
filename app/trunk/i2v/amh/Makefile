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

########################################
# Directories
########################################

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

OBJS = amh.o 
OBJS += imf.o 
OBJS += i2v_util.o
OBJS += amhlib.o
OBJS += amh_stats.o

CFLAGS += -Wall 
CFLAGS += -D_OPTIMIZED 
CFLAGS += -DASN1RT 
CFLAGS += -DGNU 
CFLAGS += -D_GNU_SOURCE 
CFLAGS += -DHAVE_STDINT_H
CFLAGS += -DCPP

INCS = -I$(ASN1_DIR)
INCS += -I$(COMMON_DIR)
INCS += -I$(INC_DIR)
INCS += -I$(J2735_UPER_DIR)
INCS += -I$(PAL_DIR)
INCS += -I$(RIS_DIR)
INCS += -I$(AEROLINK_DIR)
INCS += -I$(TPS_DIR)
INCS += -I$(I2V_COMMON_DIR)
INCS += -I$(I2V_SHM_DIR)
INCS += -I$(CONF_SRV_DIR)
INCS += -I$(I2V_RSEMIB_DIR)
INCS += -I$(RSUHEALTH_DIR)

LIBS = -L$(ASN1_LIB_DIR)
LIBS += -L$(PAL_DIR)
LIBS += -L$(RIS_DIR)
LIBS += -L$(ALSMI_API_DIR)
LIBS += -L$(TPS_DIR)
LIBS += -L$(J2735_UPER_DIR)

LIBS += -lasn1ber
LIBS += -lasn1oer
LIBS += -lasn1per
LIBS += -lasn1rt
LIBS += -lm
LIBS += -lpal
LIBS += -lpthread
LIBS += -lris
LIBS += -lalsmi_api
LIBS += -lrt
LIBS += -ltpsapi
LIBS += -lv2vuperasn1

SO_DEPS += $(PAL_DIR)/libpal.so
SO_DEPS += $(RIS_DIR)/libris.so
SO_DEPS += $(ALSMI_API_DIR)/libalsmi_api.so
SO_DEPS += $(TPS_DIR)/libtpsapi.so

BINS = amh_app amhValidator

LIBS_TO_INSTALL = $(ASN1_LIB_DIR)/libasn1ber.so
LIBS_TO_INSTALL += $(ASN1_LIB_DIR)/libasn1oer.so
LIBS_TO_INSTALL += $(ASN1_LIB_DIR)/libasn1per.so
LIBS_TO_INSTALL += $(ASN1_LIB_DIR)/libasn1rt.so
LIBS_TO_INSTALL += $(J2735_UPER_DIR)/libv2vuperasn1.so

#Start: For unit test builds
#paths
I2V_STUB_DIR = $(I2V_DIR)/stubs
#additional compile flags
CUNIT_FLAGS += -DUNIT_NET_SNMP
CUNIT_FLAGS += -DUNIT_PTHREAD
#include paths
CUNIT_INCS = -I$(I2V_STUB_DIR)
#libs
#CUNIT_LIBS += 
#unit test file
CUNIT_TEST_FILE = unit_test_amh.c
#source under test
CUNIT_SOURCE = amh.c 
CUNIT_SOURCE += imf.c 
CUNIT_SOURCE += amh_stats.c 
CUNIT_SOURCE += amhlib.c 
CUNIT_SOURCE += $(PAL_DIR)/wsu_sharedmem.c 
CUNIT_SOURCE += $(I2V_DIR)/util/i2v_util.c 
CUNIT_SOURCE += $(I2V_STUB_DIR)/stubs.c
#bins
CUNIT_BINS = ./amh.o 
CUNIT_BINS += ./imf.o 
CUNIT_BINS += ./amh_stats.o 
CUNIT_BINS += ./amhlib.o
CUNIT_BINS += ./wsu_sharedmem.o 
CUNIT_BINS += ./i2v_util.o  
CUNIT_BINS += ./stubs.o
#executable
CUNIT_EXE = unit_test_amh
#output to clean
CUNIT_OUTPUT = *.gcov *.gcda *.gcno
#Stop: For unit test builds

#Which confguration file to use with application.
APP_CONF = amh.conf

.PHONY: all clean install lint unit check coverage

all: $(BINS)

amh_app: $(OBJS) $(SO_DEPS)
	$(CC) $(CFLAGS) -o amh_app $(OBJS) $(LDFLAGS) $(LIBS)

amh.o: amh.c amh.h
	$(CC) $(CFLAGS) $(INCS) -c amh.c

amh_stats.o: amh_stats.c amh.h
	$(CC) $(CFLAGS) $(INCS) -c amh_stats.c

imf.o: imf.c
	$(CC) $(CFLAGS) $(INCS) -c imf.c

amhlib.o: amhlib.c
	$(CC) $(CFLAGS) $(INCS) -c amhlib.c

amhValidator: amhValidator.c amhlib.o i2v_util.o
	$(CC) $(CFLAGS) $(INCS) -o $@ $^ $(LDFLAGS) $(LIBS)

i2v_util.o: $(I2V_DIR)/util/i2v_util.c
	$(CC) $(CFLAGS) $(INCS) -c $(I2V_DIR)/util/i2v_util.c

$(ALSMI_API_DIR)/libalsmi_api.so:
	make -C $(ALSMI_API_DIR) libalsmi_api.so

$(PAL_DIR)/libpal.so:
	make -C $(PAL_DIR) libpal.so

$(RIS_DIR)/libris.so:
	make -C $(RIS_DIR) libris.so

$(TPS_DIR)/libtpsapi.so:
	make -C $(TPS_DIR) libtpsapi.so

coverage:
	./$(CUNIT_SOURCE)
	gcov *.c

clean:
	rm -f *.o $(BINS) *.tmp *.gch
	rm -rf ./install
	rm -f $(CUNIT_EXE) $(CUNIT_OUTPUT)

#---Install into INSTALL_PATH.
install: all
ifdef INSTALL_PATH
	@echo "I2V/amh: make install... "
	@test -d $(INSTALL_PATH) || (mkdir $(INSTALL_PATH); echo "*---Creating new installation directory ($(INSTALL_PATH)).")
	@test -d $(INSTALL_PATH)/bin || (mkdir $(INSTALL_PATH)/bin; echo "*---Creating new BIN installation directory ($(INSTALL_PATH)/bin).")
	@test -d $(INSTALL_PATH)/lib || (mkdir $(INSTALL_PATH)/lib; echo "*---Creating new LIB installation directory ($(INSTALL_PATH)/lib).")
	@test -d $(INSTALL_PATH)/config || (mkdir $(INSTALL_PATH)/config; echo "*---Creating CONFIG Installation Directory ($(INSTALL_PATH)/config).")
	cp $(BINS) $(INSTALL_PATH)/bin
	cp $(LIBS_TO_INSTALL) $(INSTALL_PATH)/lib
	cp ./$(APP_CONF) $(INSTALL_PATH)/config
else
	@echo "ERROR: i2v/amh-Makefile: Please define INSTALL_PATH before calling make install !"
	exit 1
endif

unit:
	gcc -c -fPIC $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INCS) $(CUNIT_SOURCE)
	gcc          $(CUNIT_FLAGS) $(CFLAGS) $(CUNIT_INCS) $(INCS) $(CUNIT_TEST_FILE) $(CUNIT_BINS) $(CUNIT_LIBS) -o $(CUNIT_EXE)

lint:
# Too many warnings splint will give up. If so limit to one file and silence some warnings.
	splint amh.c imf.c +showsummary  $(CFLAGS) $(INCS)
# Posix compliance means more warnings.
	#splint amh.c imf.c +posixlib +posixstrictlib +showsummary +strict $(CFLAGS) $(INCS)

check:
	cppcheck -q --force --inline-suppr $(CUNIT_SOURCE)
