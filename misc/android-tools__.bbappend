
FILESEXTRAPATHS_prepend := "${THISDIR}/android-tools-files:"

SRC_URI += "file://usb_linux.c"

copynewsrc() {
    cp -f ${WORKDIR}/usb_linux.c ${S}/system/core/adb
}

do_patch[postfuncs] += "copynewsrc"

