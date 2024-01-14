# C to object file compilation rules

rtxArrayList$(OBJ): $(RTXSRCDIR)/rtxArrayList.c $(RTXSRCDIR)/rtxArrayList.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxArrayList.c

rtxBase64$(OBJ): $(RTXSRCDIR)/rtxBase64.c $(RTXSRCDIR)/rtxBase64.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxBase64.c

rtxBench$(OBJ): $(RTXSRCDIR)/rtxBench.c $(RTXSRCDIR)/rtxBench.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxFile.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxBench.c

rtxBigFloat$(OBJ): $(RTXSRCDIR)/rtxBigFloat.c $(RTXSRCDIR)/rtxBigFloat.h \
 $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxReal.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxBigFloat.c

rtxBigInt$(OBJ): $(RTXSRCDIR)/rtxBigInt.c $(RTXSRCDIR)/rtxBigInt.hh \
 $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxReal.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxBigInt.c

rtxBigIntArithmetic$(OBJ): $(RTXSRCDIR)/rtxBigIntArithmetic.c \
 $(RTXSRCDIR)/rtxBigInt.hh $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxBigIntArithmetic.c

rtxBigIntCommon$(OBJ): $(RTXSRCDIR)/rtxBigIntCommon.c $(RTXSRCDIR)/rtxBigInt.hh \
 $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxBigIntCommon.c

rtxBigNumber$(OBJ): $(RTXSRCDIR)/rtxBigNumber.c $(RTXSRCDIR)/rtxBigNumber.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxBigNumber.c

rtxBitDecode$(OBJ): $(RTXSRCDIR)/rtxBitDecode.c $(RTXSRCDIR)/rtxBitDecode.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxContext.hh $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxBitDecode.c

rtxBitEncode$(OBJ): $(RTXSRCDIR)/rtxBitEncode.c $(RTXSRCDIR)/rtxBitEncode.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxContext.hh $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxBitEncode.c

rtxBitString$(OBJ): $(RTXSRCDIR)/rtxBitString.c $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxBitString.c

rtxCharStr$(OBJ): $(RTXSRCDIR)/rtxCharStr.c $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxCharStr.c

rtxCharStrToInt$(OBJ): $(RTXSRCDIR)/rtxCharStrToInt.c $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxCharStrToInt.c

rtxCharStrToInt64$(OBJ): $(RTXSRCDIR)/rtxCharStrToInt64.c $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxCharStrToInt64.c

rtxCharStrToUInt$(OBJ): $(RTXSRCDIR)/rtxCharStrToUInt.c $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxCharStrToUInt.c

rtxCharStrToUInt64$(OBJ): $(RTXSRCDIR)/rtxCharStrToUInt64.c \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxCharStrToUInt64.c

rtxCheckBuffer$(OBJ): $(RTXSRCDIR)/rtxCheckBuffer.c $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxCheckBuffer.c

rtxContext$(OBJ): $(RTXSRCDIR)/rtxContext.c $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxContext.hh $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxMemHeap.hh $(RTXSRCDIR)/rtxPrintStream.h \
 $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/rtxPattern.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxContext.c

rtxContextInit$(OBJ): $(RTXSRCDIR)/rtxContextInit.c $(RTXSRCDIR)/rtxContext.hh \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxContextInit.c

rtxContextInitExt$(OBJ): $(RTXSRCDIR)/rtxContextInitExt.c $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxContextInitExt.c

rtxContextPos$(OBJ): $(RTXSRCDIR)/rtxContextPos.c $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxContextPos.c

rtxCopyAsciiText$(OBJ): $(RTXSRCDIR)/rtxCopyAsciiText.c $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxCopyAsciiText.c

rtxCopyUTF8Text$(OBJ): $(RTXSRCDIR)/rtxCopyUTF8Text.c $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxUTF8.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxCopyUTF8Text.c

rtxDList$(OBJ): $(RTXSRCDIR)/rtxDList.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxContext.hh $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxUTF8.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDList.c

rtxDateTime$(OBJ): $(RTXSRCDIR)/rtxDateTime.c $(RTXSRCDIR)/rtxDateTime.hh \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDateTime.c

rtxDateTimeCompare$(OBJ): $(RTXSRCDIR)/rtxDateTimeCompare.c \
 $(RTXSRCDIR)/rtxDateTime.hh $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDateTimeCompare.c

