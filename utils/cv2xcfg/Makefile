#-------------------------------------------------------------------------------
# File: Source/C/app/utils/cv2xcfg/Makefile
#
# Copyright (C) 2021 DENSO International America, Inc.
#
# Description: cv2xcfg Makefile
#------------------------------------------------------------------------------

V2X_ROOT=../..
include $(V2X_ROOT)/v2xcommon.mk

CFLAGS += -Wall

# Object modules
OBJS  = cv2xcfg.o

# Binaries
BINS = cv2xcfg

.PHONY: all clean install mycppcheck

all: $(BINS)

cv2xcfg: $(OBJS)
	@echo "INSTALL_PATH=$(INSTALL_PATH)"
	@echo "LIB_PATH=$(LIB_PATH)"
	$(CC) $(CFLAGS) -o cv2xcfg $(OBJS) $(LIBS)

cv2xcfg.o: cv2xcfg.c
	$(CC) $(CFLAGS) $(INCS) -c cv2xcfg.c

clean:
	rm -rf $(BINS) *.o

install: all

ifdef INSTALL_PATH
	@echo "cv2xcfg: make install... "
	@test -d $(INSTALL_PATH) || (mkdir $(INSTALL_PATH); echo "*---Creating Installation Directory ($(INSTALL_PATH)).")
	@test -d $(INSTALL_PATH)/local/bin || (mkdir -p $(INSTALL_PATH)/local/bin; echo "*---Creating Installation Directory ($(INSTALL_PATH)/local/bin).")
	cp $(BINS) $(INSTALL_PATH)/local/bin

else
	@echo "ERROR: cv2xcfg: Please Define INSTALL_PATH Before Calling make install"
	exit 1
endif

check:
	cppcheck -q --force --inline-suppr cv2xcfg.c
