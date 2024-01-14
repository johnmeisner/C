# C to object file compilation rules

$(OBJDIR)$(PS)rtJsonParser$(OBJ): $(JSONSRCDIR)$(PS)rtJsonParser.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(JSONSRCDIR)$(PS)osrtjson.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonParser.c


$(OBJDIR)$(PS)rtJsonDecAny$(OBJ): $(JSONSRCDIR)$(PS)rtJsonDecAny.c $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxIntStack.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonDecAny.c


$(OBJDIR)$(PS)rtJsonDecBase64Str$(OBJ): $(JSONSRCDIR)$(PS)rtJsonDecBase64Str.c \
 $(JSONSRCDIR)$(PS)osrtjson.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h $(RTXSRCDIR)$(PS)rtxBase64.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonDecBase64Str.c


$(OBJDIR)$(PS)rtJsonDecDynBase64Str$(OBJ): $(JSONSRCDIR)$(PS)rtJsonDecDynBase64Str.c \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBase64.h $(RTXSRCDIR)$(PS)rtxCtype.h $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonDecDynBase64Str.c


$(OBJDIR)$(PS)rtJsonDecBitStr$(OBJ): $(JSONSRCDIR)$(PS)rtJsonDecBitStr.c $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonDecBitStr.c


$(OBJDIR)$(PS)rtJsonDecBool$(OBJ): $(JSONSRCDIR)$(PS)rtJsonDecBool.c $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonDecBool.c


$(OBJDIR)$(PS)rtJsonDecDateTime$(OBJ): $(JSONSRCDIR)$(PS)rtJsonDecDateTime.c \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(JSONSRCDIR)$(PS)osrtjson.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonDecDateTime.c


$(OBJDIR)$(PS)rtJsonDecDecimal$(OBJ): $(JSONSRCDIR)$(PS)rtJsonDecDecimal.c $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(JSONSRCDIR)$(PS)osrtjson.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonDecDecimal.c


$(OBJDIR)$(PS)rtJsonDecDouble$(OBJ): $(JSONSRCDIR)$(PS)rtJsonDecDouble.c $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(JSONSRCDIR)$(PS)osrtjson.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonDecDouble.c


$(OBJDIR)$(PS)rtJsonDecDynHexStr$(OBJ): $(JSONSRCDIR)$(PS)rtJsonDecDynHexStr.c \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(JSONSRCDIR)$(PS)osrtjson.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonDecDynHexStr.c


$(OBJDIR)$(PS)rtJsonDecHexStr$(OBJ): $(JSONSRCDIR)$(PS)rtJsonDecHexStr.c $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonDecHexStr.c


$(OBJDIR)$(PS)rtJsonDecNull$(OBJ): $(JSONSRCDIR)$(PS)rtJsonDecNull.c $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonDecNull.c


$(OBJDIR)$(PS)rtJsonDecNumberString$(OBJ): $(JSONSRCDIR)$(PS)rtJsonDecNumberString.c \
 $(JSONSRCDIR)$(PS)osrtjson.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonDecNumberString.c


$(OBJDIR)$(PS)rtJsonDecString$(OBJ): $(JSONSRCDIR)$(PS)rtJsonDecString.c $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(JSONSRCDIR)$(PS)osrtjson.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonDecString.c


$(OBJDIR)$(PS)rtJsonDecUCS2String$(OBJ): $(JSONSRCDIR)$(PS)rtJsonDecUCS2String.c \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h $(JSONSRCDIR)$(PS)rtJsonDecUTF.c \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonDecUCS2String.c


$(OBJDIR)$(PS)rtJsonDecUCS4String$(OBJ): $(JSONSRCDIR)$(PS)rtJsonDecUCS4String.c \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h $(JSONSRCDIR)$(PS)rtJsonDecUTF.c \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonDecUCS4String.c


$(OBJDIR)$(PS)rtJsonEncAnyAttr$(OBJ): $(JSONSRCDIR)$(PS)rtJsonEncAnyAttr.c \
 $(JSONSRCDIR)$(PS)osrtjson.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonEncAnyAttr.c


$(OBJDIR)$(PS)rtJsonEncBase64Binary$(OBJ): $(JSONSRCDIR)$(PS)rtJsonEncBase64Binary.c \
 $(JSONSRCDIR)$(PS)osrtjson.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonEncBase64Binary.c


$(OBJDIR)$(PS)rtJsonEncBitStr$(OBJ): $(JSONSRCDIR)$(PS)rtJsonEncBitStr.c $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonEncBitStr.c


$(OBJDIR)$(PS)rtJsonEncBool$(OBJ): $(JSONSRCDIR)$(PS)rtJsonEncBool.c $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonEncBool.c


$(OBJDIR)$(PS)rtJsonEncDates$(OBJ): $(JSONSRCDIR)$(PS)rtJsonEncDates.c $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.hh $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonEncDates.c


$(OBJDIR)$(PS)rtJsonEncDateTime$(OBJ): $(JSONSRCDIR)$(PS)rtJsonEncDateTime.c \
 $(JSONSRCDIR)$(PS)osrtjson.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h $(RTXSRCDIR)$(PS)rtxDateTime.hh \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonEncDateTime.c


$(OBJDIR)$(PS)rtJsonEncDecimal$(OBJ): $(JSONSRCDIR)$(PS)rtJsonEncDecimal.c \
 $(JSONSRCDIR)$(PS)osrtjson.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonEncDecimal.c


$(OBJDIR)$(PS)rtJsonEncDouble$(OBJ): $(JSONSRCDIR)$(PS)rtJsonEncDouble.c $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonEncDouble.c


$(OBJDIR)$(PS)rtJsonEncFloat$(OBJ): $(JSONSRCDIR)$(PS)rtJsonEncFloat.c \
 $(JSONSRCDIR)$(PS)rtJsonEncDouble.c $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonEncFloat.c


