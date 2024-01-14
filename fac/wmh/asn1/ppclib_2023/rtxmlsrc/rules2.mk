# C/C++ to object file compilation rules

$(OBJDIR)$(PS)rtSaxErrors$(OBJ): $(XMLSRCDIR)$(PS)rtSaxErrors.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtSaxErrors.c

$(OBJDIR)$(PS)rtXmlError$(OBJ): $(XMLSRCDIR)$(PS)rtXmlError.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlError.c

$(OBJDIR)$(PS)rtXmlCheckBuffer$(OBJ): $(XMLSRCDIR)$(PS)rtXmlCheckBuffer.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlCheckBuffer.c

$(OBJDIR)$(PS)rtXmlContextInit$(OBJ): $(XMLSRCDIR)$(PS)rtXmlContextInit.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlContextInit.c

$(OBJDIR)$(PS)rtXmlCmpBase64Str$(OBJ): $(XMLSRCDIR)$(PS)rtXmlCmpBase64Str.c $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)osrtxml.hh
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlCmpBase64Str.c

$(OBJDIR)$(PS)rtXmlCmpHexStr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlCmpHexStr.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlCmpHexStr.c

$(OBJDIR)$(PS)rtXmlCmpQName$(OBJ): $(XMLSRCDIR)$(PS)rtXmlCmpQName.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlCmpQName.c

$(OBJDIR)$(PS)rtXmlDecBase64Binary$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecBase64Binary.c \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecBase64Binary.c

$(OBJDIR)$(PS)rtXmlDecBase64Common$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecBase64Common.c \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecBase64Common.c

$(OBJDIR)$(PS)rtXmlDecBase64StrValue$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecBase64StrValue.c \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecBase64StrValue.c

$(OBJDIR)$(PS)rtXmlDecBase64Str$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecBase64Str.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecBase64Str.c

$(OBJDIR)$(PS)rtXmlDecBigInt$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecBigInt.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecBigInt.c

$(OBJDIR)$(PS)rtXmlDecBool$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecBool.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecBool.c

$(OBJDIR)$(PS)rtXmlDecDateTime$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecDateTime.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecDateTime.c

$(OBJDIR)$(PS)rtXmlDecDates$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecDates.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecDates.c

$(OBJDIR)$(PS)rtXmlDecDecimal$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecDecimal.c $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecDecimal.c

$(OBJDIR)$(PS)rtXmlDecDouble$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecDouble.c $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecDouble.c

$(OBJDIR)$(PS)rtXmlDecDynBase64Str$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecDynBase64Str.c \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecDynBase64Str.c

$(OBJDIR)$(PS)rtXmlDecDynHexStr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecDynHexStr.c $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecDynHexStr.c

$(OBJDIR)$(PS)rtXmlDecHexBinary$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecHexBinary.c \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecHexBinary.c

$(OBJDIR)$(PS)rtXmlDecHexStr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecHexStr.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecHexStr.c

$(OBJDIR)$(PS)rtXmlDecHexStrValue$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecHexStrValue.c \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecHexStrValue.c

$(OBJDIR)$(PS)rtXmlDecInt64$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecInt64.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecInt64.c

$(OBJDIR)$(PS)rtXmlDecNSAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecNSAttr.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecNSAttr.c

$(OBJDIR)$(PS)rtXmlDecQName$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecQName.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecQName.c

$(OBJDIR)$(PS)rtXmlDecUInt$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecUInt.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecUInt.c

$(OBJDIR)$(PS)rtXmlDecUInt64$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecUInt64.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecUInt64.c

$(OBJDIR)$(PS)rtXmlDecXSIAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecXSIAttr.c $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecXSIAttr.c

$(OBJDIR)$(PS)rtXmlDecXSIAttrs$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecXSIAttrs.c \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecXSIAttrs.c

