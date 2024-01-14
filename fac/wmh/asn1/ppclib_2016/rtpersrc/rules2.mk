# C/C++ to object file compilation rules

$(OBJDIR)$(PS)pd_16BitConstrainedString$(OBJ): $(PERSRCDIR)$(PS)pd_16BitConstrainedString.c \
 $(PERSRCDIR)$(PS)pd_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_16BitConstrainedString.c


$(OBJDIR)$(PS)pd_32BitConstrainedString$(OBJ): $(PERSRCDIR)$(PS)pd_32BitConstrainedString.c \
 $(PERSRCDIR)$(PS)pd_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_32BitConstrainedString.c


$(OBJDIR)$(PS)pd_BigInteger$(OBJ): $(PERSRCDIR)$(PS)pd_BigInteger.c $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_BigInteger.c


$(OBJDIR)$(PS)pd_BigIntegerEx$(OBJ): $(PERSRCDIR)$(PS)pd_BigIntegerEx.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_BigIntegerEx.c


$(OBJDIR)$(PS)pd_BitString$(OBJ): $(PERSRCDIR)$(PS)pd_BitString.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_BitString.c


$(OBJDIR)$(PS)pd_BMPString$(OBJ): $(PERSRCDIR)$(PS)pd_BMPString.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_BMPString.c


$(OBJDIR)$(PS)pd_UniversalString$(OBJ): $(PERSRCDIR)$(PS)pd_UniversalString.c \
 $(PERSRCDIR)$(PS)pd_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_UniversalString.c


$(OBJDIR)$(PS)pd_ChoiceOpenTypeExt$(OBJ): $(PERSRCDIR)$(PS)pd_ChoiceOpenTypeExt.c \
 $(PERSRCDIR)$(PS)pd_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_ChoiceOpenTypeExt.c


$(OBJDIR)$(PS)pd_common$(OBJ): $(PERSRCDIR)$(PS)pd_common.c $(RTSRCDIR)$(PS)asn1intl.h \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_common.c


$(OBJDIR)$(PS)pd_ConsWholeNumber64$(OBJ): $(PERSRCDIR)$(PS)pd_ConsWholeNumber64.c \
 $(PERSRCDIR)$(PS)pd_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_ConsWholeNumber64.c


$(OBJDIR)$(PS)pd_ConsInt8$(OBJ): $(PERSRCDIR)$(PS)pd_ConsInt8.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_ConsInt8.c


$(OBJDIR)$(PS)pd_ConsInt16$(OBJ): $(PERSRCDIR)$(PS)pd_ConsInt16.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_ConsInt16.c


$(OBJDIR)$(PS)pd_ConsInt64$(OBJ): $(PERSRCDIR)$(PS)pd_ConsInt64.c $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDiag.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_ConsInt64.c


$(OBJDIR)$(PS)pd_ConsUInt8$(OBJ): $(PERSRCDIR)$(PS)pd_ConsUInt8.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_ConsUInt8.c


$(OBJDIR)$(PS)pd_ConsUInt16$(OBJ): $(PERSRCDIR)$(PS)pd_ConsUInt16.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_ConsUInt16.c


$(OBJDIR)$(PS)pd_ConsUInt64$(OBJ): $(PERSRCDIR)$(PS)pd_ConsUInt64.c $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_ConsUInt64.c


$(OBJDIR)$(PS)pd_DateStr$(OBJ): $(PERSRCDIR)$(PS)pd_DateStr.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_DateStr.c


$(OBJDIR)$(PS)pd_DateTimeStr$(OBJ): $(PERSRCDIR)$(PS)pd_DateTimeStr.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_DateTimeStr.c


$(OBJDIR)$(PS)pd_deprecated$(OBJ): $(PERSRCDIR)$(PS)pd_deprecated.c $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_deprecated.c


$(OBJDIR)$(PS)pd_Duration$(OBJ): $(PERSRCDIR)$(PS)pd_Duration.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_Duration.c


$(OBJDIR)$(PS)pd_DynBitString$(OBJ): $(PERSRCDIR)$(PS)pd_DynBitString.c $(PERSRCDIR)$(PS)pu_common.hh \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_DynBitString.c


$(OBJDIR)$(PS)pd_extension$(OBJ): $(PERSRCDIR)$(PS)pd_extension.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_extension.c


