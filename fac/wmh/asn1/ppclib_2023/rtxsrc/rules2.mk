# C to object file compilation rules

$(OBJDIR)$(PS)rtxArrayList$(OBJ): $(RTXSRCDIR)$(PS)rtxArrayList.c $(RTXSRCDIR)$(PS)rtxArrayList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxArrayList.c


$(OBJDIR)$(PS)rtxBase64$(OBJ): $(RTXSRCDIR)$(PS)rtxBase64.c $(RTXSRCDIR)$(PS)rtxBase64.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxBase64.c


$(OBJDIR)$(PS)rtxBench$(OBJ): $(RTXSRCDIR)$(PS)rtxBench.c $(RTXSRCDIR)$(PS)rtxBench.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxFile.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxBench.c


$(OBJDIR)$(PS)rtxBigFloat$(OBJ): $(RTXSRCDIR)$(PS)rtxBigFloat.c $(RTXSRCDIR)$(PS)rtxBigFloat.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxReal.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxBigFloat.c


$(OBJDIR)$(PS)rtxBigInt$(OBJ): $(RTXSRCDIR)$(PS)rtxBigInt.c $(RTXSRCDIR)$(PS)rtxBigInt.hh \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxReal.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxBigInt.c


$(OBJDIR)$(PS)rtxBigIntArithmetic$(OBJ): $(RTXSRCDIR)$(PS)rtxBigIntArithmetic.c \
 $(RTXSRCDIR)$(PS)rtxBigInt.hh $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxBigIntArithmetic.c


$(OBJDIR)$(PS)rtxBigIntCommon$(OBJ): $(RTXSRCDIR)$(PS)rtxBigIntCommon.c $(RTXSRCDIR)$(PS)rtxBigInt.hh \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxBigIntCommon.c


$(OBJDIR)$(PS)rtxBigNumber$(OBJ): $(RTXSRCDIR)$(PS)rtxBigNumber.c $(RTXSRCDIR)$(PS)rtxBigNumber.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxBigNumber.c


$(OBJDIR)$(PS)rtxBitString$(OBJ): $(RTXSRCDIR)$(PS)rtxBitString.c $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxBitString.c


$(OBJDIR)$(PS)rtxCharStr$(OBJ): $(RTXSRCDIR)$(PS)rtxCharStr.c $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCharStr.c


$(OBJDIR)$(PS)rtxCharStrToInt$(OBJ): $(RTXSRCDIR)$(PS)rtxCharStrToInt.c $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCharStrToInt.c


$(OBJDIR)$(PS)rtxCharStrToInt64$(OBJ): $(RTXSRCDIR)$(PS)rtxCharStrToInt64.c $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCharStrToInt64.c


$(OBJDIR)$(PS)rtxCharStrToUInt$(OBJ): $(RTXSRCDIR)$(PS)rtxCharStrToUInt.c $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCharStrToUInt.c


$(OBJDIR)$(PS)rtxCharStrToUInt64$(OBJ): $(RTXSRCDIR)$(PS)rtxCharStrToUInt64.c \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCharStrToUInt64.c


$(OBJDIR)$(PS)rtxCheckBuffer$(OBJ): $(RTXSRCDIR)$(PS)rtxCheckBuffer.c $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCheckBuffer.c


$(OBJDIR)$(PS)rtxContextInitExt$(OBJ): $(RTXSRCDIR)$(PS)rtxContextInitExt.c $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxContextInitExt.c


$(OBJDIR)$(PS)rtxContextPos$(OBJ): $(RTXSRCDIR)$(PS)rtxContextPos.c $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxContextPos.c


$(OBJDIR)$(PS)rtxCopyAsciiText$(OBJ): $(RTXSRCDIR)$(PS)rtxCopyAsciiText.c $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCopyAsciiText.c


$(OBJDIR)$(PS)rtxCopyUTF8Text$(OBJ): $(RTXSRCDIR)$(PS)rtxCopyUTF8Text.c $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxUTF8.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCopyUTF8Text.c


$(OBJDIR)$(PS)rtxDatesParse$(OBJ): $(RTXSRCDIR)$(PS)rtxDatesParse.c $(RTXSRCDIR)$(PS)rtxDateTime.hh \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDatesParse.c


$(OBJDIR)$(PS)rtxDatesPrint$(OBJ): $(RTXSRCDIR)$(PS)rtxDatesPrint.c $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxPrint.h $(RTXSRCDIR)$(PS)rtxHexDump.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDatesPrint.c


$(OBJDIR)$(PS)rtxDatesToString$(OBJ): $(RTXSRCDIR)$(PS)rtxDatesToString.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxDateTime.hh \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDatesToString.c


$(OBJDIR)$(PS)rtxDateTime$(OBJ): $(RTXSRCDIR)$(PS)rtxDateTime.c $(RTXSRCDIR)$(PS)rtxDateTime.hh \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDateTime.c


$(OBJDIR)$(PS)rtxDateTimeCompare$(OBJ): $(RTXSRCDIR)$(PS)rtxDateTimeCompare.c \
 $(RTXSRCDIR)$(PS)rtxDateTime.hh $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDateTimeCompare.c


$(OBJDIR)$(PS)rtxDateTimeCompare2$(OBJ): $(RTXSRCDIR)$(PS)rtxDateTimeCompare2.c \
 $(RTXSRCDIR)$(PS)rtxDateTime.hh $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDateTimeCompare2.c


$(OBJDIR)$(PS)rtxDateTimeParse$(OBJ): $(RTXSRCDIR)$(PS)rtxDateTimeParse.c $(RTXSRCDIR)$(PS)rtxDateTime.hh \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDateTimeParse.c


