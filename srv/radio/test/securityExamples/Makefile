ifndef V2X_ROOT
   V2X_ROOT=../../../..
endif

include ${V2X_ROOT}/v2xcommon.mk

ALL_EXECUTABLES=bin/txUnsecuredWSMsNoCallbacks bin/txUnsecuredWSMsWithCallbacks bin/txSignedWSMsNoCallbacks bin/txSignedWSMsWithCallbacks bin/txSignedAndUnsignedWSMsWithCallbacks bin/certChange bin/rxUnsecuredWSMs bin/rxSignedWSMsNoVerify bin/rxSignedWSMsWithVerifyVOD bin/rxSignedWSMsWithVerifyHybrid bin/rxSignedWSMsWithVerifyVTP bin/rxUnsecuredWSAs bin/rxSignedWSAsNoVerify bin/rxSignedWSAsWithVerifyNoCallbacks bin/rxSignedWSAsWithVerifyWithCallbacks bin/rxSignedWSAsWithVerifyWithCallbacks_two_uservices bin/wsmServiceOnly

.PHONY: all clean

all: $(ALL_EXECUTABLES)
	@echo Done $(ALL_EXECUTABLES)

bin/txUnsecuredWSMsNoCallbacks: txUnsecuredWSMsNoCallbacks.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -L../../../../pal -L../../ris -L../../alsmi_api -o bin/txUnsecuredWSMsNoCallbacks txUnsecuredWSMsNoCallbacks.c ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/txSignedWSMsNoCallbacks: txSignedWSMsNoCallbacks.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/txSignedWSMsNoCallbacks txSignedWSMsNoCallbacks.c ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/rxUnsecuredWSMs: rxUnsecuredWSMs.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/rxUnsecuredWSMs rxUnsecuredWSMs.c ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/rxSignedWSMsNoVerify: rxSignedWSMsNoVerify.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/rxSignedWSMsNoVerify rxSignedWSMsNoVerify.c ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/rxSignedWSMsWithVerifyVOD: rxSignedWSMsWithVerifyVOD.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so ../../../../fac/vod/libvodapi.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink/include -I../../../../fac/vod -L../../../../fac/vod -L../../../../pal -L../../ris -L../../alsmi_api -lvodapi -o bin/rxSignedWSMsWithVerifyVOD rxSignedWSMsWithVerifyVOD.c ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/rxSignedWSMsWithVerifyHybrid: rxSignedWSMsWithVerifyHybrid.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so ../../../../fac/vod/libvodapi.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink/include -I../../../../fac/vod -L../../../../fac/vod -L../../../../pal -L../../ris -L../../alsmi_api -lvodapi -o bin/rxSignedWSMsWithVerifyHybrid rxSignedWSMsWithVerifyHybrid.c ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/rxUnsecuredWSAs: rxUnsecuredWSAs.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -L../../../../pal -L../../ris -L../../alsmi_api -o bin/rxUnsecuredWSAs rxUnsecuredWSAs.c ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

