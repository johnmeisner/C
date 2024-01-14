# C/C++ to object file compilation rules

$(OBJDIR)$(PS)asn1CEvtHndlr$(OBJ): $(RTSRCDIR)$(PS)asn1CEvtHndlr.c $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)asn1CEvtHndlr.c


$(OBJDIR)$(PS)cerEncCanonicalSort$(OBJ): $(RTSRCDIR)$(PS)cerEncCanonicalSort.c \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxStreamMemory.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)cerEncCanonicalSort.c


$(OBJDIR)$(PS)rt16BitChars$(OBJ): $(RTSRCDIR)$(PS)rt16BitChars.c $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rt16BitChars.c


$(OBJDIR)$(PS)rt32BitChars$(OBJ): $(RTSRCDIR)$(PS)rt32BitChars.c $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rt32BitChars.c


$(OBJDIR)$(PS)rtBCD$(OBJ): $(RTSRCDIR)$(PS)rtBCD.c $(RTSRCDIR)$(PS)rtBCD.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxTBCD.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtBCD.c


$(OBJDIR)$(PS)rtcompare$(OBJ): $(RTSRCDIR)$(PS)rtcompare.c $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)rtPrint.h $(RTXSRCDIR)$(PS)rtxPrint.h \
 $(RTXSRCDIR)$(PS)rtxHexDump.h $(RTSRCDIR)$(PS)rtCompare.h $(RTSRCDIR)$(PS)rtconv.h \
 $(RTSRCDIR)$(PS)rtPrintToString.h $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtcompare.c


$(OBJDIR)$(PS)rtconv$(OBJ): $(RTSRCDIR)$(PS)rtconv.c $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTSRCDIR)$(PS)rtconv.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtconv.c


$(OBJDIR)$(PS)rtcopy$(OBJ): $(RTSRCDIR)$(PS)rtcopy.c $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)rtCopy.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtcopy.c


$(OBJDIR)$(PS)rtError$(OBJ): $(RTSRCDIR)$(PS)rtError.c $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtError.c


$(OBJDIR)$(PS)rtMakeGeneralizedTime$(OBJ): $(RTSRCDIR)$(PS)rtMakeGeneralizedTime.c \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.hh \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtMakeGeneralizedTime.c


$(OBJDIR)$(PS)rtMakeUTCTime$(OBJ): $(RTSRCDIR)$(PS)rtMakeUTCTime.c $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.hh $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtMakeUTCTime.c


$(OBJDIR)$(PS)rtNewContext$(OBJ): $(RTSRCDIR)$(PS)rtNewContext.c $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtNewContext.c


$(OBJDIR)$(PS)rtOID$(OBJ): $(RTSRCDIR)$(PS)rtOID.c $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtOID.c


$(OBJDIR)$(PS)rtParseGeneralizedTime$(OBJ): $(RTSRCDIR)$(PS)rtParseGeneralizedTime.c \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxDateTime.hh \
 $(RTXSRCDIR)$(PS)rtxDateTime.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtParseGeneralizedTime.c


$(OBJDIR)$(PS)rtParseUTCTime$(OBJ): $(RTSRCDIR)$(PS)rtParseUTCTime.c $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxDateTime.hh $(RTXSRCDIR)$(PS)rtxDateTime.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtParseUTCTime.c


$(OBJDIR)$(PS)rtPrint$(OBJ): $(RTSRCDIR)$(PS)rtPrint.c $(RTSRCDIR)$(PS)rtPrint.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPrint.h $(RTXSRCDIR)$(PS)rtxHexDump.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtPrint.c


$(OBJDIR)$(PS)rtPrintToStream$(OBJ): $(RTSRCDIR)$(PS)rtPrintToStream.c $(RTSRCDIR)$(PS)rtPrint.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPrint.h $(RTXSRCDIR)$(PS)rtxHexDump.h \
 $(RTSRCDIR)$(PS)rtPrintToStream.h $(RTXSRCDIR)$(PS)rtxPrintStream.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtPrintToStream.c