$(OBJDIR)$(PS)rtxDateTimeParseCommon$(OBJ): $(RTXSRCDIR)$(PS)rtxDateTimeParseCommon.c \
 $(RTXSRCDIR)$(PS)rtxDateTime.hh $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDateTimeParseCommon.c


$(OBJDIR)$(PS)rtxDateTimeToString$(OBJ): $(RTXSRCDIR)$(PS)rtxDateTimeToString.c \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.hh $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxUTF8.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDateTimeToString.c


$(OBJDIR)$(PS)rtxDateTimeToStringCommon$(OBJ): $(RTXSRCDIR)$(PS)rtxDateTimeToStringCommon.c \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxDateTime.hh \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDateTimeToStringCommon.c


$(OBJDIR)$(PS)rtxDateTimePrint$(OBJ): $(RTXSRCDIR)$(PS)rtxDateTimePrint.c $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxPrint.h $(RTXSRCDIR)$(PS)rtxHexDump.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDateTimePrint.c


$(OBJDIR)$(PS)rtxDecimalHelper$(OBJ): $(RTXSRCDIR)$(PS)rtxDecimalHelper.c $(RTXSRCDIR)$(PS)rtxDecimal.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDecimalHelper.c


$(OBJDIR)$(PS)rtxDiag$(OBJ): $(RTXSRCDIR)$(PS)rtxDiag.c $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPrint.h $(RTXSRCDIR)$(PS)rtxHexDump.h \
 $(RTXSRCDIR)$(PS)rtxPrintStream.h $(RTXSRCDIR)$(PS)rtxPrintToStream.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDiag.c


$(OBJDIR)$(PS)rtxDiagBitTrace$(OBJ): $(RTXSRCDIR)$(PS)rtxDiagBitTrace.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxStream.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDiagBitTrace.c


$(OBJDIR)$(PS)rtxDiagBitTraceHTML$(OBJ): $(RTXSRCDIR)$(PS)rtxDiagBitTraceHTML.c \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPrint.h $(RTXSRCDIR)$(PS)rtxHexDump.h \
 $(RTXSRCDIR)$(PS)rtxPrintStream.h $(RTXSRCDIR)$(PS)rtxStream.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDiagBitTraceHTML.c


$(OBJDIR)$(PS)rtxDiagBitTracePrint$(OBJ): $(RTXSRCDIR)$(PS)rtxDiagBitTracePrint.c \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxDiagBitTrace.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPrint.h $(RTXSRCDIR)$(PS)rtxHexDump.h $(RTXSRCDIR)$(PS)rtxPrintStream.h \
 $(RTXSRCDIR)$(PS)rtxStream.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDiagBitTracePrint.c


$(OBJDIR)$(PS)rtxDuration$(OBJ): $(RTXSRCDIR)$(PS)rtxDuration.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDuration.c


$(OBJDIR)$(PS)rtxDynBitSet$(OBJ): $(RTXSRCDIR)$(PS)rtxDynBitSet.c $(RTXSRCDIR)$(PS)rtxDynBitSet.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDynBitSet.c


$(OBJDIR)$(PS)rtxDynPtrArray$(OBJ): $(RTXSRCDIR)$(PS)rtxDynPtrArray.c $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDynPtrArray.c


$(OBJDIR)$(PS)rtxFile$(OBJ): $(RTXSRCDIR)$(PS)rtxFile.c $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxFile.c


$(OBJDIR)$(PS)rtxFloat$(OBJ): $(RTXSRCDIR)$(PS)rtxFloat.c $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxFloat.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxFloat.c


$(OBJDIR)$(PS)rtxGenValueType$(OBJ): $(RTXSRCDIR)$(PS)rtxGenValueType.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxGenValueType.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxHexDump.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxGenValueType.c


$(OBJDIR)$(PS)rtxHashMap$(OBJ): $(RTXSRCDIR)$(PS)rtxHashMap.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxHashMap.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxHashMap.c


$(OBJDIR)$(PS)rtxHashMapStr2Int$(OBJ): $(RTXSRCDIR)$(PS)rtxHashMapStr2Int.c \
 $(RTXSRCDIR)$(PS)rtxHashMapStr2Int.h $(RTXSRCDIR)$(PS)rtxHashMapUndef.h \
 $(RTXSRCDIR)$(PS)rtxHashMap.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxHashMap.c $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxHashMapStr2Int.c


$(OBJDIR)$(PS)rtxHashMapStr2UInt$(OBJ): $(RTXSRCDIR)$(PS)rtxHashMapStr2UInt.c \
 $(RTXSRCDIR)$(PS)rtxHashMapStr2UInt.h $(RTXSRCDIR)$(PS)rtxHashMapUndef.h \
 $(RTXSRCDIR)$(PS)rtxHashMap.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxHashMap.c $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxHashMapStr2UInt.c


$(OBJDIR)$(PS)rtxHexCharsToBin$(OBJ): $(RTXSRCDIR)$(PS)rtxHexCharsToBin.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxHexCharsToBin.c


$(OBJDIR)$(PS)rtxHexDump$(OBJ): $(RTXSRCDIR)$(PS)rtxHexDump.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxHexDump.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxHexDump.c


$(OBJDIR)$(PS)rtxHexDumpFile$(OBJ): $(RTXSRCDIR)$(PS)rtxHexDumpFile.c $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxHexDump.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxHexDumpFile.c


$(OBJDIR)$(PS)rtxHttp$(OBJ): $(RTXSRCDIR)$(PS)rtxHttp.c $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxHttp.h $(RTXSRCDIR)$(PS)rtxArrayList.h $(RTXSRCDIR)$(PS)rtxNetUtil.h \
 $(RTXSRCDIR)$(PS)rtxSocket.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxBase64.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxHttp.c


$(OBJDIR)$(PS)rtxIndent$(OBJ): $(RTXSRCDIR)$(PS)rtxIndent.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxIndent.c


