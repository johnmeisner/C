#-------------------------------------------------------------------------------
# File: Source/C/Makefile
#
# Copyright (C) 2021 DENSO International America, Inc.
#
# Description: Top-level Makefile for DENSO Hercules source
#------------------------------------------------------------------------------

# Detect if we're building outside a yocto bitbake, invoke hdstandalone if so
# A few make builds only build within a yocto bitbake, and need to be skipped during local builds
# (One of the many variables defined during a yocto bitbake is PKG_CONFIG_DIR)
ifndef PKG_CONFIG_DIR
export HD_STAND_ALONE = TRUE
endif


ifndef V2X_ROOT
   V2X_ROOT=$(CURDIR)
endif

include $(V2X_ROOT)/v2xcommon.mk

.PHONY: everything clean install run_unit_tests check

ifndef HERCIP
HERCIP=192.168.1.80
endif

UNIT_TEST_DIRS =  app/trunk/i2v/main
UNIT_TEST_DIRS += app/trunk/i2v/amh
UNIT_TEST_DIRS += app/trunk/i2v/spat
UNIT_TEST_DIRS += app/trunk/i2v/fwdmsg
UNIT_TEST_DIRS += app/trunk/i2v/scs
UNIT_TEST_DIRS += app/trunk/i2v/srm_rx
UNIT_TEST_DIRS += app/trunk/i2v/ipb
UNIT_TEST_DIRS += app/trunk/i2v/logmgr
UNIT_TEST_DIRS += app/trunk/i2v/iwmh
UNIT_TEST_DIRS += app/trunk/i2v/ntcip-1218
UNIT_TEST_DIRS += utils/rsuhealth
UNIT_TEST_DIRS += srv/tps
UNIT_TEST_DIRS += srv/conf_srv
UNIT_TEST_DIRS += srv/radio/ns
UNIT_TEST_DIRS += utils/lpwrmonitor
UNIT_TEST_DIRS += srv/radio/util
UNIT_TEST_DIRS += utils/pshbtnmonitor
UNIT_TEST_DIRS += utils/rsuaccessmgr

everything:
	@# Make sure user has sourced ~/setup_crosscc_toolchain.sh
ifeq (cc,$(CC))
	@echo ========================
	@echo You must source \~/setup_cross_cc_toolchain.sh for me to build!!
	@echo You must source \~/setup_cross_cc_toolchain.sh for me to build!!
	@echo You must source \~/setup_cross_cc_toolchain.sh for me to build!!
	@echo ========================
	@exit 1
endif
	make -C pal everything
	make -C srv/tps
	make -C srv/radio/ris
	make -C srv/radio/alsmi_api
	make -C srv/radio/ns
	make -C srv/radio/test
	make -C srv/radio/util
ifneq (,$(findstring WEBGUI, $(CFLAGS)))
	make -C srv/conf_srv
	make -C srv/conf_srv/util
	make -C lib/hd-target-utils
	make -C utils/bozohttpd
endif
ifneq (,$(findstring SNORT_SECURITY, $(CFLAGS)))
	make -C utils/snort
endif
	make -C fac/vod
	make -C utils/wdkick
	make -C utils/dnlast
	make -C utils/rsuhealth
	make -C utils/densonor
	make -C utils/radioslct
	make -C utils/dnnetcfg
	make -C utils/cv2xcfg
	make -C utils/lpwrmonitor
	make -C utils/dncmdctl
	make -C utils/dnpasswd
	make -C utils/dnssh
	make -C utils/mdm9150update
	make -C app/trunk/i2v/main
ifndef HD_STAND_ALONE
	make -C app/trunk/i2v/amh
endif
	make -C app/trunk/i2v/spat
	make -C app/trunk/i2v/fwdmsg
ifndef HD_STAND_ALONE
	make -C app/trunk/i2v/scs
endif
	make -C app/trunk/i2v/srm_rx
	make -C app/trunk/i2v/ipb
	make -C app/trunk/i2v/logmgr
	make -C app/trunk/i2v/iwmh
	make -C app/trunk/i2v/util
# SNMP support only in Yocto. Disable for side apps
ifndef HD_STAND_ALONE
	make -C app/trunk/i2v/ntcip-1218