$(OBJDIR)$(PS)rtPrintToString$(OBJ): $(RTSRCDIR)$(PS)rtPrintToString.c $(RTSRCDIR)$(PS)rtPrintToString.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxPrint.h $(RTXSRCDIR)$(PS)rtxHexDump.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtPrintToString.c


$(OBJDIR)$(PS)rtStream$(OBJ): $(RTSRCDIR)$(PS)rtStream.c $(RTSRCDIR)$(PS)rtStream.h \
 $(RTXSRCDIR)$(PS)rtxStreamFile.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxStreamMemory.h \
 $(RTXSRCDIR)$(PS)rtxStreamSocket.h $(RTXSRCDIR)$(PS)rtxSocket.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTSRCDIR)$(PS)asn1compat.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtStream.c


$(OBJDIR)$(PS)rtTable$(OBJ): $(RTSRCDIR)$(PS)rtTable.c $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)rtTable.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtTable.c


$(OBJDIR)$(PS)rtUTF8StrToDynBitStr$(OBJ): $(RTSRCDIR)$(PS)rtUTF8StrToDynBitStr.c \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtUTF8StrToDynBitStr.c


$(OBJDIR)$(PS)rtValidateStr$(OBJ): $(RTSRCDIR)$(PS)rtValidateStr.c $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtValidateStr.c


$(OBJDIR)$(PS)rt3GPPTS32297$(OBJ): $(RTSRCDIR)$(PS)rt3GPPTS32297.c $(RTSRCDIR)$(PS)rt3GPPTS32297.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxIntDecode.h \
 $(RTXSRCDIR)$(PS)rtxIntEncode.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rt3GPPTS32297.c


$(OBJDIR)$(PS)rt3GPPTS32297PrtToStrm$(OBJ): $(RTSRCDIR)$(PS)rt3GPPTS32297PrtToStrm.c \
 $(RTSRCDIR)$(PS)rt3GPPTS32297.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTSRCDIR)$(PS)rtPrintToStream.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPrintStream.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rt3GPPTS32297PrtToStrm.c


$(LCOBJDIR)$(PS)rtContext$(OBJ): $(RTSRCDIR)$(PS)rtContext.c $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtContext.c


$(LCOBJDIR)$(PS)rtGetLibInfo$(OBJ): $(RTSRCDIR)$(PS)rtGetLibInfo.c $(RTSRCDIR)$(PS)asn1version.h \
 $(RTSRCDIR)$(PS)rtGetLibInfo.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)osSysTypes.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtGetLibInfo.c


$(OBJDIR)$(PS)asn1CppTypes$(OBJ): $(RTSRCDIR)$(PS)asn1CppTypes.cpp $(RTSRCDIR)$(PS)asn1CppTypes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.h \
 $(RTSRCDIR)$(PS)asn1CppEvtHndlr.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTSRCDIR)$(PS)ASN1Context.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBuf.h $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h \
 $(RTSRCDIR)$(PS)ASN1TOctStr.h $(RTSRCDIR)$(PS)ASN1TOctStr64.h $(RTSRCDIR)$(PS)ASN1TObjId.h \
 $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CCC) $(RTCFLAGS) $(RTCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)asn1CppTypes.cpp


$(OBJDIR)$(PS)asn1CppEvtHndlr$(OBJ): $(RTSRCDIR)$(PS)asn1CppEvtHndlr.cpp \
 $(RTSRCDIR)$(PS)asn1CppEvtHndlr.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CCC) $(RTCFLAGS) $(RTCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)asn1CppEvtHndlr.cpp


$(OBJDIR)$(PS)asn1CppEvtHndlr64$(OBJ): $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.cpp \
 $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.h $(RTSRCDIR)$(PS)asn1CppEvtHndlr.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CCC) $(RTCFLAGS) $(RTCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.cpp


$(OBJDIR)$(PS)ASN1CBitStr$(OBJ): $(RTSRCDIR)$(PS)ASN1CBitStr.cpp $(RTSRCDIR)$(PS)ASN1CBitStr.h \
 $(RTSRCDIR)$(PS)asn1CppTypes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.h \
 $(RTSRCDIR)$(PS)asn1CppEvtHndlr.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTSRCDIR)$(PS)ASN1Context.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBuf.h $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h \
 $(RTSRCDIR)$(PS)ASN1TOctStr.h $(RTSRCDIR)$(PS)ASN1TOctStr64.h $(RTSRCDIR)$(PS)ASN1TObjId.h \
 $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxUtil.h
	$(CCC) $(RTCFLAGS) $(RTCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)ASN1CBitStr.cpp


