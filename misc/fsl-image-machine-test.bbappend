
inherit extrausers

CORE_IMAGE_EXTRA_INSTALL += " dhcp-client fullgit "

#IMAGE_INSTALL_remove += " v2xse-example-app-8.1 "

EXTRA_USERS_PARAMS = " useradd rsuuser; \
                       usermod -d /home/rsuuser -s /bin/bash -P 'HDFive218' rsuuser; \
                       usermod -P 'killinit!y@' root;"


# this one can run whenever
ROOTFS_POSTPROCESS_COMMAND += " mngpython; "
# this one must run at the end (or endish - bitbake controls order but _append is mandatory)
ROOTFS_POSTPROCESS_COMMAND_append = " mngpw; "

mngpython() {
    cd ${IMAGE_ROOTFS}/usr/lib
    tar cjf ${IMAGE_ROOTFS}/etc/python35.bz2 python3.5
    rm -rf ${IMAGE_ROOTFS}/usr/lib/python3.5/*
    cd -
}

# password management requirements (require min password length and change from prior) + require change on first log in
mngpw() {
    sed -i '/Primary/a password         required                       pam_cracklib.so minlen=15 difok=3' ${IMAGE_ROOTFS}/etc/pam.d/common-password
    sed -i 's/\(rsuuser:[^:]*\):[0-9][0-9]*:\(.*\)/\1:0:\2/' ${IMAGE_ROOTFS}/etc/shadow
}

