#-------------------------------------------------------------------------------
# File: Source/C/srv/radio/test/Makefile
#
# Copyright (C) 2019 DENSO International America, Inc.
#
# Description: Makefile for Radio Services Test Programs
#------------------------------------------------------------------------------


ifndef V2X_ROOT
   V2X_ROOT=../../..
endif

include ${V2X_ROOT}/v2xcommon.mk

#---Directories
COMMON_DIR=../common
INC_DIR=../../../inc
PAL_DIR=../../../pal
RIS_DIR=../ris
TPS_DIR=../../tps
ALSMI_API_DIR=../alsmi_api
AEROLINK_DIR=../common/Aerolink/include/
VOD_DIR=../../../fac/vod

INC=-I. -I${INC_DIR} -I${COMMON_DIR} -I${PAL_DIR} -I${RIS_DIR} -I${AEROLINK_DIR}
LIBS=-L${PAL_DIR} -L${RIS_DIR} -L${ALSMI_API_DIR} -lris -lalsmi_api -lpal -lpthread -lrt
#V2X_RADIO_LIBS=-lv2x_radio -ltelux_cv2x -ltelux_qmi -ltelux_common -lqmiservices -lqmi_cci -lqmi_client_helper -ldsutils -ltelux_qmiloc -lqmi_encdec -lqmi_client_qmux -lqmi -lqmiidl
V2X_RADIO_LIBS=
#ALL_EXECUTABLES=wsm_send wsm_recv v2x_radio_tx v2x_radio_rx getRadioApiVersion serviceAvailableTest radioServicesTestCmd radio_tallies
ALL_EXECUTABLES=wsm_send wsm_recv radio_tallies

CFLAGS+=-Wall
CFLAGS+=$(LDFLAGS)    # For OpenEmbedded GNU_HASH in binary

LIB_DEPENDENCIES=$(PAL_DIR)/libpal.so $(RIS_DIR)/libris.so

.PHONY: all clean install

#---Default Rule:
all: ${ALL_EXECUTABLES}
	@echo Done ${ALL_EXECUTABLES}

wsm_send: wsm_send.c ${RIS_DIR}/type2str.c ${RIS_DIR}/ris.h ${RIS_DIR}/type2str.h ${COMMON_DIR}/ris_struct.h  ${INC_DIR}/dn_types.h ${LIB_DEPENDENCIES} ${TPS_DIR}/libtpsapi.so ${ALSMI_API_DIR}/libalsmi_api.so
	@echo "-----Building wsm_send-----"
	$(CC) ${CFLAGS} ${INC} -I${TPS_DIR} -o wsm_send wsm_send.c ${RIS_DIR}/type2str.c ${LIBS} -L${TPS_DIR} -ltpsapi

wsm_recv: wsm_recv.c ${RIS_DIR}/type2str.c ${RIS_DIR}/ris.h ${RIS_DIR}/type2str.h ${COMMON_DIR}/ris_struct.h  ${INC_DIR}/dn_types.h ${LIB_DEPENDENCIES} ${TPS_DIR}/libtpsapi.so ${ALSMI_API_DIR}/libalsmi_api.so ${VOD_DIR}/libvodapi.so
	@echo "-----Building wsm_recv-----"
	$(CC) ${CFLAGS} ${INC} -I${TPS_DIR} -I${VOD_DIR} -o wsm_recv wsm_recv.c ${RIS_DIR}/type2str.c ${LIBS} -L${TPS_DIR} -L${VOD_DIR} -ltpsapi -lm -lvodapi

v2x_radio_tx: v2x_radio_tx.c ${INC_DIR}/dn_types.h ${LIB_DEPENDENCIES}
	@echo "-----Building v2x_radio_tx-----"
	$(CC) ${CFLAGS} -I. -o v2x_radio_tx v2x_radio_tx.c ${V2X_RADIO_LIBS}

