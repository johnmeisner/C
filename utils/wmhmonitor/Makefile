#-------------------------------------------------------------------------------
# File: Source/C/utils/wmhmonitor/Makefile
#
# Copyright (C) 2020 DENSO International America, Inc.
#
# Description: Makefile to WMH monitor utility
#------------------------------------------------------------------------------

ifndef V2X_ROOT
   V2X_ROOT=../..
endif

include $(V2X_ROOT)/v2xcommon.mk

# Directories
INC_DIR=../../inc
LISTHANDLER_DIR=../../lib/listhandler
PAL_DIR=../../pal
WMH_DIR=../../fac/wmh

OBJS = wmhmonitor.o

CFLAGS += -Wall
ifdef SUPPORT_TC
CFLAGS += -DSUPPORT_TC
endif

INCS = -I$(INC_DIR)
INCS += -I$(LISTHANDLER_DIR)
INCS += -I$(PAL_DIR)
INCS += -I$(WMH_DIR)

LIBS = -L$(PAL_DIR)
LIBS += -lpal

BINS = wmhmonitor

.PHONY: all clean install

all: $(BINS)

$(BINS): $(OBJS)
	$(CC) $(CFLAGS) -o wmhmonitor $(OBJS) $(LIBS)

wmhmonitor.o: wmhmonitor.c
	$(CC) $(CFLAGS) $(INCS) -c wmhmonitor.c
clean:
	rm -f *.o $(BINS) *.tmp *.gch

#---Install into INSTALL_PATH.
install: all
ifdef INSTALL_PATH
	@echo "UTILS/WMHMONITOR-: make install... "
	@test -d $(INSTALL_PATH) || (mkdir $(INSTALL_PATH); echo "*---Creating new installation directory ($(INSTALL_PATH)).")
	@test -d $(INSTALL_PATH)/bin || (mkdir $(INSTALL_PATH)/bin; echo "*---Creating new BIN installation directory ($(INSTALL_PATH)/bin).")
	cp $(BINS) $(INSTALL_PATH)/bin
else
	@echo "ERROR: utils/wsmmonitor-Makefile: Please define INSTALL_PATH before calling make install !"
	exit 1
endif

