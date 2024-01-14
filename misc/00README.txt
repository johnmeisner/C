
This directory contains copies of development environment things that live
outside of SVN but need to be sync'd with code or Makefile changes in SVN.

The command "make check" will diff the SVN copies against files in your
dev environment and let you know if they are out of sync.

The command "make install" will copy the copies in this directory to your
local dev environment.

DEVELOPERS:
Put any modifications to the ~/work/rsu5940_h7e_imx8_yocto_v0.2.3.1 tree or to the
build environment into this directory, and modify the Makefile to include them.

HOWEVER:
    There's two exceptions, startup.sh, which lives in here just to be copied
        into the configblob during Makefile make.
    and How_Assemble_Usable_Secton_SDK.txt, which gives the steps to build our
        Autotalks SDK from WNC's Autotalks recipes.  The SDK as given from 
        Autotalks is unusably complicated; when it's unpacked and installed
        from within WNC's recipe, a usable SDK can be assembled from there.
