#
# This Makefile is for building the cv2x-example.c from the Autotalks Secton CV2X SDK
#

CFLAGS += -I/opt/autotalks/secton-5.17.0/include/atlk   # Needed bc includes verstring.h w/o atlk dir
CFLAGS += -I/opt/autotalks/secton-5.17.0/include
CFLAGS += -L/opt/autotalks/secton-5.17.0/lib

# XA's CFLAGS from sh_manual_make_apps.13.sh
XACFLAGS=-D__ATLK_CV2X -DSECTON -Wall -Werror -Wstrict-prototypes


LIBS=-lpthread -lrt
LIBS += -lcv2x      # libcv2x.so includes cv2x_service_get() and friends
LIBS += -lm         # for pow(), log(), log10f() needed by libcv2x.so
LIBS += -latlkremote_linux_u    # Provides ddm_service_get(), atlk_malloc()
LIBS += -lrefsys    # Needs to come before asn1c!
LIBS += -lasn1c
LIBS += -lpoti
LIBS += -ltommath

cv2x-example: cv2x-example.c
	$(CC) $(XACFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

install: cv2x-example
ifdef INSTALL_PATH
	@test -d $(INSTALL_PATH)/bin || mkdir -p $(INSTALL_PATH)/bin
	cp cv2x-example $(INSTALL_PATH)/bin
else
	@echo "ERROR: srv/radio/cv2xex-Makefile: Please define INSTALL_PATH before calling make install !"
	exit 1
endif

clean:
	rm -f cv2x-example