$(OBJDIR)$(PS)rtXmlEncAny$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncAny.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxStreamMemory.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(XMLSRCDIR)$(PS)rtXmlPull.hh $(XMLSRCDIR)$(PS)rtXmlPull.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncAny.c

$(OBJDIR)$(PS)rtXmlEncAnyTypeValue$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncAnyTypeValue.c \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)osrtxml.hh
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncAnyTypeValue.c

$(OBJDIR)$(PS)rtXmlEncAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncAttr.c $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncAttr.c

$(OBJDIR)$(PS)rtXmlEncAttrC14N$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncAttrC14N.c \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncAttrC14N.c

$(OBJDIR)$(PS)rtXmlEncBase64Binary$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncBase64Binary.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncBase64Binary.c

$(OBJDIR)$(PS)rtXmlEncBase64BinaryAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncBase64BinaryAttr.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncBase64BinaryAttr.c

$(OBJDIR)$(PS)rtXmlEncBigInt$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncBigInt.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncBigInt.c

$(OBJDIR)$(PS)rtXmlEncBigIntValue$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncBigIntValue.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncBigIntValue.c

$(OBJDIR)$(PS)rtXmlEncBigIntAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncBigIntAttr.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncBigIntAttr.c

$(OBJDIR)$(PS)rtXmlEncBitString$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncBitString.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncBitString.c

$(OBJDIR)$(PS)rtXmlEncBool$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncBool.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncBool.c

$(OBJDIR)$(PS)rtXmlEncBoolAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncBoolAttr.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncBoolAttr.c

$(OBJDIR)$(PS)rtXmlEncComment$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncComment.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncComment.c

$(OBJDIR)$(PS)rtXmlEncDates$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncDates.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlEncDateTmpl.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncDates.c

$(OBJDIR)$(PS)rtXmlEncDatesValue$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncDatesValue.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.hh $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncDatesValue.c

$(OBJDIR)$(PS)rtXmlEncDateTime$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncDateTime.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlEncDateTmpl.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncDateTime.c

$(OBJDIR)$(PS)rtXmlEncDateTimeValue$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncDateTimeValue.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.hh $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncDateTimeValue.c

$(OBJDIR)$(PS)rtXmlEncDecimal$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncDecimal.c $(RTXSRCDIR)$(PS)rtxRegExp.hh \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxRegExp.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUnicode.h $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncDecimal.c

$(OBJDIR)$(PS)rtXmlEncDecimalValue$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncDecimalValue.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncDecimalValue.c

$(OBJDIR)$(PS)rtXmlEncDecimalAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncDecimalAttr.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncDecimalAttr.c

$(OBJDIR)$(PS)rtXmlEncDecimalPattern$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncDecimalPattern.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncDecimalPattern.c

$(OBJDIR)$(PS)rtXmlEncDouble$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncDouble.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncDouble.c

$(OBJDIR)$(PS)rtXmlEncDoubleAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncDoubleAttr.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncDoubleAttr.c

$(OBJDIR)$(PS)rtXmlEncDoublePattern$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncDoublePattern.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(RTXSRCDIR)$(PS)rtxRegExp.hh $(RTXSRCDIR)$(PS)rtxRegExp.h $(RTXSRCDIR)$(PS)rtxUnicode.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncDoublePattern.c

$(OBJDIR)$(PS)rtXmlEncDoubleValue$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncDoubleValue.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncDoubleValue.c

$(OBJDIR)$(PS)rtXmlEncEmptyElement$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncEmptyElement.c \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncEmptyElement.c

$(OBJDIR)$(PS)rtXmlEncEndDocument$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncEndDocument.c \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncEndDocument.c

$(OBJDIR)$(PS)rtXmlEncEndElement$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncEndElement.c \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncEndElement.c

$(OBJDIR)$(PS)rtXmlEncFloat$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncFloat.c $(XMLSRCDIR)$(PS)rtXmlEncDouble.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncFloat.c

