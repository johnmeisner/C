# 
# Current Contents and their modifications from original BSP version:
# 
#   fullgit.bb - The fullgit.bb recipe file with packaging paths for webgui and snort and /usr/scripts
#       -- Added wdkick to build, and added paths /usr/share and /usr/scripts to fullgit packaging
# 
#   mkfatfsimg.sh - command script that builds the config blobs
#       -- New command created, user copies into dev environment /usr/bin/
# 
#   local.conf - copy of local.conf with addition for the "file" command
#       -- "file" and "stress-ng" added to BSP
#
#   denso_startup.service
#       -- added ExecStart for dcu, to start up with configblobs
#
#   wdrh850.service
#       -- Starts the wdkick utility (removes dependency with startup.sh)
#
#   profile
#       -- updated the path for DN utilities
#
#   new_gpio_exp.sh
#       -- updated to disable fan
#
#   pci-imx6.c
#       -- updated to disable fan
#
#   fstab
#       -- updated for automounts (should have been here already)
#
#   fsl-imx8x-mek.dtsi
#       -- updated for removal of obu components
#
#   sdhci-esdhc-imx.c
#       -- updated for parity with inspection BSP
#
#   DSRC Disconnect Patch files (also fsl-imx8x-mek.dtsi above)
#   - 90-usb-power.rules
#
#   eMMC mount patch
#   - systemd_237.bbappend
#   - systemd-files/systemd-udevd.service
#
#   rsuhealth.service
#       -- required for enabling rsuhealth service
#
#   procps_%.bbappend
#       -- should have been here; enables kernel panic
#
#   fsl-image-machine-test.bbappend
#       -- should have been here; enables password
#
#   fsl-imx8qxp-mek.dts
#       -- missing u-boot patch
#
#   kernel.conf
#       -- updates to kernel
#
#   android-tools (recipe and source directory for usb_linux.c)
#       -- enabling android-tools in BSP
#
#   libpam_%.bbappend
#       -- recipe change for password support
#
#   llc-internal.h
#       -- LLC update
#
#   llc-ipv6.c
#       -- LLC update
#
#   llc-module.c
#       -- LLC update
#
#   cw-llc.ko
#       -- LLC update
#
#   DMA_memory.patch
#       -- LLC update
#
#   skbuff.c
#       -- Patch to Linux kernel to fix rare kernel panic in negative IPv6/DSRC testing
#
#   imx-mkimage_git.inc
#       -- Fix to recipe to support URL change upstream (and prevent broken builds)
#
#   ubx_getconfig.py
#       -- Polls the ublox GNSS device for its configuration and prints it
#
#   autotalks_sdk_for_opt.tar.bz2
#       -- Autotalks SDK needed for compiling the 5940 Radio Stack, goes into /opt.
#
#   autotalks cv2x_rrc_config.uper
#       -- Autotalks config for radio in SDK that MUST be correct for radio to work.
#       -- Always apply AFTER the SDK since it doesn't have the correct version.


# LOCATION OF THE BSP TOPDIR (the h7e* directory)
BSPDIR=/home/ivi/work/rsu5940_h7e_imx8_yocto_v0.2.3.1