$(OBJDIR)$(PS)rtxIntDecode$(OBJ): $(RTXSRCDIR)$(PS)rtxIntDecode.c $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxIntDecode.h \
 $(RTXSRCDIR)$(PS)rtxIntDecTmpl.c $(RTXSRCDIR)$(PS)rtxUIntDecTmpl.c
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxIntDecode.c


$(OBJDIR)$(PS)rtxIntEncode$(OBJ): $(RTXSRCDIR)$(PS)rtxIntEncode.c $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxIntEncode.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxIntEncode.c


$(OBJDIR)$(PS)rtxIntStack$(OBJ): $(RTXSRCDIR)$(PS)rtxIntStack.c $(RTXSRCDIR)$(PS)rtxIntStack.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxIntStack.c


$(OBJDIR)$(PS)rtxIntToCharStr$(OBJ): $(RTXSRCDIR)$(PS)rtxIntToCharStr.c $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxIntToCharStr.c


$(OBJDIR)$(PS)rtxInt64ToCharStr$(OBJ): $(RTXSRCDIR)$(PS)rtxInt64ToCharStr.c \
 $(RTXSRCDIR)$(PS)rtxIntToCharStr.c $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxInt64ToCharStr.c


$(OBJDIR)$(PS)rtxLatin1$(OBJ): $(RTXSRCDIR)$(PS)rtxLatin1.c $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxLatin1.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxLatin1.c


$(OBJDIR)$(PS)rtxLookupBigEnum$(OBJ): $(RTXSRCDIR)$(PS)rtxLookupBigEnum.c \
 $(RTXSRCDIR)$(PS)rtxLookupEnum.c $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxLookupBigEnum.c


$(OBJDIR)$(PS)rtxLookupEnum$(OBJ): $(RTXSRCDIR)$(PS)rtxLookupEnum.c $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxLookupEnum.c


$(OBJDIR)$(PS)rtxLookupEnumU32$(OBJ): $(RTXSRCDIR)$(PS)rtxLookupEnumU32.c \
 $(RTXSRCDIR)$(PS)rtxLookupEnum.c $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxLookupEnumU32.c


$(OBJDIR)$(PS)rtxMemBuf$(OBJ): $(RTXSRCDIR)$(PS)rtxMemBuf.c $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemHeap.hh $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxMemBuf.c


$(OBJDIR)$(PS)rtxMemFreeOpenSeqExt$(OBJ): $(RTXSRCDIR)$(PS)rtxMemFreeOpenSeqExt.c \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxMemFreeOpenSeqExt.c


$(OBJDIR)$(PS)rtxMemFuncs$(OBJ): $(RTXSRCDIR)$(PS)rtxMemFuncs.c $(RTXSRCDIR)$(PS)rtxMemHeap.hh \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxMemFuncs.c


$(OBJDIR)$(PS)rtxMemHeap$(OBJ): $(RTXSRCDIR)$(PS)rtxMemHeap.c $(RTXSRCDIR)$(PS)rtxMemHeap.hh \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxMemHeap.c


$(OBJDIR)$(PS)rtxMemHeapAlloc$(OBJ): $(RTXSRCDIR)$(PS)rtxMemHeapAlloc.c $(RTXSRCDIR)$(PS)rtxMemHeap.hh \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxMemHeapAlloc.c


$(OBJDIR)$(PS)rtxMemHeapAutoPtr$(OBJ): $(RTXSRCDIR)$(PS)rtxMemHeapAutoPtr.c \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemHeap.hh $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxMemHeapAutoPtr.c


$(OBJDIR)$(PS)rtxMemHeapCreate$(OBJ): $(RTXSRCDIR)$(PS)rtxMemHeapCreate.c $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemHeap.hh $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxMemHeapCreate.c


$(OBJDIR)$(PS)rtxMemHeapCreateExt$(OBJ): $(RTXSRCDIR)$(PS)rtxMemHeapCreateExt.c \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemHeap.hh $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxMemHeapCreateExt.c


$(OBJDIR)$(PS)rtxMemHeapDebug$(OBJ): $(RTXSRCDIR)$(PS)rtxMemHeapDebug.c $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemHeap.hh \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPrintStream.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxMemHeapDebug.c


$(OBJDIR)$(PS)rtxMemHeapFree$(OBJ): $(RTXSRCDIR)$(PS)rtxMemHeapFree.c $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemHeap.hh $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxMemHeapFree.c


$(OBJDIR)$(PS)rtxMemHeapInvalidPtrHook$(OBJ): $(RTXSRCDIR)$(PS)rtxMemHeapInvalidPtrHook.c \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemHeap.hh $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxMemHeapInvalidPtrHook.c


$(OBJDIR)$(PS)rtxMemHeapRealloc$(OBJ): $(RTXSRCDIR)$(PS)rtxMemHeapRealloc.c \
 $(RTXSRCDIR)$(PS)rtxMemHeap.hh $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxMemHeapRealloc.c


$(OBJDIR)$(PS)rtxMemHeapReset$(OBJ): $(RTXSRCDIR)$(PS)rtxMemHeapReset.c $(RTXSRCDIR)$(PS)rtxMemHeap.hh \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxMemHeapReset.c


$(OBJDIR)$(PS)rtxNetUtil$(OBJ): $(RTXSRCDIR)$(PS)rtxNetUtil.c $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxNetUtil.h $(RTXSRCDIR)$(PS)rtxSocket.h $(RTXSRCDIR)$(PS)rtxStreamSocket.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxSysInfo.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxNetUtil.c


$(OBJDIR)$(PS)rtxPattern$(OBJ): $(RTXSRCDIR)$(PS)rtxPattern.c $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxRegExp.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxPattern.c


$(OBJDIR)$(PS)rtxPcapFile$(OBJ): $(RTXSRCDIR)$(PS)rtxPcapFile.c $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxPcapFile.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxPcapFile.c