$(OBJDIR)$(PS)rtJsonEncHexBinary$(OBJ): $(JSONSRCDIR)$(PS)rtJsonEncHexBinary.c \
 $(JSONSRCDIR)$(PS)osrtjson.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonEncHexBinary.c


$(OBJDIR)$(PS)rtJsonEncIndent$(OBJ): $(JSONSRCDIR)$(PS)rtJsonEncIndent.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(JSONSRCDIR)$(PS)osrtjson.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonEncIndent.c


$(OBJDIR)$(PS)rtJsonEncString$(OBJ): $(JSONSRCDIR)$(PS)rtJsonEncString.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonEncString.c


$(OBJDIR)$(PS)rtJsonEncUCS4String$(OBJ): $(JSONSRCDIR)$(PS)rtJsonEncUCS4String.c \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h $(JSONSRCDIR)$(PS)rtJsonEncUTF.c
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonEncUCS4String.c


$(OBJDIR)$(PS)rtJsonEncUnicode$(OBJ): $(JSONSRCDIR)$(PS)rtJsonEncUnicode.c $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(JSONSRCDIR)$(PS)osrtjson.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h $(JSONSRCDIR)$(PS)rtJsonEncUTF.c
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonEncUnicode.c


$(OBJDIR)$(PS)rtJsonEncEdges$(OBJ): $(JSONSRCDIR)$(PS)rtJsonEncEdges.c $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonEncEdges.c


$(OBJDIR)$(PS)OSJSONMessageBuffer$(OBJ): $(JSONSRCDIR)$(PS)OSJSONMessageBuffer.cpp \
 $(JSONSRCDIR)$(PS)osrtjson.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h $(JSONSRCDIR)$(PS)OSJSONMessageBuffer.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBuf.h $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h
	$(CCC) $(CCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)OSJSONMessageBuffer.cpp


$(OBJDIR)$(PS)OSJSONEncodeBuffer$(OBJ): $(JSONSRCDIR)$(PS)OSJSONEncodeBuffer.cpp \
 $(JSONSRCDIR)$(PS)OSJSONEncodeBuffer.h $(JSONSRCDIR)$(PS)OSJSONMessageBuffer.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBuf.h $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h \
 $(JSONSRCDIR)$(PS)osrtjson.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CCC) $(CCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)OSJSONEncodeBuffer.cpp


$(OBJDIR)$(PS)OSJSONDecodeBuffer$(OBJ): $(JSONSRCDIR)$(PS)OSJSONDecodeBuffer.cpp \
 $(RTXSRCDIR)$(PS)OSRTFileInputStream.h $(RTXSRCDIR)$(PS)OSRTInputStream.h \
 $(RTXSRCDIR)$(PS)OSRTInputStreamIF.h $(RTXSRCDIR)$(PS)OSRTStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)OSRTStream.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h $(RTXSRCDIR)$(PS)OSRTMemoryInputStream.h \
 $(JSONSRCDIR)$(PS)OSJSONDecodeBuffer.h $(JSONSRCDIR)$(PS)OSJSONMessageBuffer.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBuf.h $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CCC) $(CCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)OSJSONDecodeBuffer.cpp


$(OBJDIR)$(PS)OSJSONEncodeStream$(OBJ): $(JSONSRCDIR)$(PS)OSJSONEncodeStream.cpp \
 $(JSONSRCDIR)$(PS)OSJSONEncodeStream.h $(RTXSRCDIR)$(PS)OSRTOutputStream.h \
 $(RTXSRCDIR)$(PS)OSRTOutputStreamIF.h $(RTXSRCDIR)$(PS)OSRTStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)OSRTStream.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h $(JSONSRCDIR)$(PS)OSJSONMessageBuffer.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBuf.h $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h
	$(CCC) $(CCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)OSJSONEncodeStream.cpp


$(OBJDIR)$(PS)rtJsonAsn1EncOID$(OBJ): $(JSONSRCDIR)$(PS)rtJsonAsn1EncOID.c \
 $(JSONSRCDIR)$(PS)asn1json.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonAsn1EncOID.c


$(OBJDIR)$(PS)rtJsonAsn1DecExt$(OBJ): $(JSONSRCDIR)$(PS)rtJsonAsn1DecExt.c \
 $(JSONSRCDIR)$(PS)asn1json.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonAsn1DecExt.c


$(OBJDIR)$(PS)rtJsonAsn1DecOID$(OBJ): $(JSONSRCDIR)$(PS)rtJsonAsn1DecOID.c \
 $(JSONSRCDIR)$(PS)asn1json.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonAsn1DecOID.c


$(OBJDIR)$(PS)rtJsonAsn1EncOpenType$(OBJ): $(JSONSRCDIR)$(PS)rtJsonAsn1EncOpenType.c \
 $(JSONSRCDIR)$(PS)asn1json.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonAsn1EncOpenType.c


$(OBJDIR)$(PS)rtJsonAsn1DecReal$(OBJ): $(JSONSRCDIR)$(PS)rtJsonAsn1DecReal.c \
 $(JSONSRCDIR)$(PS)asn1json.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonAsn1DecReal.c


$(OBJDIR)$(PS)rtJsonAsn1EncReal$(OBJ): $(JSONSRCDIR)$(PS)rtJsonAsn1EncReal.c \
 $(JSONSRCDIR)$(PS)asn1json.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(JSONSRCDIR)$(PS)rtJsonExternDefs.h $(JSONSRCDIR)$(PS)osrtjson.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxText.h \
 $(RTXSRCDIR)$(PS)rtxContext.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)$(PS)rtJsonAsn1EncReal.c