$(OBJDIR)$(PS)pd_Interval$(OBJ): $(PERSRCDIR)$(PS)pd_Interval.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_Interval.c


$(OBJDIR)$(PS)pd_oid64$(OBJ): $(PERSRCDIR)$(PS)pd_oid64.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_oid64.c


$(OBJDIR)$(PS)pd_RelativeOID$(OBJ): $(PERSRCDIR)$(PS)pd_RelativeOID.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_RelativeOID.c


$(OBJDIR)$(PS)pd_OpenType$(OBJ): $(PERSRCDIR)$(PS)pd_OpenType.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_OpenType.c


$(OBJDIR)$(PS)pd_OpenTypeExt$(OBJ): $(PERSRCDIR)$(PS)pd_OpenTypeExt.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_OpenTypeExt.c


$(OBJDIR)$(PS)pd_Real$(OBJ): $(PERSRCDIR)$(PS)pd_Real.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxReal.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_Real.c


$(OBJDIR)$(PS)pd_SemiConsInteger$(OBJ): $(PERSRCDIR)$(PS)pd_SemiConsInteger.c \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(PERSRCDIR)$(PS)pd_UnconsInteger.c
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_SemiConsInteger.c


$(OBJDIR)$(PS)pd_SemiConsUnsigned$(OBJ): $(PERSRCDIR)$(PS)pd_SemiConsUnsigned.c \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDiag.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_SemiConsUnsigned.c


$(OBJDIR)$(PS)pd_SemiConsInt64$(OBJ): $(PERSRCDIR)$(PS)pd_SemiConsInt64.c $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDiag.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_SemiConsInt64.c


$(OBJDIR)$(PS)pd_SemiConsUInt64$(OBJ): $(PERSRCDIR)$(PS)pd_SemiConsUInt64.c \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDiag.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_SemiConsUInt64.c


$(OBJDIR)$(PS)pd_TimeDiff$(OBJ): $(PERSRCDIR)$(PS)pd_TimeDiff.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_TimeDiff.c


$(OBJDIR)$(PS)pd_TimeStr$(OBJ): $(PERSRCDIR)$(PS)pd_TimeStr.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_TimeStr.c


$(OBJDIR)$(PS)pd_VarWidthCharString$(OBJ): $(PERSRCDIR)$(PS)pd_VarWidthCharString.c \
 $(PERSRCDIR)$(PS)pd_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_VarWidthCharString.c


$(OBJDIR)$(PS)pd_Year$(OBJ): $(PERSRCDIR)$(PS)pd_Year.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_Year.c


$(OBJDIR)$(PS)pu_16BitCharSet$(OBJ): $(PERSRCDIR)$(PS)pu_16BitCharSet.c $(PERSRCDIR)$(PS)pu_common.hh \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pu_16BitCharSet.c


$(OBJDIR)$(PS)pu_32BitCharSet$(OBJ): $(PERSRCDIR)$(PS)pu_32BitCharSet.c $(PERSRCDIR)$(PS)pu_common.hh \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pu_32BitCharSet.c


$(OBJDIR)$(PS)pe_16BitConstrainedString$(OBJ): $(PERSRCDIR)$(PS)pe_16BitConstrainedString.c \
 $(PERSRCDIR)$(PS)pe_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_16BitConstrainedString.c


$(OBJDIR)$(PS)pe_32BitConstrainedString$(OBJ): $(PERSRCDIR)$(PS)pe_32BitConstrainedString.c \
 $(PERSRCDIR)$(PS)pe_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_32BitConstrainedString.c


$(OBJDIR)$(PS)pe_2sCompBinInt$(OBJ): $(PERSRCDIR)$(PS)pe_2sCompBinInt.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_2sCompBinInt.c


$(OBJDIR)$(PS)pe_2sCompBinInt64$(OBJ): $(PERSRCDIR)$(PS)pe_2sCompBinInt64.c \
 $(PERSRCDIR)$(PS)pe_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_2sCompBinInt64.c


$(OBJDIR)$(PS)pe_BigInteger$(OBJ): $(PERSRCDIR)$(PS)pe_BigInteger.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxBigInt.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_BigInteger.c


$(OBJDIR)$(PS)pe_BitString$(OBJ): $(PERSRCDIR)$(PS)pe_BitString.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_BitString.c


