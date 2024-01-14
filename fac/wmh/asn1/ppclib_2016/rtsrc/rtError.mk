# makefile to compile rtError.c 
#
# On Windows execute 'nmake -f rtError.mk'
# On Linux/Mac/UNIX, exceute 'make -f rtError.mk'

include ../platform.mk

OSROOTDIR = ..
RTSRCDIR = $(OSROOTDIR)$(PS)rtsrc
RTXSRCDIR = $(OSROOTDIR)$(PS)rtxsrc

COMFLAGS = $(CVARSR_) $(MCFLAGS) $(COPTIMIZE_) -D_NO_LICENSE_CHECK
CFLAGS	 = $(COMFLAGS) $(CFLAGS_)

all : rtError$(OBJ)

rtError$(OBJ) : $(RTSRCDIR)$(PS)rtError.c $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1version.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(CFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)$(PS)rtError.c

clean:
	$(RM) rtError$(OBJ) 