$(OBJDIR)$(PS)rtXmlEncFloatAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncFloatAttr.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncFloatAttr.c

$(OBJDIR)$(PS)rtXmlEncHexBinary$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncHexBinary.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncHexBinary.c

$(OBJDIR)$(PS)rtXmlEncHexBinaryAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncHexBinaryAttr.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncHexBinaryAttr.c

$(OBJDIR)$(PS)rtXmlEncIndent$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncIndent.c $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncIndent.c

$(OBJDIR)$(PS)rtXmlEncInt$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncInt.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncInt.c

$(OBJDIR)$(PS)rtXmlEncIntAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncIntAttr.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncIntAttr.c

$(OBJDIR)$(PS)rtXmlEncIntPattern$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncIntPattern.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(RTXSRCDIR)$(PS)rtxRegExp.hh $(RTXSRCDIR)$(PS)rtxRegExp.h $(RTXSRCDIR)$(PS)rtxUnicode.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncIntPattern.c

$(OBJDIR)$(PS)rtXmlEncInt64$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncInt64.c $(XMLSRCDIR)$(PS)rtXmlEncInt.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncInt64.c

$(OBJDIR)$(PS)rtXmlEncInt64Attr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncInt64Attr.c \
 $(XMLSRCDIR)$(PS)rtXmlEncIntAttr.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncInt64Attr.c

$(OBJDIR)$(PS)rtXmlEncInt64Pattern$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncInt64Pattern.c \
 $(XMLSRCDIR)$(PS)rtXmlEncIntPattern.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxRegExp.hh $(RTXSRCDIR)$(PS)rtxRegExp.h \
 $(RTXSRCDIR)$(PS)rtxUnicode.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncInt64Pattern.c

$(OBJDIR)$(PS)rtXmlEncNamedBits$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncNamedBits.c \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncNamedBits.c

$(OBJDIR)$(PS)rtXmlEncNSAttrs$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncNSAttrs.c $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncNSAttrs.c

$(OBJDIR)$(PS)rtXmlEncReal10$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncReal10.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncReal10.c

$(OBJDIR)$(PS)rtXmlEncSoapEnvelope$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncSoapEnvelope.c \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncSoapEnvelope.c

$(OBJDIR)$(PS)rtXmlEncSoapArrayTypeAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncSoapArrayTypeAttr.c \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncSoapArrayTypeAttr.c

$(OBJDIR)$(PS)rtXmlEncStartDocument$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncStartDocument.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxStream.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncStartDocument.c

$(OBJDIR)$(PS)rtXmlEncStartElement$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncStartElement.c \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncStartElement.c

$(OBJDIR)$(PS)rtXmlEncUInt$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncUInt.c $(XMLSRCDIR)$(PS)rtXmlEncInt.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncUInt.c

$(OBJDIR)$(PS)rtXmlEncUInt64$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncUInt64.c $(XMLSRCDIR)$(PS)rtXmlEncInt.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncUInt64.c

$(OBJDIR)$(PS)rtXmlEncUIntAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncUIntAttr.c \
 $(XMLSRCDIR)$(PS)rtXmlEncIntAttr.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncUIntAttr.c

$(OBJDIR)$(PS)rtXmlEncUInt64Attr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncUInt64Attr.c \
 $(XMLSRCDIR)$(PS)rtXmlEncIntAttr.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncUInt64Attr.c

$(OBJDIR)$(PS)rtXmlEncUIntPattern$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncUIntPattern.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(RTXSRCDIR)$(PS)rtxRegExp.hh $(RTXSRCDIR)$(PS)rtxRegExp.h $(RTXSRCDIR)$(PS)rtxUnicode.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncUIntPattern.c

$(OBJDIR)$(PS)rtXmlEncUInt64Pattern$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncUInt64Pattern.c \
 $(XMLSRCDIR)$(PS)rtXmlEncUIntPattern.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxRegExp.hh $(RTXSRCDIR)$(PS)rtxRegExp.h \
 $(RTXSRCDIR)$(PS)rtxUnicode.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncUInt64Pattern.c