$(OBJDIR)$(PS)pe_BMPString$(OBJ): $(PERSRCDIR)$(PS)pe_BMPString.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_BMPString.c


$(OBJDIR)$(PS)pe_UniversalString$(OBJ): $(PERSRCDIR)$(PS)pe_UniversalString.c \
 $(PERSRCDIR)$(PS)pe_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_UniversalString.c


$(OBJDIR)$(PS)pe_ChoiceTypeExt$(OBJ): $(PERSRCDIR)$(PS)pe_ChoiceTypeExt.c \
 $(PERSRCDIR)$(PS)pe_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_ChoiceTypeExt.c


$(OBJDIR)$(PS)pe_common64$(OBJ): $(PERSRCDIR)$(PS)pe_common64.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_common64.c


$(OBJDIR)$(PS)pe_ConsWholeNumber64$(OBJ): $(PERSRCDIR)$(PS)pe_ConsWholeNumber64.c \
 $(PERSRCDIR)$(PS)pe_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_ConsWholeNumber64.c


$(OBJDIR)$(PS)pe_ConsInt64$(OBJ): $(PERSRCDIR)$(PS)pe_ConsInt64.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_ConsInt64.c


$(OBJDIR)$(PS)pe_ConstrainedString$(OBJ): $(PERSRCDIR)$(PS)pe_ConstrainedString.c \
 $(PERSRCDIR)$(PS)pe_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_ConstrainedString.c


$(OBJDIR)$(PS)pe_ConsUInt64$(OBJ): $(PERSRCDIR)$(PS)pe_ConsUInt64.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_ConsUInt64.c


$(OBJDIR)$(PS)pe_DateStr$(OBJ): $(PERSRCDIR)$(PS)pe_DateStr.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDateTime.hh \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_DateStr.c


$(OBJDIR)$(PS)pe_DateTimeStr$(OBJ): $(PERSRCDIR)$(PS)pe_DateTimeStr.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDateTime.hh \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_DateTimeStr.c


$(OBJDIR)$(PS)pe_Duration$(OBJ): $(PERSRCDIR)$(PS)pe_Duration.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDateTime.hh \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_Duration.c


$(OBJDIR)$(PS)pe_extension$(OBJ): $(PERSRCDIR)$(PS)pe_extension.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_extension.c


$(OBJDIR)$(PS)pe_identifier$(OBJ): $(PERSRCDIR)$(PS)pe_identifier.c $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxUtil.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_identifier.c


$(OBJDIR)$(PS)pe_identifier64$(OBJ): $(PERSRCDIR)$(PS)pe_identifier64.c $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxUtil.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_identifier64.c


$(OBJDIR)$(PS)pe_Interval$(OBJ): $(PERSRCDIR)$(PS)pe_Interval.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDateTime.hh \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_Interval.c


$(OBJDIR)$(PS)pe_NonNegBinInt$(OBJ): $(PERSRCDIR)$(PS)pe_NonNegBinInt.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_NonNegBinInt.c


$(OBJDIR)$(PS)pe_NonNegBinInt64$(OBJ): $(PERSRCDIR)$(PS)pe_NonNegBinInt64.c \
 $(PERSRCDIR)$(PS)pe_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_NonNegBinInt64.c


$(OBJDIR)$(PS)pe_oid64$(OBJ): $(PERSRCDIR)$(PS)pe_oid64.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxUtil.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_oid64.c


$(OBJDIR)$(PS)pe_OpenType$(OBJ): $(PERSRCDIR)$(PS)pe_OpenType.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_OpenType.c


$(OBJDIR)$(PS)pe_OpenTypeExt$(OBJ): $(PERSRCDIR)$(PS)pe_OpenTypeExt.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_OpenTypeExt.c


$(OBJDIR)$(PS)pe_Real$(OBJ): $(PERSRCDIR)$(PS)pe_Real.c $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_Real.c


$(OBJDIR)$(PS)pe_RelativeOID$(OBJ): $(PERSRCDIR)$(PS)pe_RelativeOID.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxUtil.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_RelativeOID.c


$(OBJDIR)$(PS)pe_SemiConsInteger$(OBJ): $(PERSRCDIR)$(PS)pe_SemiConsInteger.c \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDiag.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_SemiConsInteger.c


