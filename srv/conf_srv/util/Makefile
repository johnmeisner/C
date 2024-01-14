#-------------------------------------------------------------------------------
# File:    Source/C/srv/conf_srv/util/Makefile
#
# Copyright (C) 2021 DENSO International America, Inc.
#
# Description: Configuration Server and WebGUI
#------------------------------------------------------------------------------

ifndef V2X_ROOT
   V2X_ROOT=../../..
endif

include ${V2X_ROOT}/v2xcommon.mk

.PHONY: all tests install check

UTILS = post_saver upload_saver file_val dnr2str fw_dl get_token_val get_cred refresh_saver write_cust_devid write_denso_devid
TESTS = test_libcgipost test_timefgets test_timefgets_herc

all: $(UTILS)

tests: $(TESTS)

post_saver: post_saver.c ../libtimefgets.c
	$(CC) -o $@ -I.. $^ -lpthread -lrt

get_token_val: get_token_val.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

file_val: file_val.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

dnr2str: dnr2str.c
	$(CC) $(CFLAGS) -o $@ $^

write_cust_devid: write_cust_devid.c
	$(CC) $(CFLAGS) -DWRITES_CUST_DEVID -o $@ $^

write_denso_devid: write_cust_devid.c
	$(CC) $(CFLAGS) -DWRITES_DENSO_DEVID -o $@ $^

fw_dl: fw_dl.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

upload_saver: upload_saver.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

refresh_saver: refresh_saver.c ../libtimefgets.c
	$(CC) $(CFLAGS) $(LIBS) -I.. -o $@ $^ -lpthread -lrt

test_libcgipost: test_libcgipost.c ../libcgipost.c
	$(CC) -o $@ -I.. $^

test_timefgets: test_timefgets.c ../libtimefgets.c
	$(CC) -o $@ -I.. $^ -lpthread -lrt

clean:
	rm -f $(UTILS) $(TESTS)

#---Install into INSTALL_PATH.
install: all
ifdef INSTALL_PATH
	@mkdir -p $(INSTALL_PATH)/bin
	cp $(UTILS) $(INSTALL_PATH)/bin
else
	$(error Please define INSTALL_PATH before calling make install)
endif

check:
	cppcheck -q --force --inline-suppr *.c ../libtimefgets.c ../libcgipost.c ../libtimefgets.c
