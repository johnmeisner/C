#-------------------------------------------------------------------------------
# File: fullgit_1.0.bb
#
# Copyright (C) 2021 DENSO International America, Inc.
#
# Description: Bitbake Recipe for building and including Hercules' full git tree
#------------------------------------------------------------------------------

SUMMARY = "DENSO source tree for Hercules"
DESCRIPTION = "DENSO source tree for Hercules"
LICENSE = "CLOSED"

# Dependency on libssl-dev, for its openssl/sha.h, for srv/tps/gps.c [20190313]
DEPENDS = "openssl"
# Dependency on glib in Radio stack's Qualcomm_Telemetrics libraries libdsutils.so and libtelux_qmi.so
DEPENDS += " glib-2.0 "
DEPENDS += " net-snmp "

# missing from QC telematics
RDEPENDS_${PN} += "iproute2"
#RDEPENDS_${PN} += "rng-tools"  # random number generator?
RDEPENDS_${PN} += "tcpdump"
# end missing

PROVIDES += "denso"

SRC_URI += "file://sshd_config"
SRC_URI += "file://wdrh850.service"
SRC_URI += "file://rsuhealth.service"
SRC_URI += "file://NTCIP-1218-MIB.txt"
SRC_URI += "file://SYSLOG-TC-MIB.txt"
SRC_URI += "file://URI-TC-MIB.txt"
SRC_URI += "file://90-usb-suspend.rules"

inherit module

python do_fetch() {
    bb.note("Hello DENSO fullgit")
}

python do_unpack() {
    bb.plain("Hello DENSO fullgit")
}

# NOTE: The "cp -r full git tree" line has been moved from do_unpack
#   to do_compile so bitbake compiles the current source every bake.
do_compile() {
    [ -e ${DEPLOY_DIR_IMAGE} ] && cp ${TOPDIR}/../u-boot.env.microsd ${DEPLOY_DIR_IMAGE}/u-boot.env
    cp ${TOPDIR}/../sources/meta-denso/recipes-denso/fullgit/files/sshd_config ${WORKDIR}
    cp ${TOPDIR}/../sources/meta-denso/recipes-denso/fullgit/files/wdrh850.service ${WORKDIR}
    cp ${TOPDIR}/../sources/meta-denso/recipes-denso/fullgit/files/rsuhealth.service ${WORKDIR}
    cp ${TOPDIR}/../sources/meta-denso/recipes-denso/fullgit/files/NTCIP-1218-MIB.txt ${WORKDIR}
    cp ${TOPDIR}/../sources/meta-denso/recipes-denso/fullgit/files/SYSLOG-TC-MIB.txt ${WORKDIR}
    cp ${TOPDIR}/../sources/meta-denso/recipes-denso/fullgit/files/URI-TC-MIB.txt ${WORKDIR}
    cp ${TOPDIR}/../sources/meta-denso/recipes-denso/fullgit/files/90-usb-suspend.rules ${WORKDIR}
    rm -rf ${WORKDIR}/C
    cp -r ${TOPDIR}/../../trunk/hercules/Software/iMX8/Source/C ${WORKDIR}
    # multiple workspace support (also works for directories with common structure); bitbake only
    if [ ! -z "`readlink -f $0 | grep h7e_imx8`" ]; then
      # detect the location where script executing this makefile is running and trim to parent of h7e_* directory
      parentpath=`readlink -f $0 | sed 's/\(.*\)\/rsu5940_h7e_imx8_y.*/\1/'`
      # replace fixed work path with actual parent path
      sed -i "s|\/home\/ivi\/work\(.*\)|$parentpath\1|" ${WORKDIR}/C/getBuildInfo.sh
      # AND the svn directory needs it too!
      sed -i "s|\/home\/ivi\/work\(.*\)|$parentpath\1|"  ${TOPDIR}/../../trunk/hercules/Software/iMX8/Source/C/getBuildInfo.sh
      # now collect the structure under /home/ivi from the parent path
      relpath=`echo ${parentpath##${HOME}/}`
      # replace the 'work' reference with actual structure (only BSPDIR)
      sed -i "s|\(BSPDIR.*\)work\(.*\)|\1$relpath\2|" ${WORKDIR}/C/misc/Makefile
    fi
    
    # ublox stuff
    cp -r ${TOPDIR}/../../trunk/hercules/Software/u-blox ${TMPDIR}/work/${MULTIMACH_TARGET_SYS}

    #Copy Telux stuff in /opt
    rm -rf ${WORKDIR}/recipe-sysroot/usr/include/telux
    cp -r /opt/fsl-imx-xwayland/4.14-sumo/sysroots/aarch64-poky-linux/usr/include/telux ${WORKDIR}/recipe-sysroot/usr/include
    cp -r /opt/fsl-imx-xwayland/4.14-sumo/sysroots/aarch64-poky-linux/usr/include/v2x_*.h ${WORKDIR}/recipe-sysroot/usr/include
    
    cp -rf /opt/fsl-imx-xwayland/4.14-sumo/sysroots/aarch64-poky-linux/usr/lib/lib*.so* ${WORKDIR}/recipe-sysroot/usr/lib
    
    # tmp patch for getting over "nothing provides libv2xCrypto.so.2.5()(64bit) needed by fullgit-1.0-r0.imx8qxpc0mek"
    cp -vrf ${TOPDIR}/../../trunk/hercules/Software/iMX8/Source/C/srv/radio/common/Aerolink/lib/libv2xCrypto*.so* ${WORKDIR}/recipe-sysroot/usr/lib
    
    make -C ${WORKDIR}/C everything
}