$(OBJDIR)$(PS)ASN1CSeqOfList$(OBJ): $(RTSRCDIR)$(PS)ASN1CSeqOfList.cpp $(RTSRCDIR)$(PS)ASN1CSeqOfList.h \
 $(RTSRCDIR)$(PS)asn1CppTypes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.h \
 $(RTSRCDIR)$(PS)asn1CppEvtHndlr.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTSRCDIR)$(PS)ASN1Context.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBuf.h $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h \
 $(RTSRCDIR)$(PS)ASN1TOctStr.h $(RTSRCDIR)$(PS)ASN1TOctStr64.h $(RTSRCDIR)$(PS)ASN1TObjId.h \
 $(RTSRCDIR)$(PS)asn1intl.h
	$(CCC) $(RTCFLAGS) $(RTCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)ASN1CSeqOfList.cpp


$(OBJDIR)$(PS)ASN1CTime$(OBJ): $(RTSRCDIR)$(PS)ASN1CTime.cpp $(RTSRCDIR)$(PS)ASN1CTime.h \
 $(RTSRCDIR)$(PS)asn1CppTypes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.h \
 $(RTSRCDIR)$(PS)asn1CppEvtHndlr.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTSRCDIR)$(PS)ASN1Context.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBuf.h $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h \
 $(RTSRCDIR)$(PS)ASN1TOctStr.h $(RTSRCDIR)$(PS)ASN1TOctStr64.h $(RTSRCDIR)$(PS)ASN1TObjId.h \
 $(RTSRCDIR)$(PS)ASN1TTime.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTSRCDIR)$(PS)asn1intl.h
	$(CCC) $(RTCFLAGS) $(RTCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)ASN1CTime.cpp


$(OBJDIR)$(PS)ASN1CGeneralizedTime$(OBJ): $(RTSRCDIR)$(PS)ASN1CGeneralizedTime.cpp \
 $(RTSRCDIR)$(PS)ASN1CGeneralizedTime.h $(RTSRCDIR)$(PS)ASN1CTime.h \
 $(RTSRCDIR)$(PS)asn1CppTypes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.h \
 $(RTSRCDIR)$(PS)asn1CppEvtHndlr.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTSRCDIR)$(PS)ASN1Context.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBuf.h $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h \
 $(RTSRCDIR)$(PS)ASN1TOctStr.h $(RTSRCDIR)$(PS)ASN1TOctStr64.h $(RTSRCDIR)$(PS)ASN1TObjId.h \
 $(RTSRCDIR)$(PS)ASN1TTime.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTSRCDIR)$(PS)asn1intl.h
	$(CCC) $(RTCFLAGS) $(RTCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)ASN1CGeneralizedTime.cpp


$(OBJDIR)$(PS)ASN1CUTCTime$(OBJ): $(RTSRCDIR)$(PS)ASN1CUTCTime.cpp $(RTSRCDIR)$(PS)ASN1CUTCTime.h \
 $(RTSRCDIR)$(PS)ASN1CTime.h $(RTSRCDIR)$(PS)asn1CppTypes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.h $(RTSRCDIR)$(PS)asn1CppEvtHndlr.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTSRCDIR)$(PS)ASN1Context.h $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)OSRTMsgBuf.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h $(RTSRCDIR)$(PS)ASN1TOctStr.h $(RTSRCDIR)$(PS)ASN1TOctStr64.h \
 $(RTSRCDIR)$(PS)ASN1TObjId.h $(RTSRCDIR)$(PS)ASN1TTime.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTSRCDIR)$(PS)asn1intl.h
	$(CCC) $(RTCFLAGS) $(RTCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)ASN1CUTCTime.cpp


