# makefile to compile rtXmlError.c 
#
# On Windows execute 'nmake -f rtXmlError.mk'
# On Linux/Mac/UNIX, exceute 'make -f rtXmlError.mk'

include ../platform.mk

OSROOTDIR = ..
RTXSRCDIR = $(OSROOTDIR)$(PS)rtxsrc
XMLSRCDIR = $(OSROOTDIR)$(PS)rtxmlsrc

COMFLAGS = $(CVARSR_) $(MCFLAGS) $(COPTIMIZE_) -D_NO_LICENSE_CHECK
CFLAGS	 = $(COMFLAGS) $(CFLAGS_)
IPATHS	 = -I$(OSROOTDIR) -I$(RTXSRCDIR) $(IPATHS_)

all : rtXmlError$(OBJ)

rtXmlError$(OBJ) : $(XMLSRCDIR)$(PS)rtXmlError.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(CFLAGS) -c $(IPATHS) $(XMLSRCDIR)/rtXmlError.c

clean:
	$(RM) rtXmlError$(OBJ) 