check:
#   NOTE: We are echo'ing the diff's separately and hiding the actual @diff
#       to make the experience cleaner for the user.  Seeing the diff helps them
#       get the paths to the files, and hiding the "|| (echo ..exit)" part
#       cleans up the output the user sees, and doesn't frighten the user with
#       the word "ERROR" unnecessarily on their screen.
	@echo diff mkfatfsimg.sh /usr/bin/mkfatfsimg.sh
	@diff mkfatfsimg.sh /usr/bin/mkfatfsimg.sh || ( echo Please install mkfatfsimg.sh ; exit 1 )
	@echo diff local.conf $(BSPDIR)/build-imx-xwayland-c0/conf/local.conf
	@diff local.conf $(BSPDIR)/build-imx-xwayland-c0/conf/local.conf || (echo ERROR: Mismatch: Please update your local.conf file ; exit 1)
	@echo diff fullgit_1.0.bb $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/fullgit_1.0.bb
	@diff fullgit_1.0.bb $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/fullgit_1.0.bb || (echo ERROR: Mismatch: Please update your fullgit_1.0.bb recipe file ; exit 1)
	@echo diff denso_startup.service $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/wnc-files/files/systemd/denso_startup.service
	@diff denso_startup.service $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/wnc-files/files/systemd/denso_startup.service || (echo ERROR: Mismatch: Please update your denso_startup.service file ; exit 1)
	@echo diff wdrh850.service $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/files/wdrh850.service
	@diff wdrh850.service $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/files/wdrh850.service || (echo ERROR: Mismatch: Please update your wdrh850.service file ; exit 1)
	@echo diff profile $(BSPDIR)/sources/meta-fsl-bsp-release/imx/meta-bsp/recipes-core/base-files/base-files/profile
	@diff profile $(BSPDIR)/sources/meta-fsl-bsp-release/imx/meta-bsp/recipes-core/base-files/base-files/profile || (echo ERROR: Mismatch: Please update your profile file ; exit 1)
	@echo diff new_gpio_exp.sh $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/wnc-files/files/scripts/new_gpio_exp.sh
	@diff new_gpio_exp.sh $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/wnc-files/files/scripts/new_gpio_exp.sh || (echo ERROR: Mismatch: Please update your new_gpio_exp.sh file ; exit 1)
	@echo diff pci-imx6.c $(BSPDIR)/downloads/imx8_kernel_4.14.98_2.3.0/drivers/pci/dwc/pci-imx6.c
	@diff pci-imx6.c $(BSPDIR)/downloads/imx8_kernel_4.14.98_2.3.0/drivers/pci/dwc/pci-imx6.c || (echo ERROR: Mismatch: Please update your pci-imx6.c file ; exit 1)
	@echo diff fstab $(BSPDIR)/sources/poky/meta/recipes-core/base-files/base-files/fstab
	@diff fstab $(BSPDIR)/sources/poky/meta/recipes-core/base-files/base-files/fstab || (echo ERROR: Mismatch: Please update your fstab file ; exit 1)
	@echo diff fsl-imx8x-mek.dtsi $(BSPDIR)/downloads/imx8_kernel_4.14.98_2.3.0/arch/arm64/boot/dts/freescale/fsl-imx8x-mek.dtsi
	@diff fsl-imx8x-mek.dtsi $(BSPDIR)/downloads/imx8_kernel_4.14.98_2.3.0/arch/arm64/boot/dts/freescale/fsl-imx8x-mek.dtsi || (echo ERROR: Mismatch: Please update your fsl-imx8x-mek.dtsi file ; exit 1)
	@echo diff sdhci-esdhc-imx.c $(BSPDIR)/downloads/imx8_kernel_4.14.98_2.3.0/drivers/mmc/host/sdhci-esdhc-imx.c
	@diff sdhci-esdhc-imx.c $(BSPDIR)/downloads/imx8_kernel_4.14.98_2.3.0/drivers/mmc/host/sdhci-esdhc-imx.c || (echo ERROR: Mismatch: Please update your sdhci-esdhc-imx.c file ; exit 1)
	@echo diff 90-usb-power.rules $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/wnc-files/files/etc/udev/rules.d/90-usb-power.rules
	@diff 90-usb-power.rules $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/wnc-files/files/etc/udev/rules.d/90-usb-power.rules || (echo ERROR: Mismatch: Please update your 90-usb-power.rules file ; exit 1)
	@echo diff systemd_237.bbappend $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/systemd_237.bbappend
	@diff systemd_237.bbappend $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/systemd_237.bbappend || (echo ERROR: Mismatch: Please update your systemd_237.bbappend file ; exit 1)
	@echo diff systemd-files/systemd-udevd.service $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/systemd-files/systemd-udevd.service
	@diff systemd-files/systemd-udevd.service $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/systemd-files/systemd-udevd.service || (echo ERROR: Mismatch: Please update your systemd-udevd.service file ; exit 1)
	@echo diff rsuhealth.service $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/files
	@diff rsuhealth.service $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/files || (echo ERROR: Mismatch: Please update your rsuhealth.service file ; exit 1)
	@echo diff procps_%.bbappend $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit
	@diff procps_%.bbappend $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit || (echo ERROR: Mismatch: Please update your procps_%.bbappend file ; exit 1)
	@echo diff fsl-image-machine-test.bbappend $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit
	@diff fsl-image-machine-test.bbappend $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit || (echo ERROR: Mismatch: Please update your fsl-image-machine-test.bbappend file ; exit 1)
	@echo diff fsl-imx8qxp-mek.dts $(BSPDIR)/downloads/imx8_uboot_v2018.03_4.14.98_2.3.0/arch/arm/dts
	@diff fsl-imx8qxp-mek.dts $(BSPDIR)/downloads/imx8_uboot_v2018.03_4.14.98_2.3.0/arch/arm/dts || (echo ERROR: Mismatch: Please update your fsl-imx8qxp-mek.dts file ; exit 1)
	@echo diff kernel.config $(BSPDIR)/downloads/imx8_kernel_4.14.98_2.3.0/kernel.config
	@diff kernel.config $(BSPDIR)/downloads/imx8_kernel_4.14.98_2.3.0/kernel.config || (echo ERROR: Mismatch: Please update your kernel.config file ; exit 1)
	@echo diff android-tools_%.bbappend $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/android-tools_%.bbappend
	@diff android-tools_%.bbappend $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/android-tools_%.bbappend || (echo ERROR: Mismatch: Please update your android-tools_%.bbappend file ; exit 1)
	@echo diff android-tools-files/usb_linux.c $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/android-tools-files/usb_linux.c
	@diff android-tools-files/usb_linux.c $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/android-tools-files/usb_linux.c || (echo ERROR: Mismatch: Please update your usb_linux.c file ; exit 1)
	@echo diff lpwr-int/files/lpwr-int/lpwr-int.c $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/lpwr-int/files/lpwr-int/lpwr-int.c
	@diff lpwr-int/files/lpwr-int/lpwr-int.c $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/lpwr-int/files/lpwr-int/lpwr-int.c || (echo ERROR: Mismatch: Please update your lpwr-int.c file ; exit 1)
	@echo diff sign_fw_files $(BSPDIR)/sign_fw_files
	@diff sign_fw_files $(BSPDIR)/sign_fw_files || (echo ERROR: Mismatch: Please update your sign_fw_files folder ; exit 1)
	@echo diff libpam_%.bbappend $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit
	@diff libpam_%.bbappend $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit || (echo ERROR: Mismatch: Please update your libpam_%.bbappend file ; exit 1)
	@echo diff llc-internal.h $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/llc-mod/files/llc_modules/
	@diff llc-internal.h $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/llc-mod/files/llc_modules/ || (echo ERROR: Mismatch: Please update your llc-internal.h file ; exit 1)
	@echo diff llc-ipv6.c $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/llc-mod/files/llc_modules/
	@diff llc-ipv6.c $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/llc-mod/files/llc_modules/ || (echo ERROR: Mismatch: Please update your llc-ipv6.c file ; exit 1)
	@echo diff llc-module.c $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/llc-mod/files/llc_modules/
	@diff llc-module.c $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/llc-mod/files/llc_modules/ || (echo ERROR: Mismatch: Please update your llc-module.c file ; exit 1)
	@echo diff cw-llc.ko $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/llc-mod/files/llc_modules/
	@diff cw-llc.ko $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/llc-mod/files/llc_modules/ || (echo ERROR: Mismatch: Please update your cw-llc.ko file ; exit 1)
	@echo diff DMA_memory.patch $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/llc-mod/files/llc_modules/
	@diff DMA_memory.patch $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/llc-mod/files/llc_modules/ || (echo ERROR: Mismatch: Please update your DMA_memory.patch file ; exit 1)
	@echo diff NTCIP-1218-MIB.txt $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/files
	@diff NTCIP-1218-MIB.txt $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/files || (echo ERROR: Mismatch: Please update your NTCIP-1218-MIB.txt file ; exit 1)
	@echo diff SYSLOG-TC-MIB.txt $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/files
	@diff SYSLOG-TC-MIB.txt $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/files || (echo ERROR: Mismatch: Please update your SYSLOG-TC-MIB.txt file ; exit 1)
	@echo diff URI-TC-MIB.txt $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/files
	@diff URI-TC-MIB.txt $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/files || (echo ERROR: Mismatch: Please update your URI-TC-MIB.txt file ; exit 1)
	@echo diff skbuff.c $(BSPDIR)/downloads/imx8_kernel_4.14.98_2.3.0/net/core/skbuff.c
	@diff skbuff.c $(BSPDIR)/downloads/imx8_kernel_4.14.98_2.3.0/net/core/skbuff.c || (echo ERROR: Mismatch: Please update your skbuff.c file ; exit 1)
	@echo diff imx-mkimage_git.inc $(BSPDIR)/sources/meta-fsl-bsp-release/imx/meta-bsp/recipes-bsp/imx-mkimage/imx-mkimage_git.inc
	@diff imx-mkimage_git.inc $(BSPDIR)/sources/meta-fsl-bsp-release/imx/meta-bsp/recipes-bsp/imx-mkimage/imx-mkimage_git.inc || (echo ERROR: Mismatch: Please update your imx-mkimage_git.inc file ; exit 1)
	@echo diff ubx_getconfig.py $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/wnc-files/files/scripts/ubx_getconfig.py
	@diff ubx_getconfig.py $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/wnc-files/files/scripts/ubx_getconfig.py || (echo ERROR: Mismatch: Please update your ubx_getconfig.py file ; exit 1)
	@echo ./autotalks_sdk_helper.sh check
	@./autotalks_sdk_helper.sh check || (echo ERROR: Please update your autotalks SDK ; exit 1)
	@echo diff posixDal.c        $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/sxf1800_RFP8-1/files/hostSw/v2xCryptoPal/dal/src/posixDal.c
	@diff posixDal.c             $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/sxf1800_RFP8-1/files/hostSw/v2xCryptoPal/dal/src/posixDal.c || (echo ERROR: Mismatch: Please update your posixDal.c file ; exit 1)
	@echo diff opensslCryptoAl.c $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/sxf1800_RFP8-1/files/hostSw/v2xCryptoPal/trustAl/cryptoAl/src/opensslCryptoAl.c	
	@diff opensslCryptoAl.c      $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/sxf1800_RFP8-1/files/hostSw/v2xCryptoPal/trustAl/cryptoAl/src/opensslCryptoAl.c || (echo ERROR: Mismatch: Please update your opensslCryptoAl.c file ; exit 1)
	@echo diff v2xSecExample.c   $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/sxf1800_RFP8-1/files/hostSw/ex/src/v2xSecExample.c
	@diff v2xSecExample.c        $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/sxf1800_RFP8-1/files/hostSw/ex/src/v2xSecExample.c || (echo ERROR: Mismatch: Please update your v2xSecExample.c file ; exit 1)
    # All checked passed
	@echo All components match!  You are up to date!