$(OBJDIR)$(PS)rtXmlEncUnicode$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncUnicode.c $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncUnicode.c

$(OBJDIR)$(PS)rtXmlEncXSIAttrs$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncXSIAttrs.c \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncXSIAttrs.c

$(OBJDIR)$(PS)rtXmlEncXSINilAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncXSINilAttr.c \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncXSINilAttr.c

$(OBJDIR)$(PS)rtXmlEncXSITypeAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncXSITypeAttr.c \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncXSITypeAttr.c

$(OBJDIR)$(PS)rtXmlKeyArray$(OBJ): $(XMLSRCDIR)$(PS)rtXmlKeyArray.c $(XMLSRCDIR)$(PS)rtXmlKeyArray.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlKeyArray.c

$(OBJDIR)$(PS)rtXmlMatchHexStr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlMatchHexStr.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlMatchHexStr.c

$(OBJDIR)$(PS)rtXmlMatchBase64Str$(OBJ): $(XMLSRCDIR)$(PS)rtXmlMatchBase64Str.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlMatchBase64Str.c

$(OBJDIR)$(PS)rtXmlMatchDateTime$(OBJ): $(XMLSRCDIR)$(PS)rtXmlMatchDateTime.c \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlMatchDateTime.c

$(OBJDIR)$(PS)rtXmlMatchDates$(OBJ): $(XMLSRCDIR)$(PS)rtXmlMatchDates.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlMatchDates.c

$(OBJDIR)$(PS)rtXmlMemFree$(OBJ): $(XMLSRCDIR)$(PS)rtXmlMemFree.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlMemFree.c

$(OBJDIR)$(PS)rtXmlNamespace$(OBJ): $(XMLSRCDIR)$(PS)rtXmlNamespace.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlNamespace.c

$(OBJDIR)$(PS)rtXmlNewQName$(OBJ): $(XMLSRCDIR)$(PS)rtXmlNewQName.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlNewQName.c

$(OBJDIR)$(PS)rtXmlParseElementName$(OBJ): $(XMLSRCDIR)$(PS)rtXmlParseElementName.c \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlParseElementName.c

$(OBJDIR)$(PS)rtXmlParseElemQName$(OBJ): $(XMLSRCDIR)$(PS)rtXmlParseElemQName.c \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlParseElemQName.c

$(OBJDIR)$(PS)rtXmlPrintNSAttrs$(OBJ): $(XMLSRCDIR)$(PS)rtXmlPrintNSAttrs.c \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlPrintNSAttrs.c

$(OBJDIR)$(PS)rtXmlPutChar$(OBJ): $(XMLSRCDIR)$(PS)rtXmlPutChar.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxUTF16.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlPutChar.c

$(OBJDIR)$(PS)rtXmlSetEncodingStr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlSetEncodingStr.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlSetEncodingStr.c

$(OBJDIR)$(PS)rtXmlStrCmpAsc$(OBJ): $(XMLSRCDIR)$(PS)rtXmlStrCmpAsc.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlStrCmpAsc.c

$(OBJDIR)$(PS)rtXmlStrnCmpAsc$(OBJ): $(XMLSRCDIR)$(PS)rtXmlStrnCmpAsc.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlStrnCmpAsc.c

$(OBJDIR)$(PS)rtXmlTreatWhitespaces$(OBJ): $(XMLSRCDIR)$(PS)rtXmlTreatWhitespaces.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlTreatWhitespaces.c

$(OBJDIR)$(PS)rtXmlWriteToFile$(OBJ): $(XMLSRCDIR)$(PS)rtXmlWriteToFile.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlWriteToFile.c

$(OBJDIR)$(PS)rtXmlWriteUTF16ToFile$(OBJ): $(XMLSRCDIR)$(PS)rtXmlWriteUTF16ToFile.c \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlWriteUTF16ToFile.c

