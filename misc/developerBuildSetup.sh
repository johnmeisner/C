#!/bin/bash

set -e

unset force
TOPDIR=`pwd | sed 's/\(.*\)trunk.*/\1/'`
if [ -z $BSPDIR ]; then
  BSPDIR=$TOPDIR/rsu5940_h7e_imx8_yocto_v0.2.3.1
fi

buildtype=nor
platform=MD

######################################################
# Function: usage
#
# Purpose: print out help for the user; use this
#          function when adding new options to help
#          a user understand the purpose of the
#          newly added item
#
# Needs:
#  N/A
######################################################
usage()
{
    echo
    echo "Usage: "
    echo "  `basename $1` <OPTIONS>"
    echo
    echo "    where <OPTIONS> can be of the following: "
    echo -e "      help\t\tPrint this help message"
    echo -e "      basedev\t\tThis option generates a developer build (no password)"
    echo -e "      inspection\tThis option generates an inspection build"
    echo -e "      reset\t\tReturn workspace to state for customer build"
    echo -e "      resetnollc\t\tReturn workspace to state for customer build skipping LLC mod (full build needed for that)"
    echo -e "      softreset\t\tReturn files in workspace but don't run bitbake steps"
    echo -e "      nor\t\tBuild generated is for NOR image"
    echo -e "      microsd\t\tBuild generated is for microsd image"
    echo -e "      emmc\t\tBuild generated is for emmc image"
    echo -e "      MD\t\tBuild generated is safe for Monitor Deployment platform"
    echo -e "      MP\t\tBuild generated is safe for Mass Production platform"
    echo -e "      MDMP\t\tBuild generated is safe for Monitor Deployment or Mass Production platform"
    echo -e "      -f\t\tForce option; the force option may not be wise unless you know its purpose"
    echo
    echo  "  If you don't specify a target (nor|microsd|emmc) the default is nor."
    echo  "  If you don't specify a platform (MD|MP|MDMP) the default is MD."
    echo
}

######################################################
# Function: prepBB
#
# Purpose: performs bitbake preparation for next
#          build
#
# Needs:
#  pass in input string if needed of list of packages
#  for rebuild preparation; list must be passed with
#  '/' instead of spaces
######################################################
prepBB()
{
    DEFAULTBBLIST="fsl-image-machine-test fullgit procps linux-imx wnc-files libllc base-files systemd"
    if [[ ! -z $dollc ]]; then
        DEFAULTBBLIST="$DEFAULTBBLIST llc-mod"
    fi

    if [[ ! -z "$1" ]]; then
        # input assumed to be a list of packages with a delimiter of '/'
        DEFAULTBBLIST=`echo $1 | sed 's/\// /g'`
    fi

    echo -e "\nPreparing to bitbake clean $DEFAULTBBLIST..."
    cd $BSPDIR
    source setup-environment build-imx-xwayland-c0
    bitbake -f -c cleansstate $DEFAULTBBLIST

    if [ ! -z "`echo $DEFAULTBBLIST | grep fullgit`" ]; then
        if [ -z $cleanfullgit ]; then
            echo -e "\nGoing to clean fullgit...this may take a while; be patient there is no error"
            rm -rf `find $BSPDIR/build-imx-xwayland-c0/tmp -type d | grep fullgit`
            cleanfullgit=1
        fi
    fi

}

######################################################
# Function: checkfiles
#
# Purpose: evalutes whether files are modified
#          as tracked by svn
#
# Needs:
#  space separated list of files
######################################################
checkfiles()
{
    for filenm in "$@"
    do
        if [[ ! -z "`svn stat $filenm | grep ^M`" ]]; then
            if [[ -z $ckprint ]]; then
                ckprint=1
                echo "Warning: modified file(s); you should reset first to avoid errors:"
            fi
            echo $filenm
        fi
    done

    if [[ ! -z $ckprint && -z $force ]]; then
        exit -1
    fi
}