rtxDateTimeCompare2$(OBJ): $(RTXSRCDIR)/rtxDateTimeCompare2.c \
 $(RTXSRCDIR)/rtxDateTime.hh $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDateTimeCompare2.c

rtxDateTimeParse$(OBJ): $(RTXSRCDIR)/rtxDateTimeParse.c $(RTXSRCDIR)/rtxDateTime.hh \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDateTimeParse.c

rtxDateTimeParseCommon$(OBJ): $(RTXSRCDIR)/rtxDateTimeParseCommon.c \
 $(RTXSRCDIR)/rtxDateTime.hh $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDateTimeParseCommon.c

rtxDateTimePrint$(OBJ): $(RTXSRCDIR)/rtxDateTimePrint.c $(RTXSRCDIR)/rtxDateTime.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxPrint.h $(RTXSRCDIR)/rtxHexDump.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDateTimePrint.c

rtxDateTimeToString$(OBJ): $(RTXSRCDIR)/rtxDateTimeToString.c \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxDateTime.hh $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxUTF8.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDateTimeToString.c

rtxDateTimeToStringCommon$(OBJ): $(RTXSRCDIR)/rtxDateTimeToStringCommon.c \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxDateTime.hh \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDateTimeToStringCommon.c

rtxDatesParse$(OBJ): $(RTXSRCDIR)/rtxDatesParse.c $(RTXSRCDIR)/rtxDateTime.hh \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDatesParse.c

rtxDatesPrint$(OBJ): $(RTXSRCDIR)/rtxDatesPrint.c $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxPrint.h $(RTXSRCDIR)/rtxHexDump.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDatesPrint.c

rtxDatesToString$(OBJ): $(RTXSRCDIR)/rtxDatesToString.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxDateTime.hh \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDatesToString.c

rtxDecimalHelper$(OBJ): $(RTXSRCDIR)/rtxDecimalHelper.c $(RTXSRCDIR)/rtxDecimal.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDecimalHelper.c

rtxDiag$(OBJ): $(RTXSRCDIR)/rtxDiag.c $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPrint.h $(RTXSRCDIR)/rtxHexDump.h \
 $(RTXSRCDIR)/rtxPrintStream.h $(RTXSRCDIR)/rtxPrintToStream.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDiag.c

rtxDiagBitTrace$(OBJ): $(RTXSRCDIR)/rtxDiagBitTrace.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxDiagBitTrace.h $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxPrintToStream.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxStream.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDiagBitTrace.c

rtxDiagBitTraceHTML$(OBJ): $(RTXSRCDIR)/rtxDiagBitTraceHTML.c \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxDiagBitTrace.h $(RTXSRCDIR)/rtxMemBuf.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxPrintToStream.h $(RTXSRCDIR)/rtxFile.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPrint.h $(RTXSRCDIR)/rtxHexDump.h \
 $(RTXSRCDIR)/rtxPrintStream.h $(RTXSRCDIR)/rtxStream.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDiagBitTraceHTML.c

rtxDiagBitTracePrint$(OBJ): $(RTXSRCDIR)/rtxDiagBitTracePrint.c \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxDiagBitTrace.h $(RTXSRCDIR)/rtxMemBuf.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxPrintToStream.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPrint.h $(RTXSRCDIR)/rtxHexDump.h $(RTXSRCDIR)/rtxPrintStream.h \
 $(RTXSRCDIR)/rtxStream.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDiagBitTracePrint.c

rtxDuration$(OBJ): $(RTXSRCDIR)/rtxDuration.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDuration.c

rtxDynBitSet$(OBJ): $(RTXSRCDIR)/rtxDynBitSet.c $(RTXSRCDIR)/rtxDynBitSet.h \
 $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDynBitSet.c

rtxDynPtrArray$(OBJ): $(RTXSRCDIR)/rtxDynPtrArray.c $(RTXSRCDIR)/rtxDynPtrArray.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxDynPtrArray.c

rtxError$(OBJ): $(RTXSRCDIR)/rtxError.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxContext.hh
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxError.c

rtxFile$(OBJ): $(RTXSRCDIR)/rtxFile.c $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxFile.h \
 $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxFile.c

rtxFloat$(OBJ): $(RTXSRCDIR)/rtxFloat.c $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxFloat.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxFloat.c