$(OBJDIR)$(PS)rtxPeekBytes$(OBJ): $(RTXSRCDIR)$(PS)rtxPeekBytes.c $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxPeekBytes.c


$(OBJDIR)$(PS)rtxPrint$(OBJ): $(RTXSRCDIR)$(PS)rtxPrint.c $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxPrint.h $(RTXSRCDIR)$(PS)rtxHexDump.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxPrint.c


$(OBJDIR)$(PS)rtxPrintFile$(OBJ): $(RTXSRCDIR)$(PS)rtxPrintFile.c $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxPrint.h $(RTXSRCDIR)$(PS)rtxHexDump.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxPrintFile.c


$(OBJDIR)$(PS)rtxPrintStream$(OBJ): $(RTXSRCDIR)$(PS)rtxPrintStream.c $(RTXSRCDIR)$(PS)rtxPrintStream.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxPrintStream.c


$(OBJDIR)$(PS)rtxPrintToStream$(OBJ): $(RTXSRCDIR)$(PS)rtxPrintToStream.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxPrint.h $(RTXSRCDIR)$(PS)rtxHexDump.h $(RTXSRCDIR)$(PS)rtxPrintStream.h \
 $(RTXSRCDIR)$(PS)rtxPrintToStream.h $(RTXSRCDIR)$(PS)rtxError.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxPrintToStream.c


$(OBJDIR)$(PS)rtxRandTest$(OBJ): $(RTXSRCDIR)$(PS)rtxRandTest.c $(RTXSRCDIR)$(PS)rtxRandTest.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxRandTest.c


$(OBJDIR)$(PS)rtxReadBytes$(OBJ): $(RTXSRCDIR)$(PS)rtxReadBytes.c $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxReadBytes.c


$(OBJDIR)$(PS)rtxRealHelper$(OBJ): $(RTXSRCDIR)$(PS)rtxRealHelper.c $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxRealHelper.c


$(OBJDIR)$(PS)rtxScalarDList$(OBJ): $(RTXSRCDIR)$(PS)rtxScalarDList.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxScalarDList.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxUTF8.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxScalarDList.c


$(OBJDIR)$(PS)rtxSizeToCharStr$(OBJ): $(RTXSRCDIR)$(PS)rtxSizeToCharStr.c $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxIntToCharStr.c
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxSizeToCharStr.c


$(OBJDIR)$(PS)rtxSList$(OBJ): $(RTXSRCDIR)$(PS)rtxSList.c $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxSList.c


$(OBJDIR)$(PS)rtxSocket$(OBJ): $(RTXSRCDIR)$(PS)rtxSocket.c $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSocket.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxSocket.c


$(OBJDIR)$(PS)rtxSOAP$(OBJ): $(RTXSRCDIR)$(PS)rtxSOAP.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxSOAP.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxSocket.h \
 $(RTXSRCDIR)$(PS)rtxStreamSocket.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxSOAP.c


$(OBJDIR)$(PS)rtxStack$(OBJ): $(RTXSRCDIR)$(PS)rtxStack.c $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxStack.c


$(OBJDIR)$(PS)rtxStreamBuffered$(OBJ): $(RTXSRCDIR)$(PS)rtxStreamBuffered.c \
 $(RTXSRCDIR)$(PS)rtxStreamBuffered.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxStreamBuffered.c


$(OBJDIR)$(PS)rtxStreamCtxtBuf$(OBJ): $(RTXSRCDIR)$(PS)rtxStreamCtxtBuf.c $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxStreamCtxtBuf.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxStreamCtxtBuf.c


$(OBJDIR)$(PS)rtxStreamDirectBuf$(OBJ): $(RTXSRCDIR)$(PS)rtxStreamDirectBuf.c $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxStreamMemory.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxStreamDirectBuf.hh
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxStreamDirectBuf.c


$(OBJDIR)$(PS)rtxStreamFile$(OBJ): $(RTXSRCDIR)$(PS)rtxStreamFile.c $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxHexDump.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxStreamFile.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxStreamFile.c


$(OBJDIR)$(PS)rtxStreamHexText$(OBJ): $(RTXSRCDIR)$(PS)rtxStreamHexText.c $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxStreamHexText.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxStreamHexText.c


$(OBJDIR)$(PS)rtxStreamBase64Text$(OBJ): $(RTXSRCDIR)$(PS)rtxStreamBase64Text.c \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPrint.h \
 $(RTXSRCDIR)$(PS)rtxHexDump.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxStreamBase64Text.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxStreamBase64Text.c


$(OBJDIR)$(PS)rtxStreamMemory$(OBJ): $(RTXSRCDIR)$(PS)rtxStreamMemory.c $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxHexDump.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxStreamMemory.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxStreamMemory.c


$(OBJDIR)$(PS)rtxStreamRelease$(OBJ): $(RTXSRCDIR)$(PS)rtxStreamRelease.c $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxStreamRelease.c


$(OBJDIR)$(PS)rtxStreamSocket$(OBJ): $(RTXSRCDIR)$(PS)rtxStreamSocket.c $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxHexDump.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxSocket.h $(RTXSRCDIR)$(PS)rtxStreamSocket.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxStreamSocket.c


$(OBJDIR)$(PS)rtxSysInfo$(OBJ): $(RTXSRCDIR)$(PS)rtxSysInfo.c $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxSysInfo.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxSysInfo.c


$(OBJDIR)$(PS)rtxTBCD$(OBJ): $(RTXSRCDIR)$(PS)rtxTBCD.c $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxTBCD.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTBCD.c


$(OBJDIR)$(PS)rtxToken$(OBJ): $(RTXSRCDIR)$(PS)rtxToken.c $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxToken.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxToken.c