endif
	make -C utils/pshbtnmonitor
	make -C utils/rsuaccessmgr

clean:
	make -C pal clean
	make -C srv/tps clean
	make -C srv/radio/ris clean
	make -C srv/radio/alsmi_api clean
	make -C srv/radio/ns clean
	make -C srv/radio/test clean
	make -C srv/radio/util clean
	make -C srv/conf_srv clean
	make -C srv/conf_srv/util clean
	make -C lib/hd-target-utils clean
	make -C utils/bozohttpd clean
ifneq (,$(findstring SNORT_SECURITY, $(CFLAGS)))
	make -C utils/snort clean
endif
	make -C fac/vod clean
	make -C utils/wdkick clean
	make -C utils/dnlast clean
	make -C utils/rsuhealth clean
	make -C utils/densonor clean
	make -C utils/radioslct clean
	make -C utils/lpwrmonitor clean
	make -C utils/dnnetcfg clean
	make -C utils/cv2xcfg clean
	make -C utils/dncmdctl clean
	make -C utils/dnpasswd clean
	make -C utils/dnssh clean
	make -C utils/mdm9150update clean
	make -C app/trunk/i2v/main clean
	make -C app/trunk/i2v/amh  clean
	make -C app/trunk/i2v/spat  clean
	make -C app/trunk/i2v/fwdmsg  clean
	make -C app/trunk/i2v/scs  clean
	make -C app/trunk/i2v/srm_rx clean 
	make -C app/trunk/i2v/ipb clean
	make -C app/trunk/i2v/logmgr clean
	make -C app/trunk/i2v/iwmh clean
	make -C app/trunk/i2v/util clean
	make -C app/trunk/i2v/ntcip-1218 clean
	make -C utils/pshbtnmonitor clean
	make -C utils/rsuaccessmgr clean
	rm -rf install

# Modified install for yocto build
install:
# Quick test
	make -C pal install
	make -C srv/tps install
	make -C srv/radio/ris install
	make -C srv/radio/alsmi_api install
	make -C srv/radio/ns install
	make -C srv/radio/test install
	make -C srv/radio/util install
ifneq (,$(findstring WEBGUI, $(CFLAGS)))
	make -C srv/conf_srv install
	make -C srv/conf_srv/util install
	make -C lib/hd-target-utils install
	make -C utils/bozohttpd install
endif
ifneq (,$(findstring SNORT_SECURITY, $(CFLAGS)))
	make -C utils/snort install
endif
	make -C srv/radio/common/Aerolink/bin install
	make -C srv/radio/common/Aerolink/lib install
	make -C fac/vod install
	make -C utils/wdkick install
	make -C utils/dnlast install
	make -C utils/rsuhealth install
	make -C utils/densonor install
	make -C utils/radioslct install
	make -C utils/dnnetcfg install
	make -C utils/cv2xcfg install
	make -C utils/lpwrmonitor install
	make -C utils/dncmdctl install
	make -C utils/dnpasswd install
	make -C utils/dnssh install
	make -C utils/mdm9150update install
	make -C app/trunk/i2v/main install
	make -C app/trunk/i2v/spat  install
	make -C app/trunk/i2v/fwdmsg  install
	make -C app/trunk/i2v/srm_rx install
	make -C app/trunk/i2v/ipb install
	make -C app/trunk/i2v/logmgr  install
	make -C app/trunk/i2v/iwmh install
	make -C app/trunk/i2v/util install
# SNMP support only in Yocto. Disable for side apps
ifndef HD_STAND_ALONE
	make -C app/trunk/i2v/amh  install
	make -C app/trunk/i2v/scs  install
	make -C app/trunk/i2v/ntcip-1218 install
endif
	make -C utils/pshbtnmonitor install
	make -C utils/rsuaccessmgr install

	# Copy startup.sh and shutdown.sh into install/config directory as well
	cp misc/startup.sh $(INSTALL_PATH)/config

	# Copy SNMP configs into install/config for inclusion in the config blob
	cp misc/snmp*.conf $(INSTALL_PATH)/config

    # Populate BuildInfo.txt into INST_PATH/share/BuildInfo.txt
	mkdir -p $(INSTALL_PATH)/share/
	./getBuildInfo.sh $(INSTALL_PATH)/share/BuildInfo.txt
    # Create blob of config files