do_install() {
    export INSTALL_PATH=${D}/usr/
    make -C ${WORKDIR}/C install
    mkdir -p ${D}/etc/ssh
    mkdir -p ${D}/usr/local/bin
    cp ${WORKDIR}/sshd_config ${D}/etc/ssh
    [ -e ${TOPDIR}/../sign_fw_files/publickey.pem ] && cp ${TOPDIR}/../sign_fw_files/publickey.pem ${D}/etc

    mkdir -p ${D}/etc/systemd/system
    mkdir -p ${D}/etc/systemd/system/multi-user.target.wants
    install -m 0644 ${WORKDIR}/wdrh850.service ${D}/etc/systemd/system
    ln -s /etc/systemd/system/wdrh850.service ${D}/etc/systemd/system/multi-user.target.wants/wdrh850.service
    install -m 0644 ${WORKDIR}/rsuhealth.service ${D}/etc/systemd/system
    ln -s /etc/systemd/system/rsuhealth.service ${D}/etc/systemd/system/multi-user.target.wants/rsuhealth.service
    install -d ${D}/usr/share/snmp/mibs/
    install -m 0755 ${WORKDIR}/NTCIP-1218-MIB.txt ${D}/usr/share/snmp/mibs/
    install -m 0755 ${WORKDIR}/URI-TC-MIB.txt     ${D}/usr/share/snmp/mibs/
    install -m 0755 ${WORKDIR}/SYSLOG-TC-MIB.txt  ${D}/usr/share/snmp/mibs/
    install -d ${D}/etc/udev/rules.d/
    install -m 0644 ${WORKDIR}/90-usb-suspend.rules ${D}/etc/udev/rules.d/
    ln -sf /rwflash/configs/aerolink_conf/ ${D}/etc/aerolink
    # Copy autotalks shared libraries
    #cp /opt/autotalks/secton-5.16.3/lib/*.so ${D}/usr/lib/

    # configs cleanup
    rm -rf ${D}/usr/config/bsw.conf
    rm -rf ${D}/usr/config/c*.conf
    rm -rf ${D}/usr/config/eebl.conf
    rm -rf ${D}/usr/config/fcw.conf
    rm -rf ${D}/usr/config/ima.conf
    rm -rf ${D}/usr/config/map*.conf
    rm -rf ${D}/usr/config/p*.conf
    rm -rf ${D}/usr/config/sdh*.conf
    rm -rf ${D}/usr/config/tc.conf
    rm -rf ${D}/usr/config/tel.conf
    rm -rf ${D}/usr/config/tim16.conf
    rm -rf ${D}/usr/config/v*.conf
    rm -rf ${D}/usr/config/w*.conf
    chmod 0700 ${D}/usr/bin/capture_pcap
    rm -f ${D}/usr/lib/libatlkremote_linux_u.so
    rm -f ${D}/usr/lib/libcli.so
    rm -f ${D}/usr/lib/libsmx.so
    rm -f ${D}/usr/lib/libcv2x.so
    rm -f ${D}/usr/bin/cv2x-example

    if [ -e ${TOPDIR}/../buildsetup.txt ]; then
        if [ ! -z "`cat ${TOPDIR}/../buildsetup.txt | grep Customer`" ]; then
            rm -f ${D}/usr/bin/ipv6_user
            # hide inspection scripts from customer
            chmod -R g-r ${D}/usr/scripts
            chmod -R a-r ${D}/usr/scripts
        fi
    fi
}

do_clean() {
    rm -rf ${WORKDIR}/C
}

# This lets bitbake know to package these files
FILES_${PN} += " /usr/config/* "
FILES_${PN} += " /usr/bin/* "
FILES_${PN} += " /usr/lib/* "
FILES_${PN} += " /usr/share/* "
FILES_${PN} += " /usr/scripts/* "
FILES_${PN} += " /usr/local/bin/* "
FILES_${PN} += " /usr/local/www/* "
FILES_${PN} += " /usr/lib/* "
FILES_${PN} += " /usr/local/dnutils/* "
FILES_${PN} += " /etc/ssh/* "
FILES_${PN} += " /etc/systemd/system/* "
FILES_${PN} += " /etc/* "


# Disable bitbake complaining about "contains non-symlink .so"
INSANE_SKIP_${PN}-dev += "dev-elf ldflags"

# Disable bitbake complaining about "No GNU_HASH in the elf binary errors"
INSANE_SKIP_${PN} = "dev-so"
INSANE_SKIP_${PN}_append = " ldflags"
INSANE_SKIP_${PN}_append = " already-stripped"
#INSANE_SKIP_${PN}-dev = "ldflags"
ALLOW_EMPTY_${PN} = "1"