$(OBJDIR)$(PS)rtxTestNumericEnum$(OBJ): $(RTXSRCDIR)$(PS)rtxTestNumericEnum.c $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTestNumericEnum.c


$(OBJDIR)$(PS)rtxTestNumericEnum64$(OBJ): $(RTXSRCDIR)$(PS)rtxTestNumericEnum64.c \
 $(RTXSRCDIR)$(PS)rtxTestNumericEnum.c $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTestNumericEnum64.c


$(OBJDIR)$(PS)rtxTestNumericEnumU64$(OBJ): $(RTXSRCDIR)$(PS)rtxTestNumericEnumU64.c \
 $(RTXSRCDIR)$(PS)rtxTestNumericEnum.c $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTestNumericEnumU64.c


$(OBJDIR)$(PS)rtxTestNumericEnumU32$(OBJ): $(RTXSRCDIR)$(PS)rtxTestNumericEnumU32.c \
 $(RTXSRCDIR)$(PS)rtxTestNumericEnum.c $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTestNumericEnumU32.c


$(OBJDIR)$(PS)rtxTokenConst$(OBJ): $(RTXSRCDIR)$(PS)rtxTokenConst.c $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxTokenConst.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTokenConst.c


$(OBJDIR)$(PS)rtxTxtParser$(OBJ): $(RTXSRCDIR)$(PS)rtxTxtParser.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTxtParser.c


$(OBJDIR)$(PS)rtxTxtReadBigInt$(OBJ): $(RTXSRCDIR)$(PS)rtxTxtReadBigInt.c $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTxtReadBigInt.c


$(OBJDIR)$(PS)rtxTxtReadInt8$(OBJ): $(RTXSRCDIR)$(PS)rtxTxtReadInt8.c $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTxtReadInt8.c


$(OBJDIR)$(PS)rtxTxtReadInt16$(OBJ): $(RTXSRCDIR)$(PS)rtxTxtReadInt16.c $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTxtReadInt16.c


$(OBJDIR)$(PS)rtxTxtReadInt32$(OBJ): $(RTXSRCDIR)$(PS)rtxTxtReadInt32.c $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTxtReadInt32.c


$(OBJDIR)$(PS)rtxTxtReadInt64$(OBJ): $(RTXSRCDIR)$(PS)rtxTxtReadInt64.c $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTxtReadInt64.c


$(OBJDIR)$(PS)rtxTxtReadUInt8$(OBJ): $(RTXSRCDIR)$(PS)rtxTxtReadUInt8.c $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTxtReadUInt8.c


$(OBJDIR)$(PS)rtxTxtReadUInt16$(OBJ): $(RTXSRCDIR)$(PS)rtxTxtReadUInt16.c $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTxtReadUInt16.c


$(OBJDIR)$(PS)rtxTxtReadUInt32$(OBJ): $(RTXSRCDIR)$(PS)rtxTxtReadUInt32.c $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTxtReadUInt32.c


$(OBJDIR)$(PS)rtxTxtReadUInt64$(OBJ): $(RTXSRCDIR)$(PS)rtxTxtReadUInt64.c $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTxtReadUInt64.c


$(OBJDIR)$(PS)rtxTxtWriteInt$(OBJ): $(RTXSRCDIR)$(PS)rtxTxtWriteInt.c $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTxtWriteInt.c


$(OBJDIR)$(PS)rtxTxtWriteInt64$(OBJ): $(RTXSRCDIR)$(PS)rtxTxtWriteInt64.c \
 $(RTXSRCDIR)$(PS)rtxTxtWriteInt.c $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTxtWriteInt64.c


$(OBJDIR)$(PS)rtxTxtWriteUInt$(OBJ): $(RTXSRCDIR)$(PS)rtxTxtWriteUInt.c $(RTXSRCDIR)$(PS)rtxTxtWriteInt.c \
 $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTxtWriteUInt.c


$(OBJDIR)$(PS)rtxTxtWriteUInt64$(OBJ): $(RTXSRCDIR)$(PS)rtxTxtWriteUInt64.c \
 $(RTXSRCDIR)$(PS)rtxTxtWriteInt.c $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxTxtWriteUInt64.c


$(OBJDIR)$(PS)rtxUIntToCharStr$(OBJ): $(RTXSRCDIR)$(PS)rtxUIntToCharStr.c $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxIntToCharStr.c
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxUIntToCharStr.c


$(OBJDIR)$(PS)rtxUInt64ToCharStr$(OBJ): $(RTXSRCDIR)$(PS)rtxUInt64ToCharStr.c \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxIntToCharStr.c
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxUInt64ToCharStr.c


$(OBJDIR)$(PS)rtxUTF16$(OBJ): $(RTXSRCDIR)$(PS)rtxUTF16.c $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxUTF16.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxUTF16.c


$(OBJDIR)$(PS)rtxUTF8$(OBJ): $(RTXSRCDIR)$(PS)rtxUTF8.c $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUTF8ToUnicode.c
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxUTF8.c


$(OBJDIR)$(PS)rtxUTF8StrToBool$(OBJ): $(RTXSRCDIR)$(PS)rtxUTF8StrToBool.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxUTF8StrToBool.c


$(OBJDIR)$(PS)rtxUTF8StrToDouble$(OBJ): $(RTXSRCDIR)$(PS)rtxUTF8StrToDouble.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxUTF8StrToDouble.c


$(OBJDIR)$(PS)rtxUTF8StrToDynHexStr$(OBJ): $(RTXSRCDIR)$(PS)rtxUTF8StrToDynHexStr.c \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxUTF8StrToDynHexStr.c


$(OBJDIR)$(PS)rtxUTF8StrToInt$(OBJ): $(RTXSRCDIR)$(PS)rtxUTF8StrToInt.c $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxUTF8StrToInt.c


