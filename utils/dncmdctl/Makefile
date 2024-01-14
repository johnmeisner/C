#-------------------------------------------------------------------------------
# File: Source/C/app/utils/dncmdctl/Makefile
#
# Copyright (C) 2021 DENSO International America, Inc.
#
# Description: dncmdctl Makefile
#------------------------------------------------------------------------------

V2X_ROOT=../..
include $(V2X_ROOT)/v2xcommon.mk

CFLAGS += -Wall

# Object modules
OBJS  = dncmdctl.o

# Binaries
BINS = dncmdctl

.PHONY: all clean install

all: $(BINS)

dncmdctl: $(OBJS)
	@echo "INSTALL_PATH=$(INSTALL_PATH)"
	@echo "LIB_PATH=$(LIB_PATH)"
	$(CC) $(CFLAGS) -o dncmdctl $(OBJS) $(LIBS)

dncmdctl.o: dncmdctl.c
	$(CC) $(CFLAGS) $(INCS) $(LIBS) -c dncmdctl.c

clean:
	rm -rf $(BINS) *.o

install: all

ifdef INSTALL_PATH
	@echo "dncmdctl: make install... "
	@test -d $(INSTALL_PATH) || (mkdir $(INSTALL_PATH); echo "*---Creating Installation Directory ($(INSTALL_PATH)).")
	@test -d $(INSTALL_PATH)/local/bin || (mkdir -p $(INSTALL_PATH)/local/bin; echo "*---Creating Installation Directory ($(INSTALL_PATH)/local/bin).")
	chmod u+s dncmdctl
	chmod g+s dncmdctl
	cp -p $(BINS) $(INSTALL_PATH)/local/bin

else
	@echo "ERROR: dncmdctl: Please Define INSTALL_PATH Before Calling make install"
	exit 1
endif