rtxGenValueType$(OBJ): $(RTXSRCDIR)/rtxGenValueType.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxGenValueType.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxHexDump.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxGenValueType.c

rtxHashMap$(OBJ): $(RTXSRCDIR)/rtxHashMap.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxHashMap.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxHashMap.c

rtxHashMapStr2Int$(OBJ): $(RTXSRCDIR)/rtxHashMapStr2Int.c \
 $(RTXSRCDIR)/rtxHashMapStr2Int.h $(RTXSRCDIR)/rtxHashMapUndef.h \
 $(RTXSRCDIR)/rtxHashMap.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxHashMap.c $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxHashMapStr2Int.c

rtxHashMapStr2UInt$(OBJ): $(RTXSRCDIR)/rtxHashMapStr2UInt.c \
 $(RTXSRCDIR)/rtxHashMapStr2UInt.h $(RTXSRCDIR)/rtxHashMapUndef.h \
 $(RTXSRCDIR)/rtxHashMap.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxHashMap.c $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxHashMapStr2UInt.c

rtxHexCharsToBin$(OBJ): $(RTXSRCDIR)/rtxHexCharsToBin.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxHexCharsToBin.c

rtxHexDump$(OBJ): $(RTXSRCDIR)/rtxHexDump.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxHexDump.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxHexDump.c

rtxHexDumpFile$(OBJ): $(RTXSRCDIR)/rtxHexDumpFile.c $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxHexDump.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxHexDumpFile.c

rtxHttp$(OBJ): $(RTXSRCDIR)/rtxHttp.c $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxHttp.h $(RTXSRCDIR)/rtxArrayList.h $(RTXSRCDIR)/rtxNetUtil.h \
 $(RTXSRCDIR)/rtxSocket.h $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxUTF8.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxHttp.c

rtxInt64ToCharStr$(OBJ): $(RTXSRCDIR)/rtxInt64ToCharStr.c \
 $(RTXSRCDIR)/rtxIntToCharStr.c $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxInt64ToCharStr.c

rtxIntDecode$(OBJ): $(RTXSRCDIR)/rtxIntDecode.c $(RTXSRCDIR)/rtxBitDecode.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxIntDecode.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxIntDecode.c

rtxIntEncode$(OBJ): $(RTXSRCDIR)/rtxIntEncode.c $(RTXSRCDIR)/rtxBitEncode.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxIntEncode.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxIntEncode.c

rtxIntStack$(OBJ): $(RTXSRCDIR)/rtxIntStack.c $(RTXSRCDIR)/rtxIntStack.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxIntStack.c

rtxIntToCharStr$(OBJ): $(RTXSRCDIR)/rtxIntToCharStr.c $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxIntToCharStr.c

rtxLatin1$(OBJ): $(RTXSRCDIR)/rtxLatin1.c $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxLatin1.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxLatin1.c

rtxLookupBigEnum$(OBJ): $(RTXSRCDIR)/rtxLookupBigEnum.c \
 $(RTXSRCDIR)/rtxLookupEnum.c $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxLookupBigEnum.c

rtxLookupEnum$(OBJ): $(RTXSRCDIR)/rtxLookupEnum.c $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxLookupEnum.c

rtxLookupEnumU32$(OBJ): $(RTXSRCDIR)/rtxLookupEnumU32.c \
 $(RTXSRCDIR)/rtxLookupEnum.c $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxLookupEnumU32.c

rtxMemBuf$(OBJ): $(RTXSRCDIR)/rtxMemBuf.c $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxMemBuf.c

rtxMemFreeOpenSeqExt$(OBJ): $(RTXSRCDIR)/rtxMemFreeOpenSeqExt.c \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxMemFreeOpenSeqExt.c

rtxMemFuncs$(OBJ): $(RTXSRCDIR)/rtxMemFuncs.c $(RTXSRCDIR)/rtxMemHeap.hh \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxMemFuncs.c

rtxMemHeap$(OBJ): $(RTXSRCDIR)/rtxMemHeap.c $(RTXSRCDIR)/rtxMemHeap.hh \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxMemHeap.c

rtxMemHeapAlloc$(OBJ): $(RTXSRCDIR)/rtxMemHeapAlloc.c $(RTXSRCDIR)/rtxMemHeap.hh \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxMemHeapAlloc.c

