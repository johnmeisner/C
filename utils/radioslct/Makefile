#-------------------------------------------------------------------------------
# File: Source/C/app/utils/radioslct/Makefile
#
# Copyright (C) 2021 DENSO International America, Inc.
#
# Description: radioslct Makefile
#------------------------------------------------------------------------------

V2X_ROOT=../..
include $(V2X_ROOT)/v2xcommon.mk

CFLAGS += -Wall

# Object modules
OBJS  = radioslct.o

# Binaries
BINS = radioslct

.PHONY: all clean install check

all: $(BINS)

radioslct: $(OBJS)
	@echo "INSTALL_PATH=$(INSTALL_PATH)"
	@echo "LIB_PATH=$(LIB_PATH)"
	$(CC) $(CFLAGS) -o radioslct $(OBJS) $(LIBS)

radioslct.o: radioslct.c
	$(CC) $(CFLAGS) $(INCS) -c radioslct.c

clean:
	rm -rf $(BINS) *.o

install: all

ifdef INSTALL_PATH
	@echo "Radioslct: make install... "
	@test -d $(INSTALL_PATH) || (mkdir $(INSTALL_PATH); echo "*---Creating Installation Directory ($(INSTALL_PATH)).")
	@test -d $(INSTALL_PATH)/local/bin || (mkdir -p $(INSTALL_PATH)/local/bin; echo "*---Creating Installation Directory ($(INSTALL_PATH)/local/bin).")
	cp $(BINS) $(INSTALL_PATH)/local/bin

else
	@echo "ERROR: radioslct: Please Define INSTALL_PATH Before Calling make install"
	exit 1
endif

check:
	cppcheck -q --force --inline-suppr radioslct.c