######################################################
# Function: setTargetType
#
# Purpose: set environment to build for nor, micro
#          or emmc
#
# Needs:
#  none - set by buildtype
######################################################
setTargetType()
{
    case "$buildtype" in
        'emmc' | 'microsd')
            sed -i 's/\(.*fspi\)/#\1/' local.conf
            cp local.conf $BSPDIR/build-imx-xwayland-c0/conf
            if [ ! -z "`echo $buildtype | grep emmc`" ]; then
                cp -f ubootenv/*.emmc $BSPDIR
            else
                cp -f ubootenv/*.microsd $BSPDIR
            fi
            cd mmcbuild
            ;;
        *)
            sed -i 's/#*\(.*fspi\)/\1/' local.conf
            cp local.conf $BSPDIR/build-imx-xwayland-c0/conf
            cd norbuild
            ;;
    esac

    if [[ "`echo $buildtype`" == "microsd" ]]; then
        # one change needed for header file
        sed -i 's/\(.*define CONFIG_MMCROOT.*blk\)[01]\(.*\)/\11\2/' downloads/imx8_uboot_v2018.03_4.14.98_2.3.0/include/configs/imx8qxp_mek.h
        # one change needed for fstab
        sed -i 's/mmcblk0p2/mmcblk1p2/' ../fstab
        if [ ! -z "`diff ../fstab $BSPDIR/sources/poky/meta/recipes-core/base-files/base-files/fstab`" ]; then
            cp ../fstab $BSPDIR/sources/poky/meta/recipes-core/base-files/base-files/fstab
            needprep=1
        fi
        # ensure correct uboot environment

        cp ../ubootenv/u-boot.env.microsd $BSPDIR/u-boot.env.microsd
    else
        # overwrite environment for u-boot
        cp ../ubootenv/u-boot.env.emmc $BSPDIR/u-boot.env.microsd
        # ensure fstab is correct
        svn revert ../fstab
        if [ ! -z "`diff ../fstab $BSPDIR/sources/poky/meta/recipes-core/base-files/base-files/fstab`" ]; then
            cp ../fstab $BSPDIR/sources/poky/meta/recipes-core/base-files/base-files/fstab
            needprep=1
        fi
    fi
    for i in `find . -type f`; do
        if [[ ! -z "`diff $i $BSPDIR/$i`" ]]; then
            cp -vf $i $BSPDIR/$i 
            needprep=1
        else
            echo $i OK
            pwd
            echo diff $i $BSPDIR/$i
            diff $i $BSPDIR/$i
            echo
        fi
    done
    if [[ "`echo $buildtype`" == "microsd" ]]; then
        echo svn revert downloads/imx8_uboot_v2018.03_4.14.98_2.3.0/include/configs/imx8qxp_mek.h
        # the default NOR fw_env.config is for microsd; the one in mmcbuild is for emmc; oveerwrite all the time
        # the needprep will already be set if switching from emmc to micro
        cp ../norbuild/sources/poky/meta/recipes-bsp/u-boot/files/fw_env.config $BSPDIR/sources/poky/meta/recipes-bsp/u-boot/files/fw_env.config
    fi

    if [ ! -z $needprep ]; then
        prepBB imx-boot/u-boot-imx/fullgit/wnc-files/u-boot/libpam
    fi
}

######################################################
# Function: prepareInspection
#
# Purpose: enable the next build to support inpsection
#          test
#
# Needs:
#  disable kernel panic
#  disable watchdog
#  disable dcu (+ enable editable rwflash script)
#  disable password
#  disable RSU apps
#  enable inspection scripts
######################################################
prepareInspection()
{
    curdir=`pwd`
    checkfiles procsp_* fullgit_1.0.bb rsuhealth.service fsl-image-machine-test.bbappend ../Makefile

    # disable kernel panic
    sed -i 's/echo.*/echo inpsection build/' procps_*

    # disable watchdog
    sed -i '/wdrh850/d' fullgit_1.0.bb

    # disable dcu and enable rwflash script
    sed -i '/dcu/a ExecStart=/usr/scripts/inspectionflash.sh' rsuhealth.service
    sed -i '/dcu/d' rsuhealth.service

    # disable password
    sed -i '/EXTRA_USERS/d' fsl-image-machine-test.bbappend
    sed -i '/extrausers/d' fsl-image-machine-test.bbappend
    sed -i '/usermod/d' fsl-image-machine-test.bbappend

    # disable RSU apps
    sed -i 's/\(.*\&$\)/#\1/' startup.sh
    sed -i 's/\(.*snmpd .*\)/#\1/' startup.sh
    sed -i 's/\(.*snmptrapd .*\)/#\1/' startup.sh

    # disable rsuhealth notification for i2v
    sed -i 's/\(rsuhealth -I.*\)/#\1/' startup.sh

    # disable the new block for multiple bozo interfaces
    # this may look fugly, but it actually works
    sed -i '/Start up the bozo/{n;s/\(.*\)/#\1/;n;s/\(.*\)/#\1/;n;s/\(.*\)/#\1/;n;s/\(.*\)/#\1/;n;s/\(.*\)/#\1/;n;s/\(.*\)/#\1/;}' startup.sh

    # allow root login
    sed -i '/allow-empty-p/{N;d}' local.conf

    # update path for scripts
    sed -i 's/\(.*\)\(local\/dnutils.*\)/\1scripts:\/usr\/\2/' profile

    # enable inspection scripts (need to overcome BSD limitation on \t and \n)
    inspectionline=`echo "%cp misc/inspectionscripts/* \\\$(INSTALL_PATH)/scripts/"`
    sed -i "/cp dnu/a \
$inspectionline" ../Makefile
    tr -s '%' '\t' < ../Makefile > newMakefile
    mv newMakefile ../Makefile

    # set files in workspace
    make install BSPDIR=$BSPDIR

    # prepare bitbake
    prepBB fsl-image-machine-test/fullgit/procps

    # final step
    cd $curdir
    setTargetType

    echo ""
    echo Ready for inspection build...run bitbake fsl-image-machine-test in your build shell
}