$(OBJDIR)$(PS)pe_SemiConsInt64$(OBJ): $(PERSRCDIR)$(PS)pe_SemiConsInt64.c $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDiag.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_SemiConsInt64.c


$(OBJDIR)$(PS)pe_SemiConsUnsigned$(OBJ): $(PERSRCDIR)$(PS)pe_SemiConsUnsigned.c \
 $(PERSRCDIR)$(PS)pe_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_SemiConsUnsigned.c


$(OBJDIR)$(PS)pe_SemiConsUInt64$(OBJ): $(PERSRCDIR)$(PS)pe_SemiConsUInt64.c \
 $(PERSRCDIR)$(PS)pe_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDiag.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_SemiConsUInt64.c


$(OBJDIR)$(PS)pe_TimeDiff$(OBJ): $(PERSRCDIR)$(PS)pe_TimeDiff.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDateTime.hh \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_TimeDiff.c


$(OBJDIR)$(PS)pe_TimeStr$(OBJ): $(PERSRCDIR)$(PS)pe_TimeStr.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDateTime.hh \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_TimeStr.c


$(OBJDIR)$(PS)pe_VarWidthCharString$(OBJ): $(PERSRCDIR)$(PS)pe_VarWidthCharString.c \
 $(PERSRCDIR)$(PS)pe_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_VarWidthCharString.c


$(OBJDIR)$(PS)pe_Year$(OBJ): $(PERSRCDIR)$(PS)pe_Year.c $(PERSRCDIR)$(PS)pe_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDateTime.hh \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_Year.c


$(OBJDIR)$(PS)pu_addSizeConstraint$(OBJ): $(PERSRCDIR)$(PS)pu_addSizeConstraint.c \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pu_addSizeConstraint.c


$(OBJDIR)$(PS)pu_alignCharStr$(OBJ): $(PERSRCDIR)$(PS)pu_alignCharStr.c $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pu_alignCharStr.c


$(OBJDIR)$(PS)pu_checkSize$(OBJ): $(PERSRCDIR)$(PS)pu_checkSize.c $(PERSRCDIR)$(PS)pu_common.hh \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pu_checkSize.c


$(OBJDIR)$(PS)pu_common$(OBJ): $(PERSRCDIR)$(PS)pu_common.c $(PERSRCDIR)$(PS)pu_common.hh \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxUtil.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pu_common.c


$(OBJDIR)$(PS)pu_DateStr$(OBJ): $(PERSRCDIR)$(PS)pu_DateStr.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pu_DateStr.c


$(OBJDIR)$(PS)pu_deprecated$(OBJ): $(PERSRCDIR)$(PS)pu_deprecated.c $(PERSRCDIR)$(PS)pu_common.hh \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pu_deprecated.c


$(OBJDIR)$(PS)pu_dump$(OBJ): $(PERSRCDIR)$(PS)pu_dump.c $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxPrintStream.h $(RTXSRCDIR)$(PS)rtxStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pu_dump.c


$(OBJDIR)$(PS)pu_field$(OBJ): $(PERSRCDIR)$(PS)pu_field.c $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pu_field.c


$(OBJDIR)$(PS)pu_insLenField$(OBJ): $(PERSRCDIR)$(PS)pu_insLenField.c $(PERSRCDIR)$(PS)pu_common.hh \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pu_insLenField.c


$(OBJDIR)$(PS)pu_internal$(OBJ): $(PERSRCDIR)$(PS)pu_internal.c $(PERSRCDIR)$(PS)pu_common.hh \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pu_internal.c


$(OBJDIR)$(PS)pu_setCharSet$(OBJ): $(PERSRCDIR)$(PS)pu_setCharSet.c $(PERSRCDIR)$(PS)pu_common.hh \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pu_setCharSet.c


$(OBJDIR)$(PS)pu_setFldBitCount$(OBJ): $(PERSRCDIR)$(PS)pu_setFldBitCount.c \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pu_setFldBitCount.c


$(OBJDIR)$(PS)pu_setFldBitOffset$(OBJ): $(PERSRCDIR)$(PS)pu_setFldBitOffset.c \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pu_setFldBitOffset.c


$(OBJDIR)$(PS)pu_GetLibInfo$(OBJ): $(PERSRCDIR)$(PS)pu_GetLibInfo.c $(RTSRCDIR)$(PS)asn1version.h \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pu_GetLibInfo.c


