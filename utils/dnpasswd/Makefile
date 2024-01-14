#-------------------------------------------------------------------------------
# File: Source/C/app/utils/dnpasswd/Makefile
#
# Copyright (C) 2021 DENSO International America, Inc.
#
# Description: dnpasswd Makefile
#------------------------------------------------------------------------------

V2X_ROOT=../..
include $(V2X_ROOT)/v2xcommon.mk

CFLAGS += -Wall

# Object modules
OBJS  = dnpasswd.o

# Binaries
BINS = dnpasswd

.PHONY: all clean install

all: $(BINS)

dnpasswd: $(OBJS)
	@echo "INSTALL_PATH=$(INSTALL_PATH)"
	@echo "LIB_PATH=$(LIB_PATH)"
	$(CC) $(CFLAGS) -o dnpasswd $(OBJS) $(LIBS)

dnpasswd.o: dnpasswd.c
	$(CC) $(CFLAGS) $(INCS) $(LIBS) -c dnpasswd.c

clean:
	rm -rf $(BINS) *.o

install: all

ifdef INSTALL_PATH
	@echo "dnpasswd: make install... "
	@test -d $(INSTALL_PATH) || (mkdir $(INSTALL_PATH); echo "*---Creating Installation Directory ($(INSTALL_PATH)).")
	@test -d $(INSTALL_PATH)/local/bin || (mkdir -p $(INSTALL_PATH)/local/bin; echo "*---Creating Installation Directory ($(INSTALL_PATH)/local/bin).")
	chmod u+s dnpasswd
	chmod g+s dnpasswd
	cp -p $(BINS) $(INSTALL_PATH)/local/bin

else
	@echo "ERROR: dnpasswd: Please Define INSTALL_PATH Before Calling make install"
	exit 1
endif