$(OBJDIR)$(PS)rtxUTF8StrToInt64$(OBJ): $(RTXSRCDIR)$(PS)rtxUTF8StrToInt64.c \
 $(RTXSRCDIR)$(PS)rtxUTF8StrToInt.c $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxUTF8StrToInt64.c


$(OBJDIR)$(PS)rtxUTF8StrToSize$(OBJ): $(RTXSRCDIR)$(PS)rtxUTF8StrToSize.c \
 $(RTXSRCDIR)$(PS)rtxUTF8StrToInt.c $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxUTF8StrToSize.c


$(OBJDIR)$(PS)rtxUTF8StrToUInt$(OBJ): $(RTXSRCDIR)$(PS)rtxUTF8StrToUInt.c \
 $(RTXSRCDIR)$(PS)rtxUTF8StrToInt.c $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxUTF8StrToUInt.c


$(OBJDIR)$(PS)rtxUTF8StrToUInt64$(OBJ): $(RTXSRCDIR)$(PS)rtxUTF8StrToUInt64.c \
 $(RTXSRCDIR)$(PS)rtxUTF8StrToInt.c $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxUTF8StrToUInt64.c


$(OBJDIR)$(PS)rtxUTF8StrToNamedBits$(OBJ): $(RTXSRCDIR)$(PS)rtxUTF8StrToNamedBits.c \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxDiag.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxUTF8StrToNamedBits.c


$(OBJDIR)$(PS)rtxUTF8ToDynUniStr$(OBJ): $(RTXSRCDIR)$(PS)rtxUTF8ToDynUniStr.c \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxUTF8ToDynUniStr.c


$(OBJDIR)$(PS)rtxUtil$(OBJ): $(RTXSRCDIR)$(PS)rtxUtil.c $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxUtil.c


$(OBJDIR)$(PS)rtxUtil64$(OBJ): $(RTXSRCDIR)$(PS)rtxUtil64.c $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxUtil64.c


$(OBJDIR)$(PS)rtxXmlQName$(OBJ): $(RTXSRCDIR)$(PS)rtxXmlQName.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxXmlQName.c


$(OBJDIR)$(PS)rtxXmlStr$(OBJ): $(RTXSRCDIR)$(PS)rtxXmlStr.c $(RTXSRCDIR)$(PS)rtxXmlStr.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxUTF8.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxXmlStr.c


$(LCOBJDIR)$(PS)rtxBitDecode$(OBJ): $(RTXSRCDIR)$(PS)rtxBitDecode.c $(RTXSRCDIR)$(PS)rtxBitDecode.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxBitDecTmpl.c
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxBitDecode.c


$(LCOBJDIR)$(PS)rtxBitEncode$(OBJ): $(RTXSRCDIR)$(PS)rtxBitEncode.c $(RTXSRCDIR)$(PS)rtxBitEncode.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxBitEncode.c


$(LCOBJDIR)$(PS)rtxContext$(OBJ): $(RTXSRCDIR)$(PS)rtxContext.c $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxMemHeap.hh $(RTXSRCDIR)$(PS)rtxPrintStream.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxContext.c


$(LCOBJDIR)$(PS)rtxContextInit$(OBJ): $(RTXSRCDIR)$(PS)rtxContextInit.c $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxContextInit.c


$(LCOBJDIR)$(PS)rtxDList$(OBJ): $(RTXSRCDIR)$(PS)rtxDList.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxDList.c


$(LCOBJDIR)$(PS)rtxError$(OBJ): $(RTXSRCDIR)$(PS)rtxError.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxError.c


$(LCOBJDIR)$(PS)rtxStream$(OBJ): $(RTXSRCDIR)$(PS)rtxStream.c $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxDiag.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxStream.c


$(LCOBJDIR)$(PS)rtxWriteBytes$(OBJ): $(RTXSRCDIR)$(PS)rtxWriteBytes.c $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxWriteBytes.c


$(OBJDIR)$(PS)rtxMemory$(OBJ): $(RTXSRCDIR)$(PS)rtxMemory.c
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxMemory.c


$(OBJDIR)$(PS)OSRTContext$(OBJ): $(RTXSRCDIR)$(PS)OSRTContext.cpp $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTContext.cpp


$(OBJDIR)$(PS)OSRTCtxtHolder$(OBJ): $(RTXSRCDIR)$(PS)OSRTCtxtHolder.cpp $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTCtxtHolder.cpp


$(OBJDIR)$(PS)OSRTDiag$(OBJ): $(RTXSRCDIR)$(PS)OSRTDiag.cpp $(RTXSRCDIR)$(PS)OSRTDiag.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTDiag.cpp


$(OBJDIR)$(PS)OSRTFileInputStream$(OBJ): $(RTXSRCDIR)$(PS)OSRTFileInputStream.cpp \
 $(RTXSRCDIR)$(PS)rtxStreamFile.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)OSRTFileInputStream.h \
 $(RTXSRCDIR)$(PS)OSRTInputStream.h $(RTXSRCDIR)$(PS)OSRTInputStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTStreamIF.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)OSRTStream.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTFileInputStream.cpp


$(OBJDIR)$(PS)OSRTFileOutputStream$(OBJ): $(RTXSRCDIR)$(PS)OSRTFileOutputStream.cpp \
 $(RTXSRCDIR)$(PS)rtxStreamFile.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)OSRTFileOutputStream.h \
 $(RTXSRCDIR)$(PS)OSRTOutputStream.h $(RTXSRCDIR)$(PS)OSRTOutputStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTStreamIF.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)OSRTStream.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTFileOutputStream.cpp


