# makefile to compile rtxError.c 
#
# On Windows execute 'nmake -f rtxError.mk'
# On Linux/Mac/UNIX, exceute 'make -f rtxError.mk'

include ../platform.mk

OSROOTDIR = ..
RTXSRCDIR = $(OSROOTDIR)$(PS)rtxsrc

COMFLAGS = $(CVARSR_) $(MCFLAGS) $(COPTIMIZE_) -D_NO_LICENSE_CHECK
CFLAGS	 = $(COMFLAGS) $(CFLAGS_)
IPATHS	 = -I$(OSROOTDIR) -I$(RTXSRCDIR) $(IPATHS_)

all : rtxError$(OBJ)

rtxError$(OBJ) : $(RTXSRCDIR)$(PS)rtxError.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxUTF8.h
	$(CC) $(CFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxError.c

clean:
	$(RM) rtxError$(OBJ) 
