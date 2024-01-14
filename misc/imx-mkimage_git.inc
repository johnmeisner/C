# Copyright 2017-2019 NXP

IMX_MKIMAGE_SRC ?= "git://source.codeaurora.org/external/imx/imx-mkimage;protocol=https"
SRCBRANCH = "imx_4.14.98_2.3.0"
SRC_URI = "${IMX_MKIMAGE_SRC};branch=${SRCBRANCH}"
SRCREV = "2556000499f667123094af22326cfd8e4cbadaac" 

DEPENDS = "zlib-native openssl-native"