install:
	sudo cp mkfatfsimg.sh /usr/bin/
	cp local.conf $(BSPDIR)/build-imx-xwayland-c0/conf/
	cp fullgit_1.0.bb $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/
	cp denso_startup.service $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/wnc-files/files/systemd/denso_startup.service
	cp wdrh850.service $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/files/
	cp profile $(BSPDIR)/sources/meta-fsl-bsp-release/imx/meta-bsp/recipes-core/base-files/base-files/
	cp new_gpio_exp.sh $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/wnc-files/files/scripts/new_gpio_exp.sh
	cp pci-imx6.c $(BSPDIR)/downloads/imx8_kernel_4.14.98_2.3.0/drivers/pci/dwc/pci-imx6.c
	cp fstab $(BSPDIR)/sources/poky/meta/recipes-core/base-files/base-files/fstab
	cp fsl-imx8x-mek.dtsi $(BSPDIR)/downloads/imx8_kernel_4.14.98_2.3.0/arch/arm64/boot/dts/freescale/fsl-imx8x-mek.dtsi
	cp sdhci-esdhc-imx.c $(BSPDIR)/downloads/imx8_kernel_4.14.98_2.3.0/drivers/mmc/host/sdhci-esdhc-imx.c
	cp 90-usb-power.rules $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/wnc-files/files/etc/udev/rules.d/90-usb-power.rules
	cp systemd_237.bbappend $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/systemd_237.bbappend
	cp -r systemd-files $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit
	cp rsuhealth.service $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/files
	cp procps_%.bbappend $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit
	cp fsl-image-machine-test.bbappend $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit
	cp fsl-imx8qxp-mek.dts $(BSPDIR)/downloads/imx8_uboot_v2018.03_4.14.98_2.3.0/arch/arm/dts
	cp kernel.config $(BSPDIR)/downloads/imx8_kernel_4.14.98_2.3.0/kernel.config
	cp android-tools_%.bbappend $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/android-tools_%.bbappend
	cp -rf android-tools-files $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/
	cp -R lpwr-int $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/lpwr-int
	cp -rf sign_fw_files $(BSPDIR)
	cp libpam_%.bbappend $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit
	cp llc-internal.h $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/llc-mod/files/llc_modules/
	cp llc-ipv6.c $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/llc-mod/files/llc_modules/
	cp llc-module.c $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/llc-mod/files/llc_modules/
	cp cw-llc.ko $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/llc-mod/files/llc_modules/
	cp DMA_memory.patch $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/llc-mod/files/llc_modules/
	cp NTCIP-1218-MIB.txt $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/files
	cp SYSLOG-TC-MIB.txt  $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/files
	cp URI-TC-MIB.txt     $(BSPDIR)/sources/meta-denso/recipes-denso/fullgit/files
	cp skbuff.c $(BSPDIR)/downloads/imx8_kernel_4.14.98_2.3.0/net/core/skbuff.c
	cp imx-mkimage_git.inc $(BSPDIR)/sources/meta-fsl-bsp-release/imx/meta-bsp/recipes-bsp/imx-mkimage/imx-mkimage_git.inc
	cp ubx_getconfig.py  $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/wnc-files/files/scripts/
	cp posixDal.c        $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/sxf1800_RFP8-1/files/hostSw/v2xCryptoPal/dal/src/posixDal.c
	cp opensslCryptoAl.c $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/sxf1800_RFP8-1/files/hostSw/v2xCryptoPal/trustAl/cryptoAl/src/opensslCryptoAl.c
	cp v2xSecExample.c   $(BSPDIR)/sources/meta-h71e-B-sample/recipes-derek/sxf1800_RFP8-1/files/hostSw/ex/src/v2xSecExample.c
	./autotalks_sdk_helper.sh install