ifneq (,$(findstring CONFIGBLOB, $(CFLAGS)))
    # If creating blob, check if mkfatfsimg.sh is installed
ifeq (,$(wildcard /usr/bin/mkfatfsimg.sh))
	$(error You need to install mkfatfsimg to enable MAKECONFIGBLOB)
endif
	mkdir -p $(INSTALL_PATH)/lib/
	# Temporarily adding BuildInfo.txt to INST_PATH/config to add it into config blob
	cp $(INSTALL_PATH)/share/BuildInfo.txt $(INSTALL_PATH)/config
	/usr/bin/mkfatfsimg.sh $(INSTALL_PATH)/config $(INSTALL_PATH)/lib/ihr
	rm $(INSTALL_PATH)/config/BuildInfo.txt
endif

    # Copy some scripts that were in /scripts on 5900 that will be in /usr/scripts on Herc
    # 20210304: JJG update for any script, not just scripts ending with .sh
	mkdir -p $(INSTALL_PATH)/scripts/
	cp scripts/*[^.txt] $(INSTALL_PATH)/scripts/

    # 20210317: adding utilities that need to be in the path
	mkdir -p $(INSTALL_PATH)/local/dnutils/
	cp dnutils/*[^.txt] $(INSTALL_PATH)/local/dnutils
	chmod 755  $(INSTALL_PATH)/local/dnutils/*.sh
	cp misc/inspectionscripts/* $(INSTALL_PATH)/scripts/
	chmod 755  $(INSTALL_PATH)/scripts/*.sh

scpinstall:
	@test -d install/bin || (echo ERROR: No installation files. First, run make install ; exit 1)
	@# Note the following command freaks everyone out because the "echo ERROR" is
	@# visible.  So it's hidden, and we echo a copy of command before it runs.
	@echo scp -q 'install/bin/*' root@$(HERCIP):/usr/bin/
	@scp -q install/bin/* root@$(HERCIP):/usr/bin/ || (echo ERROR: scp cannot update running executables, stop apps on Herc or check VMware .ssh/known_hosts file; exit 1)
	scp -q install/lib/* root@$(HERCIP):/usr/lib/
	ssh -n root@$(HERCIP) "rm -f /usr/lib/libaerolinkPKI.so"
	ssh -n root@$(HERCIP) "ln -s libaerolinkPKI.so.1 /usr/lib/libaerolinkPKI.so"
	ssh -n root@$(HERCIP) "rm -f /usr/lib/libaerolinkPKI_dummy.so"
	ssh -n root@$(HERCIP) "ln -s libaerolinkPKI_dummy.so.1 /usr/lib/libaerolinkPKI_dummy.so"
	ssh -n root@$(HERCIP) "rm -f /usr/lib/libmisbehaviorReport.so"
	ssh -n root@$(HERCIP) "ln -s libmisbehaviorReport.so.1 /usr/lib/libmisbehaviorReport.so"
	ssh -n root@$(HERCIP) "rm -f /usr/lib/libviicsec.so"
	ssh -n root@$(HERCIP) "ln -s libviicsec.so.1 /usr/lib/libviicsec.so"
	ssh -n root@$(HERCIP) "cp /usr/bin/v2xscppalutil.bin /home/root"
	ssh -n root@$(HERCIP) "mkdir -p /usr/services/version/"
	scp -q install/services/version/* root@$(HERCIP):/usr/services/version/

	# Also copy to /mnt/rwflash/bin and /mnt/rwflash/lib
	@scp -q install/bin/* root@$(HERCIP):/mnt/rwflash/bin/ || (echo ERROR: scp to /mnt/rwflash/bin; exit 1)
	ssh -n root@$(HERCIP) "mkdir /mnt/rwflash/bin/"
	ssh -n root@$(HERCIP) "mkdir /mnt/rwflash/lib/"
	scp -q install/lib/* root@$(HERCIP):/mnt/rwflash/lib/
	
	@echo Successfully copied install files to Hercules.
	@echo NOTE: config files were not copied.  Run '"make scpconfigs"' to copy configs too.
	@echo Done.

scpconfigs:
	scp -q install/config/* root@$(HERCIP):/rwflash/configs/

help:
	@echo This is the main, top-level Makefile for Hercules
	@echo Available make targets:
	@echo "  " make  -- builds the entire source tree
	@echo "  " make clean -- removes all compiled objects, compiled binaries and populated install dir
	@echo "  " make install -- copies compiled binaries and libraries and configs into dir ./install
	@echo "  " make scpinstall -- scp-copies contents of ./install '(except configs)' to Hercules at $(HERCIP)
	@echo "  " make scpconfigs -- scp-copies contents of ./install/configs to Hercules at $(HERCIP)


getCFLAGS:
	@echo "$(CFLAGS)"

check: 
	# Perform static code analysis on srv/radio
	make -C srv/radio/util            check 
	make -C srv/radio/ns              check
	# Perform static code analysis on WEBGUI
	# --Skip bozohttpd as this is a third party tool 
	make -C srv/conf_srv              check 
	make -C srv/conf_srv/util         check
	make -C lib/hd-target-utils       check
	# Perform static code analysis on i2v
	# -- optional static code analysis as
	#
	# -- mandatory static code analysis
	make -C app/trunk/i2v/main        check 
	make -C app/trunk/i2v/amh         check 
	make -C app/trunk/i2v/spat        check
	make -C app/trunk/i2v/fwdmsg      check
	make -C app/trunk/i2v/scs         check
	make -C app/trunk/i2v/srm_rx      check
	make -C app/trunk/i2v/ipb         check
	make -C app/trunk/i2v/logmgr      check
	make -C app/trunk/i2v/iwmh        check
	make -C app/trunk/i2v/ntcip-1218      check
	# Perform static code analysis on libraries
	# --fac/wmh Makefile builds: alsmi_api, cfgcommon, cfp, cmm, listhandler, pal, ris, tps, prm, rvlist, pp, and ph
	# --srv/radio/util Makefile builds: pal, ris, alsmi, and tps
	# --srv/radio/ns Makefile builds: pal (note: calls make on Aerolink, which uses prebuilt libs)
	# --app/trunk/i2v Makefiles (except ntcip-1218) build: alsmi_api, cfgcommon, cfp, cmm, listhandler, pal, ris, and tps
	make -C ./srv/radio/alsmi_api     check
	make -C ./pal                     check
	make -C ./srv/radio/ris           check
	make -C ./srv/tps                 check
	# Perform static code analysis on utils
	make -C ./utils/wdkick            check
	make -C ./utils/densonor          check
	make -C ./utils/radioslct         check
	make -C ./utils/dnnetcfg          check
	make -C ./utils/cv2xcfg           check
	make -C utils/lpwrmonitor         check
	make -C utils/pshbtnmonitor       check
	make -C utils/rsuaccessmgr        check
run_unit_tests:
	@# For each unit test:
	@# - enter its directory
	@# - do a make clean
	@# - build the unit test
	@# - get the name of the unit test executable
	@#   (the one without a file extension)
	@# - if the unit test does not exist
	@#   then report the error  
	@# - otherwise, execute the unit test and pipe its output to awk
	@#   have awk output the name of the unit test followed
	@#   by an indent of failures and an indent of the Run Summary section
	@# - return to the Makefile directory
	@orig_path=`pwd`; \
	for dir in $(UNIT_TEST_DIRS); do \
	  cd $$dir; \
	  make clean > /dev/null 2>&1; \
	  make unit > /dev/null 2>&1; \
	  unit_test=`ls -1 | grep unit_test_ | grep -v \\\\.`; \
	  if ! [ -f "$$unit_test" ]; then \
	    base_dir=`basename unit_test_$$dir`; \
	    echo; \
	    echo unit_test_$$base_dir; \
	    echo; \
	    echo "  ERROR: could not build unit test"; \
	    echo; \
	  else \
	    ./$$unit_test 2> /dev/null | \
	      awk -v tn="$$unit_test" \
	        'BEGIN { \
	          print "\n"tn"\n"  \
	        } \
	        /test_main... had failures:/,/^$$/ { \
	          print "  "$$0 \
	        } \
	        /Run Summary/,/asserts/ { \
	          print "  "$$0 \
	        }'; \
	  fi; \
	  cd $$orig_path; \
	done

