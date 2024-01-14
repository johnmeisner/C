#
# Yocto recipe to build a kernel module out of the kernel tree
# lpwr_int_%.bb  
#

SUMMARY = "GPIO interrupt kernel module for VBAT_FAIL_N signal"
DESCRIPTION = "GPIO interrupt kernel module for VBAT_FAIL_N signal"
#SECTION = "examples"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"
#PR = "r0"

SRC_URI = "file://lpwr-int"

inherit module


S = "${WORKDIR}/lpwr-int"


KERNEL_MODULE_AUTOLOAD += "lpwr-int"

RPROVIDES_${PN} += "kernel-module-lpwr-int"
