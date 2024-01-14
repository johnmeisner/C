# C to object file compilation rules

$(OBJDIR)$(PS)rtJsonAsn1DecNull$(OBJ): $(JSONSRCDIR)/rtJsonAsn1DecNull.c \
 $(JSONSRCDIR)/asn1json.h $(RTSRCDIR)/asn1type.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxUTF8.h $(RTSRCDIR)/asn1tag.h \
 $(RTSRCDIR)/asn1ErrCodes.h $(RTSRCDIR)/rtExternDefs.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTSRCDIR)/rtContext.h $(RTSRCDIR)/asn1CEvtHndlr.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUtil.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonAsn1DecNull.c

$(OBJDIR)$(PS)rtJsonAsn1DecOID$(OBJ): $(JSONSRCDIR)/rtJsonAsn1DecOID.c \
 $(JSONSRCDIR)/asn1json.h $(RTSRCDIR)/asn1type.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxUTF8.h $(RTSRCDIR)/asn1tag.h \
 $(RTSRCDIR)/asn1ErrCodes.h $(RTSRCDIR)/rtExternDefs.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTSRCDIR)/rtContext.h $(RTSRCDIR)/asn1CEvtHndlr.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUtil.h $(RTXSRCDIR)/rtxCtype.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonAsn1DecOID.c

$(OBJDIR)$(PS)rtJsonAsn1EncOID$(OBJ): $(JSONSRCDIR)/rtJsonAsn1EncOID.c \
 $(JSONSRCDIR)/asn1json.h $(RTSRCDIR)/asn1type.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxUTF8.h $(RTSRCDIR)/asn1tag.h \
 $(RTSRCDIR)/asn1ErrCodes.h $(RTSRCDIR)/rtExternDefs.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTSRCDIR)/rtContext.h $(RTSRCDIR)/asn1CEvtHndlr.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUtil.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonAsn1EncOID.c

$(OBJDIR)$(PS)rtJsonAsn1EncOpenType$(OBJ): $(JSONSRCDIR)/rtJsonAsn1EncOpenType.c \
 $(JSONSRCDIR)/asn1json.h $(RTSRCDIR)/asn1type.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxUTF8.h $(RTSRCDIR)/asn1tag.h \
 $(RTSRCDIR)/asn1ErrCodes.h $(RTSRCDIR)/rtExternDefs.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTSRCDIR)/rtContext.h $(RTSRCDIR)/asn1CEvtHndlr.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUtil.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonAsn1EncOpenType.c

$(OBJDIR)$(PS)rtJsonDecAny$(OBJ): $(JSONSRCDIR)/rtJsonDecAny.c $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxIntStack.h $(RTXSRCDIR)/rtxMemBuf.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecAny.c

$(OBJDIR)$(PS)rtJsonDecBase64Str$(OBJ): $(JSONSRCDIR)/rtJsonDecBase64Str.c \
 $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h \
 $(RTXSRCDIR)/rtxBase64.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecBase64Str.c

$(OBJDIR)$(PS)rtJsonDecBitStr$(OBJ): $(JSONSRCDIR)/rtJsonDecBitStr.c $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxCommon.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecBitStr.c

$(OBJDIR)$(PS)rtJsonDecBool$(OBJ): $(JSONSRCDIR)/rtJsonDecBool.c $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxCommon.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecBool.c

$(OBJDIR)$(PS)rtJsonDecDateTime$(OBJ): $(JSONSRCDIR)/rtJsonDecDateTime.c \
 $(RTXSRCDIR)/rtxCtype.h $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxFile.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h \
 $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecDateTime.c

$(OBJDIR)$(PS)rtJsonDecDecimal$(OBJ): $(JSONSRCDIR)/rtJsonDecDecimal.c $(RTXSRCDIR)/rtxCtype.h \
 $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecDecimal.c

$(OBJDIR)$(PS)rtJsonDecDouble$(OBJ): $(JSONSRCDIR)/rtJsonDecDouble.c $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxCommon.h \
 $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxFile.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecDouble.c