######################################################
# Function: prepareBasicDeveloper
#
# Purpose: enable the next build to support basic
#          developer
#
# Needs:
#  disable password
######################################################
prepareBasicDeveloper()
{
    curdir=`pwd`
    checkfiles fsl-image-machine-test.bbappend

    # disable password
    sed -i '/EXTRA_USERS/d' fsl-image-machine-test.bbappend
    sed -i '/extrausers/d' fsl-image-machine-test.bbappend
    sed -i '/usermod/d' fsl-image-machine-test.bbappend

    # allow root login
    sed -i '/allow-empty-p/{N;d}' local.conf

    # set files in workspace
    make install BSPDIR=$BSPDIR

    # prepare bitbake
    prepBB fsl-image-machine-test/fullgit

    # final step
    cd $curdir
    setTargetType

    echo ""
    echo Ready for base developer build...run bitbake fsl-image-machine-test in your build shell
}


######################################################
# Begin execution (i.e. 'main')
######################################################
cd $TOPDIR/trunk/hercules/Software/iMX8/Source/C/misc
MAINDIR=`pwd`

if [[ ! -z $2 ]]; then
    if [[ "$2" == "FORCE" ]]; then
        force=1
    fi
fi

# last one wins if multiple actions specified or multiple build types specified
while [ $# -gt 0 ]
do
    case "$1" in
        '-f')
            force=1
            shift
            ;;
        'basedev' | 'inspection' | 'reset' | 'resetnollc' | 'softreset')
            action=$1
            shift

            ;;
        'emmc' | 'microsd' | 'nor')
            buildtype=$1
            shift
            ;;
        'MD' | 'MP' | 'MDMP')
            platform=$1
            shift
            ;;
        *)
            action=dousage
            shift
            ;;
    esac
done

case "$action" in
    'basedev')
        echo "CFLAGS += -DBUILD_DEVINSP" >> ../v2xcommon.mk
        prepareBasicDeveloper
        cd $MAINDIR
        echo Basic Developer Build > $BSPDIR/buildsetup.txt
        ;;
    'inspection')
        echo "CFLAGS += -DBUILD_DEVINSP" >> ../v2xcommon.mk
        prepareInspection
        cd $MAINDIR
        echo Inspection Build > $BSPDIR/buildsetup.txt
        ;;
    'reset' | 'resetnollc' | 'softreset')
        for i in `svn stat | grep -v ^? | awk '{print $NF}'`; do svn revert $i; done
        svn revert ../Makefile
        svn revert ../v2xcommon.mk
        make install BSPDIR=$BSPDIR
        if [[ ! -z "`echo $action | grep -v soft`" ]]; then
            if [[ ! -z "`echo $action | grep -v llc`" ]]; then
              export dollc=1
            fi
            prepBB
            cd $MAINDIR
        fi
        setTargetType
        cd $MAINDIR
        if [[ ! -z "`echo $action | grep -v llc`" ]]; then
          echo -e "\nFinal full reset cleanup; your next build will be a full rebuild"
          rm -rf $BSPDIR/build-imx-xwayland-c0/tmp
        fi
        echo Customer Build > $BSPDIR/buildsetup.txt
        echo -e "\ncompleted reset action"
        ;;
    *)
        usage $0
        ;;
esac

echo Target Platform: $buildtype >> $BSPDIR/buildsetup.txt
echo $platform >> $BSPDIR/buildsetup.txt
