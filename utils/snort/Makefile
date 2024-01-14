
# DENSO-created makefile to fit Snort into the main fullgit Makefile for
#   bitbake builds.  Snort needs open-source projects daq and libdnet, which
#   are built first as part of the make, and installed locally into ./install
#   for snort to access them as it builds itself.

# LOCAL_INSTALL_DIRS = bin include lib man sbin share

all: test_dependencies
	make -C daq-2.0.7 install
	make -C libdnet-master install
	make -C snort-2.9.17

# To compile daq, have to install bison and flex
test_dependencies:
ifeq (,$(wildcard /usr/bin/bison))
    $(error You have to install bison first to compile snort/daq)
endif
ifeq (,$(wildcard /usr/bin/flex))
    $(error You have to install flex first to compile snort/daq)
endif

install: test_dependencies
	make -C daq-2.0.7 install
	make -C libdnet-master install
	make -C snort-2.9.17 install
ifdef INSTALL_PATH
	cp -p -r install/lib/* $(INSTALL_PATH)/lib
	cp -p -r install/bin/* $(INSTALL_PATH)/bin
else
	cp -p -r install/lib/* ../../install/lib
	cp -p -r install/bin/* ../../install/bin
endif

clean:
#rm -rf $(LOCAL_INSTALL_DIRS)
	rm -rf install
	make -C daq-2.0.7 clean
	make -C libdnet-master clean
	make -C snort-2.9.17 clean


