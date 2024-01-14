#-------------------------------------------------------------------------------
# File: Source/C/app/utils/wdkick/Makefile
#
# Copyright (C) 2021 DENSO International America, Inc.
#
# Description: wdkick Makefile
#------------------------------------------------------------------------------

V2X_ROOT=../..
include $(V2X_ROOT)/v2xcommon.mk

CFLAGS += -Wall

# Object modules
OBJS  = wdkick.o

# Binaries
BINS = wdkick

.PHONY: all clean install check

all: $(BINS)

wdkick: $(OBJS)
	@echo "INSTALL_PATH=$(INSTALL_PATH)"
	@echo "LIB_PATH=$(LIB_PATH)"
	$(CC) $(CFLAGS) -o wdkick $(OBJS) $(LIBS)

wdkick.o: wdkick.c
	$(CC) $(CFLAGS) $(INCS) -c wdkick.c

clean:
	rm -rf $(BINS) *.o

install: all

ifdef INSTALL_PATH
	@echo "WDKick: make install... "
	@test -d $(INSTALL_PATH) || (mkdir $(INSTALL_PATH); echo "*---Creating Installation Directory ($(INSTALL_PATH)).")
	@test -d $(INSTALL_PATH)/local/bin || (mkdir -p $(INSTALL_PATH)/local/bin; echo "*---Creating Installation Directory ($(INSTALL_PATH)/local/bin).")
	cp $(BINS) $(INSTALL_PATH)/local/bin

else
	@echo "ERROR: wdkick: Please Define INSTALL_PATH Before Calling make install"
	exit 1
endif

check:
	cppcheck -q --force --inline-suppr wdkick.c