$(OBJDIR)$(PS)OSRTHexTextInputStream$(OBJ): $(RTXSRCDIR)$(PS)OSRTHexTextInputStream.cpp \
 $(RTXSRCDIR)$(PS)rtxStreamHexText.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)OSRTHexTextInputStream.h $(RTXSRCDIR)$(PS)OSRTInputStream.h \
 $(RTXSRCDIR)$(PS)OSRTInputStreamIF.h $(RTXSRCDIR)$(PS)OSRTStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)OSRTStream.h $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTHexTextInputStream.cpp


$(OBJDIR)$(PS)OSRTBase64TextInputStream$(OBJ): $(RTXSRCDIR)$(PS)OSRTBase64TextInputStream.cpp \
 $(RTXSRCDIR)$(PS)OSRTBase64TextInputStream.h $(RTXSRCDIR)$(PS)OSRTInputStream.h \
 $(RTXSRCDIR)$(PS)OSRTInputStreamIF.h $(RTXSRCDIR)$(PS)OSRTStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)OSRTStream.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h $(RTXSRCDIR)$(PS)rtxStreamBase64Text.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTBase64TextInputStream.cpp


$(OBJDIR)$(PS)OSRTInputStream$(OBJ): $(RTXSRCDIR)$(PS)OSRTInputStream.cpp $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)OSRTInputStream.h \
 $(RTXSRCDIR)$(PS)OSRTInputStreamIF.h $(RTXSRCDIR)$(PS)OSRTStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)OSRTStream.h $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTInputStream.cpp


$(OBJDIR)$(PS)OSRTMemBuf$(OBJ): $(RTXSRCDIR)$(PS)OSRTMemBuf.cpp $(RTXSRCDIR)$(PS)OSRTMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTMemBuf.cpp


$(OBJDIR)$(PS)OSRTMemoryInputStream$(OBJ): $(RTXSRCDIR)$(PS)OSRTMemoryInputStream.cpp \
 $(RTXSRCDIR)$(PS)rtxStreamMemory.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)OSRTMemoryInputStream.h \
 $(RTXSRCDIR)$(PS)OSRTInputStream.h $(RTXSRCDIR)$(PS)OSRTInputStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTStreamIF.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)OSRTStream.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTMemoryInputStream.cpp


$(OBJDIR)$(PS)OSRTMemoryOutputStream$(OBJ): $(RTXSRCDIR)$(PS)OSRTMemoryOutputStream.cpp \
 $(RTXSRCDIR)$(PS)rtxStreamMemory.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)OSRTMemoryOutputStream.h \
 $(RTXSRCDIR)$(PS)OSRTOutputStream.h $(RTXSRCDIR)$(PS)OSRTOutputStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTStreamIF.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)OSRTStream.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTMemoryOutputStream.cpp


$(OBJDIR)$(PS)OSRTMsgBuf$(OBJ): $(RTXSRCDIR)$(PS)OSRTMsgBuf.cpp $(RTXSRCDIR)$(PS)OSRTMsgBuf.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h $(RTXSRCDIR)$(PS)OSRTMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTMsgBuf.cpp


$(OBJDIR)$(PS)OSRTOutputStream$(OBJ): $(RTXSRCDIR)$(PS)OSRTOutputStream.cpp $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)OSRTOutputStream.h $(RTXSRCDIR)$(PS)OSRTOutputStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTStreamIF.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)OSRTStream.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTOutputStream.cpp


$(OBJDIR)$(PS)OSRTSocket$(OBJ): $(RTXSRCDIR)$(PS)OSRTSocket.cpp $(RTXSRCDIR)$(PS)OSRTSocket.h \
 $(RTXSRCDIR)$(PS)rtxSocket.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTSocket.cpp


$(OBJDIR)$(PS)OSRTSocketInputStream$(OBJ): $(RTXSRCDIR)$(PS)OSRTSocketInputStream.cpp \
 $(RTXSRCDIR)$(PS)rtxStreamSocket.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxSocket.h \
 $(RTXSRCDIR)$(PS)OSRTSocketInputStream.h $(RTXSRCDIR)$(PS)OSRTSocket.h \
 $(RTXSRCDIR)$(PS)OSRTInputStream.h $(RTXSRCDIR)$(PS)OSRTInputStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTStreamIF.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)OSRTStream.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTSocketInputStream.cpp


$(OBJDIR)$(PS)OSRTSocketOutputStream$(OBJ): $(RTXSRCDIR)$(PS)OSRTSocketOutputStream.cpp \
 $(RTXSRCDIR)$(PS)rtxStreamSocket.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxSocket.h \
 $(RTXSRCDIR)$(PS)OSRTSocketOutputStream.h $(RTXSRCDIR)$(PS)OSRTSocket.h \
 $(RTXSRCDIR)$(PS)OSRTOutputStream.h $(RTXSRCDIR)$(PS)OSRTOutputStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTStreamIF.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)OSRTStream.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTSocketOutputStream.cpp


$(OBJDIR)$(PS)OSRTStream$(OBJ): $(RTXSRCDIR)$(PS)OSRTStream.cpp $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)OSRTStream.h $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)OSRTStreamIF.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTStream.cpp


$(OBJDIR)$(PS)OSRTString$(OBJ): $(RTXSRCDIR)$(PS)OSRTString.cpp $(RTXSRCDIR)$(PS)OSRTString.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)rtxPrint.h $(RTXSRCDIR)$(PS)rtxHexDump.h $(RTXSRCDIR)$(PS)OSRTStringIF.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTString.cpp


$(OBJDIR)$(PS)OSRTStringConst$(OBJ): $(RTXSRCDIR)$(PS)OSRTStringConst.cpp \
 $(RTXSRCDIR)$(PS)OSRTStringConst.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTStringConst.cpp


$(OBJDIR)$(PS)OSRTStringTokenizer$(OBJ): $(RTXSRCDIR)$(PS)OSRTStringTokenizer.cpp \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)OSRTStringTokenizer.h $(RTXSRCDIR)$(PS)OSRTVoidPtrList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTStringTokenizer.cpp