$(OBJDIR)$(PS)ASN1TObjId$(OBJ): $(RTSRCDIR)$(PS)ASN1TObjId.cpp $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTSRCDIR)$(PS)ASN1TObjId.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CCC) $(RTCFLAGS) $(RTCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)ASN1TObjId.cpp


$(OBJDIR)$(PS)ASN1TObjId64$(OBJ): $(RTSRCDIR)$(PS)ASN1TObjId64.cpp $(RTSRCDIR)$(PS)ASN1TObjId64.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CCC) $(RTCFLAGS) $(RTCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)ASN1TObjId64.cpp


$(OBJDIR)$(PS)ASN1TOctStr$(OBJ): $(RTSRCDIR)$(PS)ASN1TOctStr.cpp $(RTSRCDIR)$(PS)ASN1TOctStr.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxPrint.h \
 $(RTXSRCDIR)$(PS)rtxHexDump.h
	$(CCC) $(RTCFLAGS) $(RTCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)ASN1TOctStr.cpp


$(OBJDIR)$(PS)ASN1TOctStr64$(OBJ): $(RTSRCDIR)$(PS)ASN1TOctStr64.cpp $(RTSRCDIR)$(PS)ASN1TOctStr64.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxPrint.h \
 $(RTXSRCDIR)$(PS)rtxHexDump.h
	$(CCC) $(RTCFLAGS) $(RTCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)ASN1TOctStr64.cpp


$(OBJDIR)$(PS)ASN1TTime$(OBJ): $(RTSRCDIR)$(PS)ASN1TTime.cpp $(RTSRCDIR)$(PS)ASN1TTime.h \
 $(RTSRCDIR)$(PS)asn1CppTypes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.h \
 $(RTSRCDIR)$(PS)asn1CppEvtHndlr.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTSRCDIR)$(PS)ASN1Context.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBuf.h $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h \
 $(RTSRCDIR)$(PS)ASN1TOctStr.h $(RTSRCDIR)$(PS)ASN1TOctStr64.h $(RTSRCDIR)$(PS)ASN1TObjId.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTSRCDIR)$(PS)asn1intl.h
	$(CCC) $(RTCFLAGS) $(RTCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)ASN1TTime.cpp


$(OBJDIR)$(PS)ASN1TGeneralizedTime$(OBJ): $(RTSRCDIR)$(PS)ASN1TGeneralizedTime.cpp \
 $(RTSRCDIR)$(PS)ASN1TTime.h $(RTSRCDIR)$(PS)asn1CppTypes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.h $(RTSRCDIR)$(PS)asn1CppEvtHndlr.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTSRCDIR)$(PS)ASN1Context.h $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)OSRTMsgBuf.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h $(RTSRCDIR)$(PS)ASN1TOctStr.h $(RTSRCDIR)$(PS)ASN1TOctStr64.h \
 $(RTSRCDIR)$(PS)ASN1TObjId.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTSRCDIR)$(PS)asn1intl.h
	$(CCC) $(RTCFLAGS) $(RTCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)ASN1TGeneralizedTime.cpp


$(OBJDIR)$(PS)ASN1TUTCTime$(OBJ): $(RTSRCDIR)$(PS)ASN1TUTCTime.cpp $(RTSRCDIR)$(PS)ASN1TTime.h \
 $(RTSRCDIR)$(PS)asn1CppTypes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.h \
 $(RTSRCDIR)$(PS)asn1CppEvtHndlr.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTSRCDIR)$(PS)ASN1Context.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBuf.h $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h \
 $(RTSRCDIR)$(PS)ASN1TOctStr.h $(RTSRCDIR)$(PS)ASN1TOctStr64.h $(RTSRCDIR)$(PS)ASN1TObjId.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTSRCDIR)$(PS)asn1intl.h
	$(CCC) $(RTCFLAGS) $(RTCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)ASN1TUTCTime.cpp


$(OBJDIR)$(PS)rtevalstub$(OBJ): $(RTSRCDIR)$(PS)rtevalstub.c $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxSList.h
	$(CC) $(RTCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTSRCDIR)$(PS)rtevalstub.c

