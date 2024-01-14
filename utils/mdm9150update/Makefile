#-------------------------------------------------------------------------------
# File: Source/C/app/utils/mdm9150update/Makefile
#
# Copyright (C) 2021 DENSO International America, Inc.
#
# Description: mdm9150update Makefile
#------------------------------------------------------------------------------

V2X_ROOT=../..
include $(V2X_ROOT)/v2xcommon.mk

CFLAGS += -Wall

# Object modules
OBJS  = mdm9150update.o

# Binaries
BINS = mdm9150update

.PHONY: all clean install

all: $(BINS)

mdm9150update: $(OBJS)
	@echo "INSTALL_PATH=$(INSTALL_PATH)"
	@echo "LIB_PATH=$(LIB_PATH)"
	$(CC) $(CFLAGS) -o mdm9150update $(OBJS) $(LIBS)

mdm9150update.o: mdm9150update.c
	$(CC) $(CFLAGS) $(INCS) $(LIBS) -c mdm9150update.c

clean:
	rm -rf $(BINS) *.o

install: all

ifdef INSTALL_PATH
	@echo "mdm9150update: make install... "
	@test -d $(INSTALL_PATH) || (mkdir $(INSTALL_PATH); echo "*---Creating Installation Directory ($(INSTALL_PATH)).")
	@test -d $(INSTALL_PATH)/local/bin || (mkdir -p $(INSTALL_PATH)/local/bin; echo "*---Creating Installation Directory ($(INSTALL_PATH)/local/bin).")
	chmod u+s $(BINS)
	chmod g+s $(BINS)
	cp -p $(BINS) $(INSTALL_PATH)/local/bin

else
	@echo "ERROR: mdm9150update: Please Define INSTALL_PATH Before Calling make install"
	exit 1
endif

