
# DENSO-created makefile to fit bozohttpd into the main fullgit Makefile for bitbake builds
#
# Bozohttpd retrieved from:
#    http://www.eterna.com.au/bozohttpd/
#    http://www.eterna.com.au/bozohttpd/bozohttpd-20201014.tar.bz2

BOZO_SRC_DIR=bozohttpd-20201014

all:
	make -C $(BOZO_SRC_DIR)

clean:
	make -C $(BOZO_SRC_DIR) clean

install:
	make -C $(BOZO_SRC_DIR)
ifdef INSTALL_PATH
	mkdir -p $(INSTALL_PATH)/bin/
	cp -p $(BOZO_SRC_DIR)/bozohttpd $(INSTALL_PATH)/bin/
else
	mkdir -p ../../install/bin/
	cp -p $(BOZO_SRC_DIR)/bozohttpd ../../install/bin/
endif

