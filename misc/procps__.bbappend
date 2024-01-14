do_install_append() {
    echo "kernel.panic = 20" >> ${D}/etc/sysctl.conf
}
