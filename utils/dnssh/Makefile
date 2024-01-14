#-------------------------------------------------------------------------------
# File: Source/C/app/utils/dnssh/Makefile
#
# Copyright (C) 2022 DENSO International America, Inc.
#
# Description: dnssh Makefile
#------------------------------------------------------------------------------

V2X_ROOT=../..
include $(V2X_ROOT)/v2xcommon.mk

CFLAGS += -Wall

# Object modules
OBJS  = dnssh.o

# Binaries
BINS = dnssh

.PHONY: all clean install

all: $(BINS)

dnssh: $(OBJS)
	@echo "INSTALL_PATH=$(INSTALL_PATH)"
	@echo "LIB_PATH=$(LIB_PATH)"
	$(CC) $(CFLAGS) -o dnssh $(OBJS) $(LIBS)

dnssh.o: dnssh.c
	$(CC) $(CFLAGS) $(INCS) $(LIBS) -c dnssh.c

clean:
	rm -rf $(BINS) *.o

install: all

ifdef INSTALL_PATH
	@echo "dnssh: make install... "
	@test -d $(INSTALL_PATH) || (mkdir $(INSTALL_PATH); echo "*---Creating Installation Directory ($(INSTALL_PATH)).")
	@test -d $(INSTALL_PATH)/local/bin || (mkdir -p $(INSTALL_PATH)/local/bin; echo "*---Creating Installation Directory ($(INSTALL_PATH)/local/bin).")
	chmod u+s dnssh
	chmod g+s dnssh
	cp -p $(BINS) $(INSTALL_PATH)/local/bin

else
	@echo "ERROR: dnssh: Please Define INSTALL_PATH Before Calling make install"
	exit 1
endif

