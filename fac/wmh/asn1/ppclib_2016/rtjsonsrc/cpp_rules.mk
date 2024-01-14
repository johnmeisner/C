# C to object file compilation rules

$(OBJDIR)$(PS)OSJSONDecodeBuffer$(OBJ): $(JSONSRCDIR)/OSJSONDecodeBuffer.cpp \
 $(RTXSRCDIR)/OSRTFileInputStream.h $(RTXSRCDIR)/OSRTInputStream.h \
 $(RTXSRCDIR)/OSRTInputStreamIF.h $(RTXSRCDIR)/OSRTStreamIF.h \
 $(RTXSRCDIR)/OSRTCtxtHolderIF.h $(RTXSRCDIR)/OSRTContext.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/OSRTStream.h \
 $(RTXSRCDIR)/OSRTCtxtHolder.h $(RTXSRCDIR)/OSRTMemoryInputStream.h \
 $(JSONSRCDIR)/OSJSONDecodeBuffer.h $(JSONSRCDIR)/OSJSONMessageBuffer.h \
 $(RTXSRCDIR)/OSRTMsgBuf.h $(RTXSRCDIR)/OSRTMsgBufIF.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxFile.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CCC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/OSJSONDecodeBuffer.cpp

$(OBJDIR)$(PS)OSJSONEncodeBuffer$(OBJ): $(JSONSRCDIR)/OSJSONEncodeBuffer.cpp \
 $(JSONSRCDIR)/OSJSONEncodeBuffer.h $(JSONSRCDIR)/OSJSONMessageBuffer.h \
 $(RTXSRCDIR)/OSRTMsgBuf.h $(RTXSRCDIR)/OSRTCtxtHolder.h \
 $(RTXSRCDIR)/OSRTCtxtHolderIF.h $(RTXSRCDIR)/OSRTContext.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/OSRTMsgBufIF.h \
 $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h \
 $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxEnum.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h \
 $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CCC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/OSJSONEncodeBuffer.cpp

$(OBJDIR)$(PS)OSJSONEncodeStream$(OBJ): $(JSONSRCDIR)/OSJSONEncodeStream.cpp \
 $(JSONSRCDIR)/OSJSONEncodeStream.h $(RTXSRCDIR)/OSRTOutputStream.h \
 $(RTXSRCDIR)/OSRTOutputStreamIF.h $(RTXSRCDIR)/OSRTStreamIF.h \
 $(RTXSRCDIR)/OSRTCtxtHolderIF.h $(RTXSRCDIR)/OSRTContext.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/OSRTStream.h \
 $(RTXSRCDIR)/OSRTCtxtHolder.h $(JSONSRCDIR)/OSJSONMessageBuffer.h \
 $(RTXSRCDIR)/OSRTMsgBuf.h $(RTXSRCDIR)/OSRTMsgBufIF.h $(JSONSRCDIR)/osrtjson.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxBitString.h \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h \
 $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxFile.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h
	$(CCC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/OSJSONEncodeStream.cpp

$(OBJDIR)$(PS)OSJSONMessageBuffer$(OBJ): $(JSONSRCDIR)/OSJSONMessageBuffer.cpp \
 $(JSONSRCDIR)/osrtjson.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h \
 $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/rtxPattern.h $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h \
 $(RTXSRCDIR)/rtxUtil.h $(JSONSRCDIR)/rtJsonExternDefs.h \
 $(JSONSRCDIR)/OSJSONMessageBuffer.h $(RTXSRCDIR)/OSRTMsgBuf.h \
 $(RTXSRCDIR)/OSRTCtxtHolder.h $(RTXSRCDIR)/OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)/OSRTContext.h $(RTXSRCDIR)/OSRTMsgBufIF.h
	$(CCC) $(JSONCFLAGS) -c $(IPATHS) $(OBJOUT) $(JSONSRCDIR)/OSJSONMessageBuffer.cpp
