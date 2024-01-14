asn1CEvtHndlr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(RTSRCDIR)/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(RTSRCDIR)/asn1CEvtHndlr.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/asn1CEvtHndlr.c

cerEncCanonicalSort$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(RTSRCDIR)/cerEncCanonicalSort.c \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxStreamMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/cerEncCanonicalSort.c

rt16BitChars$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(RTSRCDIR)/rt16BitChars.c \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rt16BitChars.c

rt32BitChars$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(RTSRCDIR)/rt32BitChars.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rt32BitChars.c

rt3GPPTS32297$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxReal.h \
    $(OSROOTDIR)/rtxsrc/rtxUtil.h \
    $(OSROOTDIR)/rtxsrc/rtxPattern.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxCommon.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxEnum.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxIntEncode.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(RTSRCDIR)/rt3GPPTS32297.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxIntDecode.h \
    $(OSROOTDIR)/rtxsrc/rtxCharStr.h \
    $(OSROOTDIR)/rtxsrc/rtxBigInt.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxFile.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(RTSRCDIR)/rt3GPPTS32297.c \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDiag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxDateTime.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rt3GPPTS32297.c

rt3GPPTS32297PrtToStrm$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtPrintToStream.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(RTSRCDIR)/rt3GPPTS32297.h \
    $(RTSRCDIR)/rt3GPPTS32297PrtToStrm.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxPrintStream.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxPrintToStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rt3GPPTS32297PrtToStrm.c

rtBCD$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtBCD.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(RTSRCDIR)/rtBCD.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxCtype.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxTBCD.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtBCD.c

rtcompare$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(RTSRCDIR)/rtPrint.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(RTSRCDIR)/asn1type.h \
    $(RTSRCDIR)/rtcompare.c \
    $(RTSRCDIR)/rtCompare.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxHexDump.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxPrint.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(RTSRCDIR)/rtconv.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(RTSRCDIR)/rtPrintToString.h \
    $(OSROOTDIR)/rtxsrc/rtxCharStr.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtcompare.c

rtContext$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(RTSRCDIR)/rtContext.c \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtContext.c

rtconv$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtconv.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(RTSRCDIR)/rtconv.c \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCharStr.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtconv.c

rtcopy$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCharStr.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtCopy.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(RTSRCDIR)/rtcopy.c \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtcopy.c

rtError$(OBJ): \
    $(RTSRCDIR)/rtError.c \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtError.c

rtevalstub$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(RTSRCDIR)/rtevalstub.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtevalstub.c

rtGetLibInfo$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1version.h \
    $(RTSRCDIR)/rtGetLibInfo.c \
    $(OSROOTDIR)/rtsrc/rtGetLibInfo.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtGetLibInfo.c

rtMakeGeneralizedTime$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(RTSRCDIR)/rtMakeGeneralizedTime.c \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxCtype.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxDateTime.hh \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCharStr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxDateTime.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtMakeGeneralizedTime.c

rtMakeUTCTime$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxCtype.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxDateTime.hh \
    $(RTSRCDIR)/rtMakeUTCTime.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCharStr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxDateTime.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtMakeUTCTime.c

rtMemNucleusDefs$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDiag.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxPrintStream.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemHeap.hh \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(RTSRCDIR)/rtMemNucleusDefs.c
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtMemNucleusDefs.c

rtNewContext$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(RTSRCDIR)/rtNewContext.c \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtNewContext.c

rtOID$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxCtype.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(RTSRCDIR)/rtOID.c
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtOID.c

rtParseGeneralizedTime$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(RTSRCDIR)/rtParseGeneralizedTime.c \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxDateTime.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxDateTime.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCtype.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtParseGeneralizedTime.c

rtParseUTCTime$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxCtype.h \
    $(RTSRCDIR)/rtParseUTCTime.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxDateTime.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxDateTime.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtParseUTCTime.c

rtPrint$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxCtype.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxHexDump.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(RTSRCDIR)/rtPrint.c \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/rtPrint.h \
    $(OSROOTDIR)/rtxsrc/rtxPrint.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtPrint.c

rtPrintToStream$(OBJ): \
    $(RTSRCDIR)/rtPrintToStream.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxCtype.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxHexDump.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtPrintToStream.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCharStr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxPrintStream.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxPrintToStream.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/rtPrint.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxPrint.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtPrintToStream.c

rtPrintToString$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxCtype.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxHexDump.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtPrintToString.h \
    $(OSROOTDIR)/rtxsrc/rtxCharStr.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(RTSRCDIR)/rtPrintToString.c \
    $(OSROOTDIR)/rtxsrc/rtxPrint.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtPrintToString.c

rtStream$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(RTSRCDIR)/rtStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSocket.h \
    $(OSROOTDIR)/rtxsrc/rtxStreamMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxStreamFile.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/asn1compat.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(RTSRCDIR)/rtStream.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStreamSocket.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtStream.c

rtTable$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtTable.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(RTSRCDIR)/rtTable.c \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtTable.c

rtUTF8StrToDynBitStr$(OBJ): \
    $(RTSRCDIR)/rtUTF8StrToDynBitStr.c \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtUTF8StrToDynBitStr.c

rtValidateStr$(OBJ): \
    $(RTSRCDIR)/asn1CharSet.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(RTSRCDIR)/rtValidateStr.c \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(RTSRCDIR)/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(RTCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(RTSRCDIR)/rtValidateStr.c