$(OBJDIR)$(PS)OSRTVoidPtrList$(OBJ): $(RTXSRCDIR)$(PS)OSRTVoidPtrList.cpp \
 $(RTXSRCDIR)$(PS)OSRTVoidPtrList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)OSRTVoidPtrList.cpp


$(OBJDIR)$(PS)rtxCppAnyAttr$(OBJ): $(RTXSRCDIR)$(PS)rtxCppAnyAttr.cpp $(RTXSRCDIR)$(PS)rtxCppAnyAttr.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)OSRTBaseType.h $(RTXSRCDIR)$(PS)OSRTContext.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCppAnyAttr.cpp


$(OBJDIR)$(PS)rtxCppAnyElement$(OBJ): $(RTXSRCDIR)$(PS)rtxCppAnyElement.cpp \
 $(RTXSRCDIR)$(PS)rtxCppAnyElement.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)OSRTBaseType.h \
 $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxPrint.h $(RTXSRCDIR)$(PS)rtxHexDump.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCppAnyElement.cpp


$(OBJDIR)$(PS)rtxCppBitString$(OBJ): $(RTXSRCDIR)$(PS)rtxCppBitString.cpp $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxErrCodes.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCppBitString.cpp


$(OBJDIR)$(PS)rtxCppDList$(OBJ): $(RTXSRCDIR)$(PS)rtxCppDList.cpp $(RTXSRCDIR)$(PS)rtxCppDList.h \
 $(RTXSRCDIR)$(PS)OSRTBaseType.h $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCppDList.cpp


$(OBJDIR)$(PS)rtxCppMemory$(OBJ): $(RTXSRCDIR)$(PS)rtxCppMemory.cpp
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCppMemory.cpp


$(OBJDIR)$(PS)rtxCppBufferedInputStream$(OBJ): $(RTXSRCDIR)$(PS)rtxCppBufferedInputStream.cpp \
 $(RTXSRCDIR)$(PS)rtxStreamBuffered.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxCppBufferedInputStream.h $(RTXSRCDIR)$(PS)OSRTInputStream.h \
 $(RTXSRCDIR)$(PS)OSRTInputStreamIF.h $(RTXSRCDIR)$(PS)OSRTStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)OSRTStream.h $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCppBufferedInputStream.cpp


$(OBJDIR)$(PS)rtxCppDateTime$(OBJ): $(RTXSRCDIR)$(PS)rtxCppDateTime.cpp $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxCppDateTime.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)OSRTBaseType.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.hh $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxPrint.h \
 $(RTXSRCDIR)$(PS)rtxHexDump.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCppDateTime.cpp


$(OBJDIR)$(PS)rtxCppDynOctStr$(OBJ): $(RTXSRCDIR)$(PS)rtxCppDynOctStr.cpp $(RTXSRCDIR)$(PS)rtxBase64.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxCppDynOctStr.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)OSRTBaseType.h $(RTXSRCDIR)$(PS)OSRTContext.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCppDynOctStr.cpp


$(OBJDIR)$(PS)rtxCppXmlString$(OBJ): $(RTXSRCDIR)$(PS)rtxCppXmlString.cpp $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxCppXmlString.h $(RTXSRCDIR)$(PS)OSRTBaseType.h \
 $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)OSRTMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxPrint.h $(RTXSRCDIR)$(PS)rtxHexDump.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxXmlStr.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCppXmlString.cpp


$(OBJDIR)$(PS)rtxCppXmlStringList$(OBJ): $(RTXSRCDIR)$(PS)rtxCppXmlStringList.cpp \
 $(RTXSRCDIR)$(PS)rtxCppXmlStringList.h $(RTXSRCDIR)$(PS)rtxCppDList.h \
 $(RTXSRCDIR)$(PS)OSRTBaseType.h $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxCppXmlString.h $(RTXSRCDIR)$(PS)OSRTMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxPrint.h $(RTXSRCDIR)$(PS)rtxHexDump.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxXmlStr.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCppXmlStringList.cpp


$(OBJDIR)$(PS)rtxCppXmlSTLString$(OBJ): $(RTXSRCDIR)$(PS)rtxCppXmlSTLString.cpp
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCppXmlSTLString.cpp


$(OBJDIR)$(PS)rtxCppXmlSTLStringList$(OBJ): $(RTXSRCDIR)$(PS)rtxCppXmlSTLStringList.cpp
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxCppXmlSTLStringList.cpp


$(OBJDIR)$(PS)wceAddon$(OBJ): $(RTXSRCDIR)$(PS)wceAddon.c
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)wceAddon.c


$(OBJDIR)$(PS)rtxRegExp$(OBJ): $(RTXSRCDIR)$(PS)rtxRegExp.c $(RTXSRCDIR)$(PS)rtxRegExp.hh \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxRegExp.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUnicode.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxRegExp.c


$(OBJDIR)$(PS)rtxRegExp2$(OBJ): $(RTXSRCDIR)$(PS)rtxRegExp2.c $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxRegExp.hh $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxRegExp.h $(RTXSRCDIR)$(PS)rtxUnicode.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxRegExp2.c


$(OBJDIR)$(PS)rtxRegExpPrint$(OBJ): $(RTXSRCDIR)$(PS)rtxRegExpPrint.c $(RTXSRCDIR)$(PS)rtxRegExp.hh \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxRegExp.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUnicode.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxRegExpPrint.c


$(OBJDIR)$(PS)rtxUnicode$(OBJ): $(RTXSRCDIR)$(PS)rtxUnicode.c $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxUnicode.h
	$(CC) $(RTXCFLAGS) -c $(IPATHS) $(OBJOUT) $(RTXSRCDIR)$(PS)rtxUnicode.c