$(OBJDIR)$(PS)rtXmlpCreateReader$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpCreateReader.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpCreateReader.c

$(OBJDIR)$(PS)rtXmlpDecAny$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecAny.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.hh $(XMLSRCDIR)$(PS)rtXmlPull.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecAny.c

$(OBJDIR)$(PS)rtXmlpDecAnyElem$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecAnyElem.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.hh $(XMLSRCDIR)$(PS)rtXmlPull.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecAnyElem.c

$(OBJDIR)$(PS)rtXmlpDecAnyAttrStr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecAnyAttrStr.c \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.hh $(XMLSRCDIR)$(PS)rtXmlPull.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecAnyAttrStr.c

$(OBJDIR)$(PS)rtXmlpDecBase64Str$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecBase64Str.c \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecBase64Str.c

$(OBJDIR)$(PS)rtXmlpDecBigInt$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecBigInt.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecBigInt.c

$(OBJDIR)$(PS)rtXmlpDecBitString$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecBitString.c \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecBitString.c

$(OBJDIR)$(PS)rtXmlpDecBool$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecBool.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecBool.c

$(OBJDIR)$(PS)rtXmlpDecDateTime$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecDateTime.c \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecDateTime.c

$(OBJDIR)$(PS)rtXmlpDecDecimal$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecDecimal.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecDecimal.c

$(OBJDIR)$(PS)rtXmlpDecDouble$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecDouble.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecDouble.c

$(OBJDIR)$(PS)rtXmlpDecDynBase64Str$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecDynBase64Str.c \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecDynBase64Str.c

$(OBJDIR)$(PS)rtXmlpDecDynHexStr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecDynHexStr.c \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecDynHexStr.c

$(OBJDIR)$(PS)rtXmlpDecHexStr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecHexStr.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecHexStr.c

$(OBJDIR)$(PS)rtXmlpDecInt$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecInt.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecInt.c

$(OBJDIR)$(PS)rtXmlpDecInt64$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecInt64.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecInt64.c

$(OBJDIR)$(PS)rtXmlpDecNamedBits$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecNamedBits.c \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecNamedBits.c

$(OBJDIR)$(PS)rtXmlpDecStrList$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecStrList.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecStrList.c

$(OBJDIR)$(PS)rtXmlpDecUInt$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecUInt.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecUInt.c

$(OBJDIR)$(PS)rtXmlpDecUInt64$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecUInt64.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecUInt64.c

$(OBJDIR)$(PS)rtXmlpDecUnicode$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecUnicode.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h \
 $(XMLSRCDIR)$(PS)rtXmlPull.hh
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecUnicode.c

$(OBJDIR)$(PS)rtXmlpDecXmlStr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecXmlStr.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h \
 $(XMLSRCDIR)$(PS)rtXmlPull.hh
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecXmlStr.c

$(OBJDIR)$(PS)rtXmlpDecXSIAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecXSIAttr.c \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecXSIAttr.c

$(OBJDIR)$(PS)rtXmlpDecXSITypeAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpDecXSITypeAttr.c \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecXSITypeAttr.c

$(OBJDIR)$(PS)rtXmlpEvent$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpEvent.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.hh $(XMLSRCDIR)$(PS)rtXmlPull.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpEvent.c

$(OBJDIR)$(PS)rtXmlpGetAttributeCount$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpGetAttributeCount.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(XMLSRCDIR)$(PS)rtXmlPull.hh $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpGetAttributeCount.c

$(OBJDIR)$(PS)rtXmlpGetAttributeID$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpGetAttributeID.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpGetAttributeID.c

$(OBJDIR)$(PS)rtXmlpGetContent$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpGetContent.c $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpGetContent.c

$(OBJDIR)$(PS)rtXmlpGetNextElem$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpGetNextElem.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(XMLSRCDIR)$(PS)rtXmlPull.hh $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpGetNextElem.c