$(OBJDIR)$(PS)perDecDecimal$(OBJ): $(PERSRCDIR)$(PS)perDecDecimal.c $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxDecimal.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)perDecDecimal.c


$(OBJDIR)$(PS)perDecReal10$(OBJ): $(PERSRCDIR)$(PS)perDecReal10.c $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)perDecReal10.c


$(OBJDIR)$(PS)perEncReal10$(OBJ): $(PERSRCDIR)$(PS)perEncReal10.c $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)perEncReal10.c


$(LCOBJDIR)$(PS)pd_ConsInteger$(OBJ): $(PERSRCDIR)$(PS)pd_ConsInteger.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_ConsInteger.c


$(LCOBJDIR)$(PS)pd_ConsUnsigned$(OBJ): $(PERSRCDIR)$(PS)pd_ConsUnsigned.c $(PERSRCDIR)$(PS)pd_common.hh \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_ConsUnsigned.c


$(LCOBJDIR)$(PS)pd_ConstrainedString$(OBJ): $(PERSRCDIR)$(PS)pd_ConstrainedString.c \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_ConstrainedString.c


$(LCOBJDIR)$(PS)pd_DynOctetString$(OBJ): $(PERSRCDIR)$(PS)pd_DynOctetString.c \
 $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_DynOctetString.c


$(LCOBJDIR)$(PS)pd_ObjectIdentifier$(OBJ): $(PERSRCDIR)$(PS)pd_ObjectIdentifier.c \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_ObjectIdentifier.c


$(LCOBJDIR)$(PS)pd_OctetString$(OBJ): $(PERSRCDIR)$(PS)pd_OctetString.c $(PERSRCDIR)$(PS)pu_common.hh \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pd_OctetString.c


$(LCOBJDIR)$(PS)pe_common$(OBJ): $(PERSRCDIR)$(PS)pe_common.c $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_common.c


$(LCOBJDIR)$(PS)pe_ObjectIdentifier$(OBJ): $(PERSRCDIR)$(PS)pe_ObjectIdentifier.c \
 $(PERSRCDIR)$(PS)pe_common.hh $(PERSRCDIR)$(PS)pu_common.hh $(PERSRCDIR)$(PS)asn1per.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_ObjectIdentifier.c


$(LCOBJDIR)$(PS)pe_OctetString$(OBJ): $(PERSRCDIR)$(PS)pe_OctetString.c $(PERSRCDIR)$(PS)pu_common.hh \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_OctetString.c


$(LCOBJDIR)$(PS)pe_SmallNonNegWholeNumber$(OBJ): $(PERSRCDIR)$(PS)pe_SmallNonNegWholeNumber.c \
 $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTSRCDIR)$(PS)asn1CharSet.h \
 $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(PERCFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)pe_SmallNonNegWholeNumber.c


$(OBJDIR)$(PS)asn1PerCppTypes$(OBJ): $(PERSRCDIR)$(PS)asn1PerCppTypes.cpp \
 $(PERSRCDIR)$(PS)asn1PerCppTypes.h $(PERSRCDIR)$(PS)asn1per.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTSRCDIR)$(PS)asn1CharSet.h $(RTXSRCDIR)$(PS)rtxBitEncode.h $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTSRCDIR)$(PS)asn1CppTypes.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.h $(RTSRCDIR)$(PS)asn1CppEvtHndlr.h \
 $(RTSRCDIR)$(PS)ASN1Context.h $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)OSRTMsgBuf.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h $(RTSRCDIR)$(PS)ASN1TOctStr.h $(RTSRCDIR)$(PS)ASN1TOctStr64.h \
 $(RTSRCDIR)$(PS)ASN1TObjId.h $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)OSRTInputStream.h \
 $(RTXSRCDIR)$(PS)OSRTInputStreamIF.h $(RTXSRCDIR)$(PS)OSRTStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTStream.h $(RTXSRCDIR)$(PS)OSRTOutputStream.h \
 $(RTXSRCDIR)$(PS)OSRTOutputStreamIF.h $(RTXSRCDIR)$(PS)rtxStream.h
	$(CCC) $(PERCFLAGS) $(RTCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(PERSRCDIR)$(PS)asn1PerCppTypes.cpp