rtxMemHeapAutoPtr$(OBJ): $(RTXSRCDIR)/rtxMemHeapAutoPtr.c \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemHeap.hh $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxMemHeapAutoPtr.c

rtxMemHeapCreate$(OBJ): $(RTXSRCDIR)/rtxMemHeapCreate.c $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxMemHeap.hh $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxMemHeapCreate.c

rtxMemHeapCreateExt$(OBJ): $(RTXSRCDIR)/rtxMemHeapCreateExt.c \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemHeap.hh $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxMemHeapCreateExt.c

rtxMemHeapDebug$(OBJ): $(RTXSRCDIR)/rtxMemHeapDebug.c $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemHeap.hh \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPrintStream.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxMemHeapDebug.c

rtxMemHeapFree$(OBJ): $(RTXSRCDIR)/rtxMemHeapFree.c $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxMemHeap.hh $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxMemHeapFree.c

rtxMemHeapInvalidPtrHook$(OBJ): $(RTXSRCDIR)/rtxMemHeapInvalidPtrHook.c \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemHeap.hh $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxMemHeapInvalidPtrHook.c

rtxMemHeapRealloc$(OBJ): $(RTXSRCDIR)/rtxMemHeapRealloc.c \
 $(RTXSRCDIR)/rtxMemHeap.hh $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxMemHeapRealloc.c

rtxMemHeapReset$(OBJ): $(RTXSRCDIR)/rtxMemHeapReset.c $(RTXSRCDIR)/rtxMemHeap.hh \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxMemHeapReset.c

rtxMemStatic$(OBJ): $(RTXSRCDIR)/rtxMemStatic.c $(RTXSRCDIR)/rtxMemStatic.h \
 $(RTXSRCDIR)/rtxMemHeap.hh $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxMemStatic.c

rtxMemory$(OBJ): $(RTXSRCDIR)/rtxMemory.c
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxMemory.c

rtxNetUtil$(OBJ): $(RTXSRCDIR)/rtxNetUtil.c $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxNetUtil.h $(RTXSRCDIR)/rtxSocket.h $(RTXSRCDIR)/rtxStreamSocket.h \
 $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/rtxSysInfo.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxNetUtil.c

rtxPattern$(OBJ): $(RTXSRCDIR)/rtxPattern.c $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxRegExp.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxPattern.c

rtxPrint$(OBJ): $(RTXSRCDIR)/rtxPrint.c $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxPrint.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxHexDump.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxPrint.c

rtxPrintFile$(OBJ): $(RTXSRCDIR)/rtxPrintFile.c $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxPrint.h $(RTXSRCDIR)/rtxHexDump.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxPrintFile.c

rtxPrintStream$(OBJ): $(RTXSRCDIR)/rtxPrintStream.c $(RTXSRCDIR)/rtxPrintStream.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxPrintStream.c

rtxPrintToStream$(OBJ): $(RTXSRCDIR)/rtxPrintToStream.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxFile.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxPrint.h $(RTXSRCDIR)/rtxHexDump.h $(RTXSRCDIR)/rtxPrintStream.h \
 $(RTXSRCDIR)/rtxPrintToStream.h $(RTXSRCDIR)/rtxError.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxPrintToStream.c

rtxRandTest$(OBJ): $(RTXSRCDIR)/rtxRandTest.c $(RTXSRCDIR)/rtxRandTest.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxStream.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxMemBuf.h \
 $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxRandTest.c

rtxReadBytes$(OBJ): $(RTXSRCDIR)/rtxReadBytes.c $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxStream.h \
 $(RTXSRCDIR)/rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxReadBytes.c

rtxRealHelper$(OBJ): $(RTXSRCDIR)/rtxRealHelper.c $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxRealHelper.c

rtxRegExp$(OBJ): $(RTXSRCDIR)/rtxRegExp.c $(RTXSRCDIR)/rtxRegExp.hh \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxRegExp.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxUnicode.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxUTF8.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxRegExp.c

rtxRegExp2$(OBJ): $(RTXSRCDIR)/rtxRegExp2.c $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxRegExp.hh $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxRegExp.h $(RTXSRCDIR)/rtxUnicode.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxRegExp2.c

rtxRegExpPrint$(OBJ): $(RTXSRCDIR)/rtxRegExpPrint.c $(RTXSRCDIR)/rtxRegExp.hh \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxRegExp.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxUnicode.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxUTF8.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxRegExpPrint.c

