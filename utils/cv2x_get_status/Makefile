#-------------------------------------------------------------------------------
# File: Source/C/app/utils/cv2x_get_status/Makefile
#
# Copyright (C) 2022 DENSO International America, Inc.
#
# Description: cv2x_get_status Makefile
#------------------------------------------------------------------------------

V2X_ROOT=../..
include $(V2X_ROOT)/v2xcommon.mk

CFLAGS += -Wall

# Object modules
OBJS  = cv2x_get_status.o

# Binaries
BINS = cv2x_get_status

.PHONY: all clean install mycppcheck

all: $(BINS)

cv2x_get_status: $(OBJS)
	@echo "INSTALL_PATH=$(INSTALL_PATH)"
	@echo "LIB_PATH=$(LIB_PATH)"
	$(CC) $(CFLAGS) -o cv2x_get_status $(OBJS) $(LIBS)

cv2x_get_status.o: cv2x_get_status.c
	$(CC) $(CFLAGS) $(INCS) -c cv2x_get_status.c

clean:
	rm -rf $(BINS) *.o

install: all

ifdef INSTALL_PATH
	@echo "cv2x_get_status: make install... "
	@test -d $(INSTALL_PATH) || (mkdir $(INSTALL_PATH); echo "*---Creating Installation Directory ($(INSTALL_PATH)).")
	@test -d $(INSTALL_PATH)/local/bin || (mkdir -p $(INSTALL_PATH)/local/bin; echo "*---Creating Installation Directory ($(INSTALL_PATH)/local/bin).")
	cp $(BINS) $(INSTALL_PATH)/local/bin

else
	@echo "ERROR: cv2x_get_status: Please Define INSTALL_PATH Before Calling make install"
	exit 1
endif

mycppcheck:
	cppcheck -q --force --inline-suppr cv2x_get_status.c