$(OBJDIR)$(PS)rtXmlpGetNextElemID$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpGetNextElemID.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(XMLSRCDIR)$(PS)rtXmlPull.hh $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h \
 $(XMLSRCDIR)$(PS)rtXmlpGetNextAllElemID.c
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpGetNextElemID.c

$(OBJDIR)$(PS)rtXmlpHasAttributes$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpHasAttributes.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(XMLSRCDIR)$(PS)rtXmlPull.hh $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpHasAttributes.c

$(OBJDIR)$(PS)rtXmlpIsInGroup$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpIsInGroup.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpIsInGroup.c

$(OBJDIR)$(PS)rtXmlpIsUTF8Encoding$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpIsUTF8Encoding.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpIsUTF8Encoding.c

$(OBJDIR)$(PS)rtXmlpMatch$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpMatch.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.hh $(XMLSRCDIR)$(PS)rtXmlPull.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpMatch.c

$(OBJDIR)$(PS)rtXmlpReadBytes$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpReadBytes.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpReadBytes.c

$(OBJDIR)$(PS)rtXmlpSelectAttribute$(OBJ): $(XMLSRCDIR)$(PS)rtXmlpSelectAttribute.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(XMLSRCDIR)$(PS)rtXmlPull.hh $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpSelectAttribute.c

$(OBJDIR)$(PS)rtXmlWriteChars$(OBJ): $(XMLSRCDIR)$(PS)rtXmlWriteChars.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxUTF16.h $(RTXSRCDIR)$(PS)rtxLatin1.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlWriteChars.c

$(LCOBJDIR)$(PS)rtXmlContext$(OBJ): $(XMLSRCDIR)$(PS)rtXmlContext.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(XMLSRCDIR)$(PS)rtXmlCtxtAppInfo.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlContext.c

$(LCOBJDIR)$(PS)rtXmlDecDynUTF8Str$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecDynUTF8Str.c \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxSList.h $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtSaxCParser.h \
 $(RTXSRCDIR)$(PS)rtxUnicode.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecDynUTF8Str.c

$(LCOBJDIR)$(PS)rtXmlDecInt$(OBJ): $(XMLSRCDIR)$(PS)rtXmlDecInt.c $(XMLSRCDIR)$(PS)osrtxml.hh \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlDecInt.c

$(LCOBJDIR)$(PS)rtXmlEncString$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncString.c $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncString.c

$(LCOBJDIR)$(PS)rtXmlPull$(OBJ): $(XMLSRCDIR)$(PS)rtXmlPull.c $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxUnicode.h $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(XMLSRCDIR)$(PS)rtXmlPull.hh $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlPull.c

$(LCOBJDIR)$(PS)rtXmlSetEncBufPtr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlSetEncBufPtr.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxStream.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlSetEncBufPtr.c

$(OBJDIR)$(PS)rtXmlEncAnyAttr$(OBJ): $(XMLSRCDIR)$(PS)rtXmlEncAnyAttr.c $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlEncAnyAttr.c