v2x_radio_rx: v2x_radio_rx.c ${INC_DIR}/dn_types.h ${LIB_DEPENDENCIES}
	@echo "-----Building v2x_radio_rx-----"
	$(CC) ${CFLAGS} -I. -o v2x_radio_rx v2x_radio_rx.c ${V2X_RADIO_LIBS}

getRadioApiVersion: getRadioApiVersion.c ${INC_DIR}/dn_types.h ${LIB_DEPENDENCIES}
	@echo "-----Building getRadioApiVersion-----"
	$(CC) ${CFLAGS} -I. -o getRadioApiVersion getRadioApiVersion.c ${V2X_RADIO_LIBS}

serviceAvailableTest: serviceAvailableTest.c ${RIS_DIR}/type2str.c ${PAL_DIR}/libpal.so ${RIS_DIR}/libris.so ${ALSMI_API_DIR}/libalsmi_api.so ${INC_DIR}/dn_types.h ${LIB_DEPENDENCIES}
	$(CC) $(CFLAGS) ${INC} -o serviceAvailableTest serviceAvailableTest.c ${RIS_DIR}/type2str.c ${LIBS}

radioServicesTestCmd: radioServicesTestCmd.c ${RIS_DIR}/type2str.c ${COMMON_DIR}/rs.h ${PAL_DIR}/ipcsock.h ${INC_DIR}/dn_types.h ${LIB_DEPENDENCIES}
	$(CC) $(CFLAGS) ${INC} -o radioServicesTestCmd radioServicesTestCmd.c ${RIS_DIR}/type2str.c ${LIBS}

radio_tallies: radio_tallies.c ${RIS_DIR}/type2str.c ${INC_DIR}/dn_types.h ${RIS_DIR}/type2str.h ${LIB_DEPENDENCIES}
	@echo "-----Building radio_tallies-----"
	$(CC) ${CFLAGS} ${INC} -o radio_tallies radio_tallies.c ${RIS_DIR}/type2str.c ${LIBS}

ipv6_user: ipv6_user.c ${RIS_DIR}/type2str.c ${RIS_DIR}/libipv6.c ${RIS_DIR}/libipv6.h ${RIS_DIR}/ris.h ${RIS_DIR}/type2str.h ${COMMON_DIR}/ris_struct.h  ${INC_DIR}/dn_types.h ${INC_DIR}/v2v_general.h ${LIB_DEPENDENCIES} ${TPS_DIR}/libtpsapi.so ${ALSMI_API_DIR}/libalsmi_api.so ${VOD_DIR}/libvodapi.so
	@echo "-----Building ipv6_user-----"
	$(CC) ${CFLAGS} ${INC} -I${TPS_DIR} -I${VOD_DIR} -o ipv6_user ipv6_user.c ${RIS_DIR}/type2str.c ${RIS_DIR}/libipv6.c ${LIBS} -L${TPS_DIR} -L${VOD_DIR} -ltpsapi -lm -lvodapi

${PAL_DIR}/libpal.so:
	make -C ${PAL_DIR}

${RIS_DIR}/libris.so:
	make -C ${RIS_DIR}

${ALSMI_API_DIR}/libalsmi_api.so:
	make -C ${ALSMI_API_DIR}

${TPS_DIR}/libtpsapi.so:
	make -C $(TPS_DIR) libtpsapi.so

${VOD_DIR}/libvodapi.so:
	make -C ${VOD_DIR} libvodapi.so

clean:
	@echo "Cleaning ${ALL_EXECUTABLES}"
	rm -rf ${ALL_EXECUTABLES}

install: all
ifdef INSTALL_PATH
	@test -d $(INSTALL_PATH)/bin || mkdir -p $(INSTALL_PATH)/bin
	cp $(ALL_EXECUTABLES) $(INSTALL_PATH)/bin
else
	@echo "ERROR: srv/radio/test-Makefile: Please define INSTALL_PATH before calling make install !"
	exit 1
endif

