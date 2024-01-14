FILESEXTRAPATHS_prepend := "${THISDIR}/systemd-files:"

#SRC_URI += "file://system.conf"
#SRC_URI += "file://systemd-networkd.service"
SRC_URI += "file://systemd-udevd.service"

do_install_append() {
    install -d ${D}${sysconfdir}/systemd
#    install -m 0644 ${WORKDIR}/system.conf ${D}${sysconfdir}/systemd/system.conf

#    install -d ${D}${sysconfdir}/systemd/system/multi-user.target.wants
#    install -m 0644 ${WORKDIR}/systemd-networkd.service \
#	${D}${systemd_unitdir}/system/systemd-networkd.service
#    ln -sf ${systemd_unitdir}/system/systemd-networkd.service \
#	${D}${sysconfdir}/systemd/system/multi-user.target.wants/systemd-networkd.service
    install -m 0644 ${WORKDIR}/systemd-udevd.service \
	${D}${systemd_unitdir}/system/systemd-udevd.service
}

#FILES_${PN} += "${sysconfdir}/systemd/system.conf"