$(OBJDIR)$(PS)OSXMLMessageBuffer$(OBJ): $(XMLSRCDIR)$(PS)OSXMLMessageBuffer.cpp \
 $(XMLSRCDIR)$(PS)osrtxml.hh $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(XMLSRCDIR)$(PS)OSXMLMessageBuffer.h $(RTXSRCDIR)$(PS)OSRTMsgBuf.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h
	$(CCC) $(XMLCCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/OSXMLMessageBuffer.cpp

$(OBJDIR)$(PS)OSXMLEncodeBuffer$(OBJ): $(XMLSRCDIR)$(PS)OSXMLEncodeBuffer.cpp \
 $(XMLSRCDIR)$(PS)OSXMLEncodeBuffer.h $(XMLSRCDIR)$(PS)OSXMLMessageBuffer.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBuf.h $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(XMLSRCDIR)$(PS)osrtxml.hh
	$(CCC) $(XMLCCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/OSXMLEncodeBuffer.cpp

$(OBJDIR)$(PS)OSXMLEncodeStream$(OBJ): $(XMLSRCDIR)$(PS)OSXMLEncodeStream.cpp \
 $(XMLSRCDIR)$(PS)OSXMLEncodeStream.h $(RTXSRCDIR)$(PS)OSRTOutputStream.h \
 $(RTXSRCDIR)$(PS)OSRTOutputStreamIF.h $(RTXSRCDIR)$(PS)OSRTStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)OSRTStream.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h $(XMLSRCDIR)$(PS)OSXMLMessageBuffer.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBuf.h $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CCC) $(XMLCCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/OSXMLEncodeStream.cpp

$(OBJDIR)$(PS)OSXMLDecodeBuffer$(OBJ): $(XMLSRCDIR)$(PS)OSXMLDecodeBuffer.cpp \
 $(RTXSRCDIR)$(PS)OSRTFileInputStream.h $(RTXSRCDIR)$(PS)OSRTInputStream.h \
 $(RTXSRCDIR)$(PS)OSRTInputStreamIF.h $(RTXSRCDIR)$(PS)OSRTStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)OSRTStream.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h $(RTXSRCDIR)$(PS)OSRTMemoryInputStream.h \
 $(XMLSRCDIR)$(PS)OSXMLDecodeBuffer.h $(XMLSRCDIR)$(PS)OSXMLMessageBuffer.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBuf.h $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtSaxCppParserIF.h
	$(CCC) $(XMLCCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/OSXMLDecodeBuffer.cpp

$(OBJDIR)$(PS)rtLx2DomSerializeToMem$(OBJ): $(XMLSRCDIR)$(PS)rtLx2DomSerializeToMem.c \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(XMLSRCDIR)$(PS)rtLx2Dom.h \
 $(LIBXML2INC)$(PS)libxml$(PS)tree.h \
 $(LIBXML2INC)$(PS)libxml$(PS)xmlversion.h \
 $(LIBXML2INC)$(PS)libxml$(PS)xmlexports.h \
 $(LIBXML2INC)$(PS)libxml$(PS)xmlstring.h \
 $(LIBXML2INC)$(PS)libxml$(PS)xmlregexp.h \
 $(LIBXML2INC)$(PS)libxml$(PS)dict.h \
 $(LIBXML2INC)$(PS)libxml$(PS)xmlmemory.h \
 $(LIBXML2INC)$(PS)libxml$(PS)threads.h \
 $(LIBXML2INC)$(PS)libxml$(PS)globals.h \
 $(LIBXML2INC)$(PS)libxml$(PS)parser.h \
 $(LIBXML2INC)$(PS)libxml$(PS)hash.h $(LIBXML2INC)$(PS)libxml$(PS)valid.h \
 $(LIBXML2INC)$(PS)libxml$(PS)xmlerror.h \
 $(LIBXML2INC)$(PS)libxml$(PS)list.h \
 $(LIBXML2INC)$(PS)libxml$(PS)xmlautomata.h \
 $(LIBXML2INC)$(PS)libxml$(PS)entities.h \
 $(LIBXML2INC)$(PS)libxml$(PS)encoding.h \
 $(LIBXML2INC)$(PS)libxml$(PS)xmlIO.h $(LIBXML2INC)$(PS)libxml$(PS)SAX.h \
 $(LIBXML2INC)$(PS)libxml$(PS)xlink.h $(LIBXML2INC)$(PS)libxml$(PS)SAX2.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtLx2DomSerializeToMem.c

$(OBJDIR)$(PS)rtAsn1XmlAddAnyAttr$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlAddAnyAttr.c \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(XMLSRCDIR)$(PS)asn1xml.h $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlAddAnyAttr.c

$(OBJDIR)$(PS)rtAsn1XmlEncGenTime$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlEncGenTime.c \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(XMLSRCDIR)$(PS)asn1xml.h \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlEncGenTime.c

$(OBJDIR)$(PS)rtAsn1XmlEncUTCTime$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlEncUTCTime.c \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(XMLSRCDIR)$(PS)asn1xml.h \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlEncUTCTime.c

$(OBJDIR)$(PS)rtAsn1XmlEncOpenType$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlEncOpenType.c \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(XMLSRCDIR)$(PS)asn1xml.h $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlEncOpenType.c

$(OBJDIR)$(PS)rtAsn1XmlEncObjId$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlEncObjId.c \
 $(XMLSRCDIR)$(PS)asn1xml.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlEncObjId.c

$(OBJDIR)$(PS)rtAsn1XmlEncOpenTypeExt$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlEncOpenTypeExt.c \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(XMLSRCDIR)$(PS)asn1xml.h $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlEncOpenTypeExt.c

$(OBJDIR)$(PS)rtAsn1XmlEncReal$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlEncReal.c $(XMLSRCDIR)$(PS)asn1xml.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)osrtxml.hh
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlEncReal.c

