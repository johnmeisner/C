#-------------------------------------------------------------------------------
# File:
#
# Copyright (C) 2020 DENSO International America, Inc.
#
# Description:
#------------------------------------------------------------------------------

####################
# Heavy Duty offsets
####################

HEAVYDUTY_OFFSET_C       =../..
HEAVYDUTY_OFFSET_C_FAC   =../../fac

#########
# At ./C
#########
ifndef V2X_ROOT
   V2X_ROOT=$(HEAVYDUTY_OFFSET_C)
endif

include $(V2X_ROOT)/v2xcommon.mk


CFLAGS += -Wall -D_OPTIMIZED -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DCPP

CFLAGS += -DHEAVY_DUTY

BINS =  denso_nor_lock denso_nor_unlock denso_nor_write denso_nor_read nor_write_test lpwr_daemon

.PHONY: all docs clean install mylint myunit

all: $(BINS)


#denso_nor_lock.o: denso_nor_lock.c 
#	$(CC) $(CFLAGS) $(INCS) $(INCS2) -c denso_nor_lock.c

#denso_nor_unlock.o: denso_nor_unlock.c 
#	$(CC) $(CFLAGS) $(INCS) $(INCS2) -c denso_nor_unlock.c

clean:
	rm -f *.o $(BINS) *.tmp *.gch
	rm -f ./executable
	rm -f ./*.gcov
	rm -f ./*.gcda
	rm -f ./*.gcno

#---Install into INSTALL_PATH.
install: all
ifdef INSTALL_PATH
	@echo "denso_nor: make install... "
	@test -d $(INSTALL_PATH) || (mkdir $(INSTALL_PATH); echo "*---Creating Installation Directory ($(INSTALL_PATH)).")
	@test -d $(INSTALL_PATH)/bin || (mkdir $(INSTALL_PATH)/bin; echo "*---Creating Installation Directory ($(INSTALL_PATH)/v2x).")
	cp $(BINS) $(INSTALL_PATH)/bin
else
	@echo "ERROR: i2v/srmrx-Makefile: Please define INSTALL_PATH before calling make install !"
	exit 1
endif

myunit:
	gcc -c -Wall -Wextra  -fPIC  -fprofile-arcs -ftest-coverage  $(CFLAGS) $(INCS) $(INCS2) -I. -DMY_UNIT_TEST -DWSU_LITTLE_ENDIAN srm_rx.c
	gcc -c -Wall -Wextra  -fPIC  -fprofile-arcs -ftest-coverage  $(CFLAGS) $(INCS) $(INCS2) -I. -DMY_UNIT_TEST -DWSU_LITTLE_ENDIAN ../util/i2v_util.c
	gcc -c -Wall -Wextra  -fPIC  -fprofile-arcs -ftest-coverage  $(CFLAGS) $(INCS) $(INCS2) -I. -DMY_UNIT_TEST -DWSU_LITTLE_ENDIAN ../stubs/stubs.c
	gcc    -Wall -fprofile-arcs -ftest-coverage -DMY_UNIT_TEST $(CFLAGS) $(INCS) $(INCS2)   -I. -I../stubs test.c  -lcunit  ./stubs.o ./srm_rx.o ./i2v_util.o -o executable

mylint:
	splint srm_rx.c i2v_util.c +showsummary  $(CFLAGS) $(INCS) $(INCS2)