$(OBJDIR)$(PS)rtJsonDecDynBase64Str$(OBJ): $(JSONSRCDIR)/rtJsonDecDynBase64Str.c \
 $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxBase64.h $(RTXSRCDIR)/rtxCtype.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxFile.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h \
 $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecDynBase64Str.c

$(OBJDIR)$(PS)rtJsonDecDynHexStr$(OBJ): $(JSONSRCDIR)/rtJsonDecDynHexStr.c \
 $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h \
 $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecDynHexStr.c

$(OBJDIR)$(PS)rtJsonDecHexStr$(OBJ): $(JSONSRCDIR)/rtJsonDecHexStr.c $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxCommon.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecHexStr.c

$(OBJDIR)$(PS)rtJsonDecInt16$(OBJ): $(JSONSRCDIR)/rtJsonDecInt16.c $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecInt16.c

$(OBJDIR)$(PS)rtJsonDecInt32$(OBJ): $(JSONSRCDIR)/rtJsonDecInt32.c $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecInt32.c

$(OBJDIR)$(PS)rtJsonDecInt64$(OBJ): $(JSONSRCDIR)/rtJsonDecInt64.c $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecInt64.c

$(OBJDIR)$(PS)rtJsonDecInt8$(OBJ): $(JSONSRCDIR)/rtJsonDecInt8.c $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecInt8.c

$(OBJDIR)$(PS)rtJsonDecNumberString$(OBJ): $(JSONSRCDIR)/rtJsonDecNumberString.c \
 $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h $(RTXSRCDIR)/rtxCtype.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecNumberString.c

$(OBJDIR)$(PS)rtJsonDecString$(OBJ): $(JSONSRCDIR)/rtJsonDecString.c $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemBuf.h \
 $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h \
 $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxFile.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h \
 $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecString.c

$(OBJDIR)$(PS)rtJsonDecUCS2String$(OBJ): $(JSONSRCDIR)/rtJsonDecUCS2String.c \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h \
 $(JSONSRCDIR)/rtJsonDecUTF.c $(RTXSRCDIR)/rtxMemBuf.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecUCS2String.c

$(OBJDIR)$(PS)rtJsonDecUCS4String$(OBJ): $(JSONSRCDIR)/rtJsonDecUCS4String.c \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h \
 $(JSONSRCDIR)/rtJsonDecUTF.c $(RTXSRCDIR)/rtxMemBuf.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecUCS4String.c

$(OBJDIR)$(PS)rtJsonDecUInt16$(OBJ): $(JSONSRCDIR)/rtJsonDecUInt16.c $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecUInt16.c

$(OBJDIR)$(PS)rtJsonDecUInt32$(OBJ): $(JSONSRCDIR)/rtJsonDecUInt32.c $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecUInt32.c

$(OBJDIR)$(PS)rtJsonDecUInt64$(OBJ): $(JSONSRCDIR)/rtJsonDecUInt64.c $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCtype.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecUInt64.c

$(OBJDIR)$(PS)rtJsonDecUInt8$(OBJ): $(JSONSRCDIR)/rtJsonDecUInt8.c $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecUInt8.c

$(OBJDIR)$(PS)rtJsonDecUTF$(OBJ): $(JSONSRCDIR)/rtJsonDecUTF.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonDecUTF.c

$(OBJDIR)$(PS)rtJsonEncAnyAttr$(OBJ): $(JSONSRCDIR)/rtJsonEncAnyAttr.c \
 $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncAnyAttr.c

$(OBJDIR)$(PS)rtJsonEncBase64Binary$(OBJ): $(JSONSRCDIR)/rtJsonEncBase64Binary.c \
 $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncBase64Binary.c

$(OBJDIR)$(PS)rtJsonEncBitStr$(OBJ): $(JSONSRCDIR)/rtJsonEncBitStr.c $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxCommon.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncBitStr.c

$(OBJDIR)$(PS)rtJsonEncBool$(OBJ): $(JSONSRCDIR)/rtJsonEncBool.c $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxCommon.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncBool.c

