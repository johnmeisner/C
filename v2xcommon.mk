#/******************************************************************************
# *                                                                            *
# *     File Name:  v2xcommon.mk                                               *
# *     Author:                                                                *
# *         DENSO International America, Inc.                                  *
# *         North America Research Laboratory, California Office               *
# *         3252 Business Park Drive                                           *
# *         Vista, CA 92081                                                    *
# *                                                                            *
# ******************************************************************************
# * (C) Copyright 2019 DENSO International America, Inc.  All rights reserved. *
# *****************************************************************************/
#
# This file defines values that are to be common to all V2XSP Makefiles.
# Specifically, the hardware platform and version, and dependent hardware-related
# settings, are defined here.   
#     To Change:                                    Modify this DEFINE below:
#     ----------------------                        ----------------------------
#     Cohda LLC version used                         LLC_VERSION
#     BSP to pull BSP libraries/headers              BSP_ROOT
#     Shared Memory Configuration(Model/Non-Model)   MODEL_BASED
#
#
# Some Options can be overridden on the command line (useful for compiling 
# individual components without having to edit v2xcommon.mk).  The following
# command line parameters can be used to override MAKE defines in this file:
#
#     To change this DEFINE  Add this to make command line   Valid Values
#     ---------------------  -----------------------------   -------------------
#     HWFLAG                 target=<target>                 vmware,mk5,poc,poc10,poc11
#     LLC_VERSION            llc=<llcversion>                120,130,15
#     MODEL_BASED            model=<on|off>                  on,off
#     SDHCAN		         sdhcan=<static|dynamic>         static,dynamic
#
#     examples:   make target=mk5 llc=120 
#                 make model=off install
#                 make sdhcan=dynamic install
#
# When you are writing your Makefiles, to test for hardware version, use
#    the "findstring" command against CFLAGS, combined with ifneq like this:
#           ifneq (,$(findstring V2XSP_HW_VERSION_MK5, $(CFLAGS)))
#
# In your C code, test for hardware definitions (from CFLAGS) like this:
#
#       #ifdef V2XSP_HW_VERSION_VMWARE
#       #if defined(V2XSP_HW_VERSION_MK5) || defined(V2XSP_HW_VERSION_VMWARE)
#     
#
# NOTE:  All makefiles must at a minimum have 3 rules clean, all, install
#        and install must have a dependency on all so that make install will
#        build all components as needed.
#

# PLATFORM HD RSU 
CFLAGS += -DPLATFORM_HD_RSU_5940 
# HDRD FEATURES: SNMP MIB support Yocto only.
CFLAGS += -DWEBGUI
#CFLAGS += -DSNORT_SECURITY
CFLAGS += -DCONFIGBLOB
CFLAGS += -DHSM
#Add Heavy Duty extras
CFLAGS += -DHEAVY_DUTY
CFLAGS += -DBSM_PSM_FORWARDING
# V2X_ROOT must be defined, by the Makefile importing this file, by the time we get here.  Throw error otherwise.
ifndef V2X_ROOT
    $(error ERROR: V2X_ROOT is not defined!  Aborting!)
endif

#Build Destination Path (for executables)
ifndef INSTALL_PATH
    export INSTALL_PATH = ${V2X_ROOT}/install
endif

#-- Shared Library search path
export LIB_PATH = $(V2X_ROOT)/install/lib

#Denso Official builds require firmware files to be signed
CFLAGS += -DDENSO_OFFICIAL
#From radio stack.
#CFLAGS += -DNO_SECURITY

#
#Common unit test defines
#
CUNIT_FLAGS = -Wall -Wextra -fprofile-arcs -ftest-coverage -DMY_UNIT_TEST
CUNIT_LIBS  = -lcunit -lpthread -lrt 

#
#Select one version of J2735 or if nothing default to 2023.
#
##J2735_2016 = 1
J2735_2023 = 1

ifdef $(J2735_2016)
CFLAGS += -DJ2735_2016
CUNIT_FLAGS += -DJ2735_2016
export ASN1_DIR       = $(V2X_ROOT)/fac/wmh/asn1/ppclib_2016
export J2735_UPER_DIR = $(V2X_ROOT)/fac/wmh/asn1/J2735_2016_03_UPER
else ifdef $(J2735_2023)
CFLAGS += -DJ2735_2023
CUNIT_FLAGS += -DJ2735_2023
export ASN1_DIR       = $(V2X_ROOT)/fac/wmh/asn1/ppclib_2023
export J2735_UPER_DIR = $(V2X_ROOT)/fac/wmh/asn1/J2735_2023_04_UPER
else
CFLAGS += -DJ2735_2023
CUNIT_FLAGS += -DJ2735_2023
export ASN1_DIR       = $(V2X_ROOT)/fac/wmh/asn1/ppclib_2023
export J2735_UPER_DIR = $(V2X_ROOT)/fac/wmh/asn1/J2735_2023_04_UPER
endif
export ASN1_LIB_DIR   = $(ASN1_DIR)/c/lib