rtxSList$(OBJ): $(RTXSRCDIR)/rtxSList.c $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxSList.c

rtxSOAP$(OBJ): $(RTXSRCDIR)/rtxSOAP.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/rtxSOAP.h $(RTXSRCDIR)/rtxCommon.h \
 $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxFile.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h \
 $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h $(RTXSRCDIR)/rtxSocket.h \
 $(RTXSRCDIR)/rtxStreamSocket.h $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxSOAP.c

rtxScalarDList$(OBJ): $(RTXSRCDIR)/rtxScalarDList.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxScalarDList.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxUTF8.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxScalarDList.c

rtxSizeToCharStr$(OBJ): $(RTXSRCDIR)/rtxSizeToCharStr.c $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxIntToCharStr.c
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxSizeToCharStr.c

rtxSocket$(OBJ): $(RTXSRCDIR)/rtxSocket.c $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSocket.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxSocket.c

rtxStack$(OBJ): $(RTXSRCDIR)/rtxStack.c $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxStack.c

rtxStream$(OBJ): $(RTXSRCDIR)/rtxStream.c $(RTXSRCDIR)/rtxStream.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxContext.hh $(RTXSRCDIR)/rtxDiag.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxStream.c

rtxStreamBase64Text$(OBJ): $(RTXSRCDIR)/rtxStreamBase64Text.c \
 $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPrint.h \
 $(RTXSRCDIR)/rtxHexDump.h $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxMemBuf.h \
 $(RTXSRCDIR)/rtxStreamBase64Text.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxStreamBase64Text.c

rtxStreamBuffered$(OBJ): $(RTXSRCDIR)/rtxStreamBuffered.c \
 $(RTXSRCDIR)/rtxStreamBuffered.h $(RTXSRCDIR)/rtxStream.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxStreamBuffered.c

rtxStreamCtxtBuf$(OBJ): $(RTXSRCDIR)/rtxStreamCtxtBuf.c $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxStreamCtxtBuf.h $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxStreamCtxtBuf.c

rtxStreamDirectBuf$(OBJ): $(RTXSRCDIR)/rtxStreamDirectBuf.c $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxStreamMemory.h $(RTXSRCDIR)/rtxStream.h \
 $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/rtxStreamDirectBuf.hh
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxStreamDirectBuf.c

rtxStreamFile$(OBJ): $(RTXSRCDIR)/rtxStreamFile.c $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxFile.h \
 $(RTXSRCDIR)/rtxHexDump.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxStreamFile.h \
 $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxStreamFile.c

rtxStreamHexText$(OBJ): $(RTXSRCDIR)/rtxStreamHexText.c $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxStreamHexText.h $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxStreamHexText.c

rtxStreamMemory$(OBJ): $(RTXSRCDIR)/rtxStreamMemory.c $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxHexDump.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxStreamMemory.h $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxStreamMemory.c

rtxStreamRelease$(OBJ): $(RTXSRCDIR)/rtxStreamRelease.c $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxStreamRelease.c

rtxStreamSocket$(OBJ): $(RTXSRCDIR)/rtxStreamSocket.c $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxHexDump.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxSocket.h $(RTXSRCDIR)/rtxStreamSocket.h $(RTXSRCDIR)/rtxStream.h \
 $(RTXSRCDIR)/rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxStreamSocket.c

rtxSysInfo$(OBJ): $(RTXSRCDIR)/rtxSysInfo.c $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxSysInfo.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxSysInfo.c

rtxTBCD$(OBJ): $(RTXSRCDIR)/rtxTBCD.c $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxTBCD.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxTBCD.c

rtxTestNumericEnum$(OBJ): $(RTXSRCDIR)/rtxTestNumericEnum.c $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxTestNumericEnum.c

rtxTestNumericEnum64$(OBJ): $(RTXSRCDIR)/rtxTestNumericEnum64.c \
 $(RTXSRCDIR)/rtxTestNumericEnum.c $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxTestNumericEnum64.c

rtxTestNumericEnumU32$(OBJ): $(RTXSRCDIR)/rtxTestNumericEnumU32.c \
 $(RTXSRCDIR)/rtxTestNumericEnum.c $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxTestNumericEnumU32.c

