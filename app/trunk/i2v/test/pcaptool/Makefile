ifndef target
target=wsu
endif

ifeq ($(strip $(target)),wsu)
export prefix=/opt/mtwk/usr/local/powerpc-603e-linux/gcc-3.3.2-glibc-2.3.2
export cross_compile=powerpc-603e-linux-
else
WSUCFLAGS+=-Wno-pointer-sign -DWSU_5001
export prefix=/opt/powerpc-e300c3-linux-gnu
export cross_compile=/opt/powerpc-e300c3-linux-gnu/bin/powerpc-e300c3-linux-gnu-
endif

PATH:=${prefix}/bin:${PATH}
export WSUCC=${cross_compile}gcc
export WSUSTRIP=${cross_compile}strip

# host compilation variables
export GCC=gcc
export STRIP=strip

# this file belongs in /your svn root/I2V/test/pcaptool
# if you move the file, you MUST change the lines below
export SVNROOT=../../..
export WSUPATH=${SVNROOT}/WSUServices
export WSUINC=-I${WSUPATH}/Common/wsu_include

ifeq ($(strip $(target)),wsu)
export WSUAPI=-I${WSUPATH}/wsu_services_api/include
export WSUAPILIB=-L${WSUPATH}/wsu_services_api/lib
export SMILIB=-L${WSUPATH}/esbsmi
export RISLIB=-lris
export RISSMI=-lrissmi
export TPSLIB=-ltpsapi -lipc -lrt
export ESBSMI=-lesbsmi
else
export WSUAPI=-I${WSUPATH}/wsu_services_api/include -I${WSUPATH}/Common/WSU-5001_include
export WSUAPILIB=-L${WSUPATH}/wsu_services_api/lib/WSU-5001 -L${WSUPATH}/wsu_services_api/lib/WSU-5001_prebuilt
export SMILIB=${WSUAPILIB}
export RISLIB=-ldot2 -ldot3 -lm -lcrypto -lgps -lrisapi -lsr_ris
export RISSMI=-ldot2 -ldot3 -lm -lcrypto -lgps -lrisapi -lsr_ris
export TPSLIB=-ltpsapi -lipc -lrt
export ESBSMI=-lesbsmi
endif

.PHONY: all dofirst build clean

all: dofirst build

dofirst:
	rm -f resend

build:
	${WSUCC} $(WSUCFLAGS) -Wall -W -g -O2 ${WSUINC} ${WSUAPI} pcapreplay.c ${WSUAPILIB} ${RISSMI} ${TPSLIB} ${SMILIB} ${ESBSMI} -o resend
	${WSUSTRIP} resend

clean: dofirst

