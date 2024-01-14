ifndef target
target=wsu
endif

ifeq ($(strip $(target)),wsu)
export prefix=/opt/mtwk/usr/local/powerpc-603e-linux/gcc-3.3.2-glibc-2.3.2
export cross_compile=powerpc-603e-linux-
else
WSUCFLAGS+=-Wno-pointer-sign
export prefix=/opt/powerpc-e300c3-linux-gnu
export cross_compile=/opt/powerpc-e300c3-linux-gnu/bin/powerpc-e300c3-linux-gnu-
endif

PATH:=${prefix}/bin:${PATH}
export WSUCC=${cross_compile}gcc
export WSUSTRIP=${cross_compile}strip

# host compilation variables
export GCC=gcc
export STRIP=strip

.PHONY: all dofirst obe scms clean

all: dofirst obe scms

dofirst:
	rm -f obe scms

obe:
	${WSUCC} $(WSUCFLAGS) -Wall -W -g -O2 obe.c -o obe
	${WSUSTRIP} obe

scms:
	${GCC} $(CFLAGS) -Wall -W -g -O2 scms.c -o scms
	${STRIP} scms

clean: dofirst