rtxTestNumericEnumU64$(OBJ): $(RTXSRCDIR)/rtxTestNumericEnumU64.c \
 $(RTXSRCDIR)/rtxTestNumericEnum.c $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxTestNumericEnumU64.c

rtxToken$(OBJ): $(RTXSRCDIR)/rtxToken.c $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxToken.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxToken.c

rtxTokenConst$(OBJ): $(RTXSRCDIR)/rtxTokenConst.c $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxTokenConst.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxTokenConst.c

rtxUInt64ToCharStr$(OBJ): $(RTXSRCDIR)/rtxUInt64ToCharStr.c \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxIntToCharStr.c
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUInt64ToCharStr.c

rtxUIntToCharStr$(OBJ): $(RTXSRCDIR)/rtxUIntToCharStr.c $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxIntToCharStr.c
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUIntToCharStr.c

rtxUTF16$(OBJ): $(RTXSRCDIR)/rtxUTF16.c $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxUTF16.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUTF16.c

rtxUTF8$(OBJ): $(RTXSRCDIR)/rtxUTF8.c $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUTF8ToUnicode.c
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUTF8.c

rtxUTF8StrToBool$(OBJ): $(RTXSRCDIR)/rtxUTF8StrToBool.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUTF8StrToBool.c

rtxUTF8StrToDouble$(OBJ): $(RTXSRCDIR)/rtxUTF8StrToDouble.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxReal.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/rtxStack.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUTF8StrToDouble.c

rtxUTF8StrToDynHexStr$(OBJ): $(RTXSRCDIR)/rtxUTF8StrToDynHexStr.c \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUTF8StrToDynHexStr.c

rtxUTF8StrToInt$(OBJ): $(RTXSRCDIR)/rtxUTF8StrToInt.c $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUTF8StrToInt.c

rtxUTF8StrToInt64$(OBJ): $(RTXSRCDIR)/rtxUTF8StrToInt64.c \
 $(RTXSRCDIR)/rtxUTF8StrToInt.c $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUTF8StrToInt64.c

rtxUTF8StrToNamedBits$(OBJ): $(RTXSRCDIR)/rtxUTF8StrToNamedBits.c \
 $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxDiag.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUTF8StrToNamedBits.c

rtxUTF8StrToSize$(OBJ): $(RTXSRCDIR)/rtxUTF8StrToSize.c \
 $(RTXSRCDIR)/rtxUTF8StrToInt.c $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUTF8StrToSize.c

rtxUTF8StrToUInt$(OBJ): $(RTXSRCDIR)/rtxUTF8StrToUInt.c \
 $(RTXSRCDIR)/rtxUTF8StrToInt.c $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUTF8StrToUInt.c

rtxUTF8StrToUInt64$(OBJ): $(RTXSRCDIR)/rtxUTF8StrToUInt64.c \
 $(RTXSRCDIR)/rtxUTF8StrToInt.c $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUTF8StrToUInt64.c

rtxUTF8ToDynUniStr$(OBJ): $(RTXSRCDIR)/rtxUTF8ToDynUniStr.c \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUTF8ToDynUniStr.c

rtxUTF8ToUnicode$(OBJ): $(RTXSRCDIR)/rtxUTF8ToUnicode.c $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxUTF8.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUTF8ToUnicode.c

rtxUnicode$(OBJ): $(RTXSRCDIR)/rtxUnicode.c $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxUnicode.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUnicode.c

rtxUtil$(OBJ): $(RTXSRCDIR)/rtxUtil.c $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxUtil.h \
 $(RTXSRCDIR)/rtxExternDefs.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUtil.c

rtxUtil64$(OBJ): $(RTXSRCDIR)/rtxUtil64.c $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxUtil.h $(RTXSRCDIR)/rtxExternDefs.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxUtil64.c

rtxWriteBytes$(OBJ): $(RTXSRCDIR)/rtxWriteBytes.c $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxContext.hh $(RTXSRCDIR)/rtxStream.h \
 $(RTXSRCDIR)/rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxWriteBytes.c

rtxXmlQName$(OBJ): $(RTXSRCDIR)/rtxXmlQName.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxXmlQName.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxUTF8.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxXmlQName.c

rtxXmlStr$(OBJ): $(RTXSRCDIR)/rtxXmlStr.c $(RTXSRCDIR)/rtxXmlStr.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxUTF8.h
	$(CC) $(CFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxXmlStr.c