$(OBJDIR)$(PS)rtJsonEncDateTime$(OBJ): $(JSONSRCDIR)/rtJsonEncDateTime.c \
 $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h \
 $(RTXSRCDIR)/rtxDateTime.hh $(RTXSRCDIR)/rtxCtype.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncDateTime.c

$(OBJDIR)$(PS)rtJsonEncDates$(OBJ): $(JSONSRCDIR)/rtJsonEncDates.c $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxDateTime.hh $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxSList.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncDates.c

$(OBJDIR)$(PS)rtJsonEncDecimal$(OBJ): $(JSONSRCDIR)/rtJsonEncDecimal.c \
 $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncDecimal.c

$(OBJDIR)$(PS)rtJsonEncDouble$(OBJ): $(JSONSRCDIR)/rtJsonEncDouble.c $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxCommon.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncDouble.c

$(OBJDIR)$(PS)rtJsonEncEdges$(OBJ): $(JSONSRCDIR)/rtJsonEncEdges.c $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxCommon.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncEdges.c

$(OBJDIR)$(PS)rtJsonEncFloat$(OBJ): $(JSONSRCDIR)/rtJsonEncFloat.c \
 $(JSONSRCDIR)/rtJsonEncDouble.c $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxCommon.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncFloat.c

$(OBJDIR)$(PS)rtJsonEncHexBinary$(OBJ): $(JSONSRCDIR)/rtJsonEncHexBinary.c \
 $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncHexBinary.c

$(OBJDIR)$(PS)rtJsonEncIndent$(OBJ): $(JSONSRCDIR)/rtJsonEncIndent.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncIndent.c

$(OBJDIR)$(PS)rtJsonEncInt$(OBJ): $(JSONSRCDIR)/rtJsonEncInt.c $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncInt.c

$(OBJDIR)$(PS)rtJsonEncInt64$(OBJ): $(JSONSRCDIR)/rtJsonEncInt64.c \
 $(JSONSRCDIR)/rtJsonEncInt.c $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/rtxCommon.h \
 $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxDateTime.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxFile.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h \
 $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncInt64.c

$(OBJDIR)$(PS)rtJsonEncString$(OBJ): $(JSONSRCDIR)/rtJsonEncString.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncString.c

$(OBJDIR)$(PS)rtJsonEncUCS4String$(OBJ): $(JSONSRCDIR)/rtJsonEncUCS4String.c \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h \
 $(JSONSRCDIR)/rtJsonEncUTF.c
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncUCS4String.c

$(OBJDIR)$(PS)rtJsonEncUInt$(OBJ): $(JSONSRCDIR)/rtJsonEncUInt.c $(JSONSRCDIR)/rtJsonEncInt.c \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/rtxCommon.h \
 $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxDateTime.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxFile.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h \
 $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncUInt.c

$(OBJDIR)$(PS)rtJsonEncUInt64$(OBJ): $(JSONSRCDIR)/rtJsonEncUInt64.c \
 $(JSONSRCDIR)/rtJsonEncInt.c $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/rtxCommon.h \
 $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxDateTime.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxFile.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h \
 $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncUInt64.c

$(OBJDIR)$(PS)rtJsonEncUTF$(OBJ): $(JSONSRCDIR)/rtJsonEncUTF.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncUTF.c

$(OBJDIR)$(PS)rtJsonEncUnicode$(OBJ): $(JSONSRCDIR)/rtJsonEncUnicode.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(JSONSRCDIR)/rtJsonExternDefs.h $(JSONSRCDIR)/rtJsonEncUTF.c
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonEncUnicode.c

$(OBJDIR)$(PS)rtJsonParser$(OBJ): $(JSONSRCDIR)/rtJsonParser.c $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxCtype.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxMemBuf.h \
 $(RTXSRCDIR)/rtxUTF8.h $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/rtxCommon.h \
 $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxFile.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/rtJsonParser.c