ifeq (,$(findstring C_SAMPLE, $(CFLAGS)))
bin/txUnsecuredWSMsWithCallbacks: txUnsecuredWSMsWithCallbacks.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/txUnsecuredWSMsWithCallbacks txUnsecuredWSMsWithCallbacks.c ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/txSignedWSMsWithCallbacks: txSignedWSMsWithCallbacks.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/txSignedWSMsWithCallbacks txSignedWSMsWithCallbacks.c  ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/txSignedAndUnsignedWSMsWithCallbacks: txSignedAndUnsignedWSMsWithCallbacks.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/txSignedAndUnsignedWSMsWithCallbacks txSignedAndUnsignedWSMsWithCallbacks.c  ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/certChange: certChange.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/certChange certChange.c  ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/rxSignedWSMsWithVerifyVTP: rxSignedWSMsWithVerifyVTP.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/rxSignedWSMsWithVerifyVTP rxSignedWSMsWithVerifyVTP.c  ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/rxSignedWSAsNoVerify: rxSignedWSAsNoVerify.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/rxSignedWSAsNoVerify rxSignedWSAsNoVerify.c ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/rxSignedWSAsWithVerifyNoCallbacks: rxSignedWSAsWithVerifyNoCallbacks.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/rxSignedWSAsWithVerifyNoCallbacks rxSignedWSAsWithVerifyNoCallbacks.c ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/rxSignedWSAsWithVerifyWithCallbacks: rxSignedWSAsWithVerifyWithCallbacks.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/rxSignedWSAsWithVerifyWithCallbacks rxSignedWSAsWithVerifyWithCallbacks.c ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/rxSignedWSAsWithVerifyWithCallbacks_two_uservices: rxSignedWSAsWithVerifyWithCallbacks_two_uservices.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/rxSignedWSAsWithVerifyWithCallbacks_two_uservices rxSignedWSAsWithVerifyWithCallbacks_two_uservices.c ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/wsmServiceOnly: wsmServiceOnly.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/wsmServiceOnly wsmServiceOnly.c  ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt
else
bin/txUnsecuredWSMsWithCallbacks: txUnsecuredWSMsWithCallbacks.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink_C_sample/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/txUnsecuredWSMsWithCallbacks txUnsecuredWSMsWithCallbacks.c ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/txSignedWSMsWithCallbacks: txSignedWSMsWithCallbacks.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink_C_sample/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/txSignedWSMsWithCallbacks txSignedWSMsWithCallbacks.c  ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/txSignedAndUnsignedWSMsWithCallbacks: txSignedAndUnsignedWSMsWithCallbacks.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink_C_sample/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/txSignedAndUnsignedWSMsWithCallbacks txSignedAndUnsignedWSMsWithCallbacks.c  ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/certChange: certChange.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink_C_sample/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/certChange certChange.c  ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/rxSignedWSMsWithVerifyVTP: rxSignedWSMsWithVerifyVTP.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink_C_sample/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/rxSignedWSMsWithVerifyVTP rxSignedWSMsWithVerifyVTP.c  ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/rxSignedWSAsNoVerify: rxSignedWSAsNoVerify.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink_C_sample/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/rxSignedWSAsNoVerify rxSignedWSAsNoVerify.c ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/rxSignedWSAsWithVerifyNoCallbacks: rxSignedWSAsWithVerifyNoCallbacks.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink_C_sample/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/rxSignedWSAsWithVerifyNoCallbacks rxSignedWSAsWithVerifyNoCallbacks.c ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/rxSignedWSAsWithVerifyWithCallbacks: rxSignedWSAsWithVerifyWithCallbacks.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink_C_sample/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/rxSignedWSAsWithVerifyWithCallbacks rxSignedWSAsWithVerifyWithCallbacks.c ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/rxSignedWSAsWithVerifyWithCallbacks_two_uservices: rxSignedWSAsWithVerifyWithCallbacks_two_uservices.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink_C_sample/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/rxSignedWSAsWithVerifyWithCallbacks_two_uservices rxSignedWSAsWithVerifyWithCallbacks_two_uservices.c ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt

bin/wsmServiceOnly: wsmServiceOnly.c ../../ris/type2str.c param.h ../../../../pal/libpal.so ../../ris/libris.so ../../alsmi_api/libalsmi_api.so
	$(CC) $(CFLAGS) -I../../ris -I../../common -I../../../../inc -I../../../../pal -I../../common/Aerolink_C_sample/include -L../../../../pal/ -L../../ris -L../../alsmi_api/ -o bin/wsmServiceOnly wsmServiceOnly.c  ../../ris/type2str.c -lpal -lris -lalsmi_api -lpthread -lrt
endif

../../../../pal/libpal.so:
	make -C ../../../../pal

../../../../fac/vod/libvodapi.so:
	make -C ../../../../fac/vod libvodapi.so

../../ris/libris.so:
	make -C ../../ris libris.so

../../alsmi_api/libalsmi_api.so:
	make -C ../../alsmi_api libalsmi_api.so

clean:
	@echo "Cleaning bin/*"
	rm -rf $(ALL_EXECUTABLES)