$(OBJDIR)$(PS)rtAsn1XmlEncRelOID$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlEncRelOID.c \
 $(XMLSRCDIR)$(PS)asn1xml.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlEncRelOID.c

$(OBJDIR)$(PS)rtAsn1XmlEncUnivStr$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlEncUnivStr.c \
 $(RTXSRCDIR)$(PS)rtxCommon.hh $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxContext.hh $(XMLSRCDIR)$(PS)osrtxml.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlEncUnicode.c \
 $(XMLSRCDIR)$(PS)osrtxml.hh
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlEncUnivStr.c

$(OBJDIR)$(PS)rtAsn1XmlFmtAttrStr$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlFmtAttrStr.c \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(XMLSRCDIR)$(PS)asn1xml.h $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlFmtAttrStr.c

$(OBJDIR)$(PS)rtAsn1XmlParseAttrStr$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlParseAttrStr.c \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(XMLSRCDIR)$(PS)asn1xml.h $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlParseAttrStr.c

$(OBJDIR)$(PS)rtAsn1XmlpDecGenTime$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlpDecGenTime.c \
 $(XMLSRCDIR)$(PS)asn1xml.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlpDecGenTime.c

$(OBJDIR)$(PS)rtAsn1XmlpDecObjId$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlpDecObjId.c \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxCtype.h $(XMLSRCDIR)$(PS)asn1xml.h \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlpDecObjId.c

$(OBJDIR)$(PS)rtAsn1XmlpDecOpenType$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlpDecOpenType.c \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(XMLSRCDIR)$(PS)asn1xml.h $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlpDecOpenType.c

$(OBJDIR)$(PS)rtAsn1XmlpDecReal$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlpDecReal.c $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(XMLSRCDIR)$(PS)asn1xml.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlpDecReal.c

$(OBJDIR)$(PS)rtAsn1XmlpDecUnivStr$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlpDecUnivStr.c \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h \
 $(XMLSRCDIR)$(PS)rtXmlPull.hh $(XMLSRCDIR)$(PS)rtXmlpDecUnicode.c
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlpDecUnivStr.c

$(OBJDIR)$(PS)rtAsn1XmlpDecUTCTime$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlpDecUTCTime.c \
 $(XMLSRCDIR)$(PS)asn1xml.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlpDecUTCTime.c

$(OBJDIR)$(PS)rtAsn1XmlpDecDynBitStr$(OBJ): $(XMLSRCDIR)$(PS)rtAsn1XmlpDecDynBitStr.c \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(XMLSRCDIR)$(PS)asn1xml.h $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxStreamBuffered.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtAsn1XmlpDecDynBitStr.c

$(OBJDIR)$(PS)rtXmlpDecListOfASN1DynBitStr$(OBJ): \
 $(XMLSRCDIR)$(PS)rtXmlpDecListOfASN1DynBitStr.c $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h \
 $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlPull.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxStreamBuffered.h \
 $(XMLSRCDIR)$(PS)asn1xml.h
	$(CC) $(XMLCFLAGS) -c $(IPATHS) $(OBJOUT) $(XMLSRCDIR)/rtXmlpDecListOfASN1DynBitStr.c
