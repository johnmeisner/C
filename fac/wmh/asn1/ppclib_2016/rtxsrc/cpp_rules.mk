# C++ to object file compilation rules

OSRTBase64TextInputStream$(OBJ): $(RTXSRCDIR)/OSRTBase64TextInputStream.cpp \
 $(RTXSRCDIR)/OSRTBase64TextInputStream.h $(RTXSRCDIR)/OSRTInputStream.h \
 $(RTXSRCDIR)/OSRTInputStreamIF.h $(RTXSRCDIR)/OSRTStreamIF.h \
 $(RTXSRCDIR)/OSRTCtxtHolderIF.h $(RTXSRCDIR)/OSRTContext.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/OSRTStream.h \
 $(RTXSRCDIR)/OSRTCtxtHolder.h $(RTXSRCDIR)/rtxStreamBase64Text.h \
 $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxMemBuf.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTBase64TextInputStream.cpp

OSRTContext$(OBJ): $(RTXSRCDIR)/OSRTContext.cpp $(RTXSRCDIR)/OSRTContext.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxContext.hh \
 $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h $(RTXSRCDIR)/rtxCharStr.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTContext.cpp

OSRTCtxtHolder$(OBJ): $(RTXSRCDIR)/OSRTCtxtHolder.cpp $(RTXSRCDIR)/OSRTCtxtHolder.h \
 $(RTXSRCDIR)/OSRTCtxtHolderIF.h $(RTXSRCDIR)/OSRTContext.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTCtxtHolder.cpp

OSRTFileInputStream$(OBJ): $(RTXSRCDIR)/OSRTFileInputStream.cpp \
 $(RTXSRCDIR)/rtxStreamFile.h $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/OSRTFileInputStream.h \
 $(RTXSRCDIR)/OSRTInputStream.h $(RTXSRCDIR)/OSRTInputStreamIF.h \
 $(RTXSRCDIR)/OSRTStreamIF.h $(RTXSRCDIR)/OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)/OSRTContext.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/OSRTStream.h \
 $(RTXSRCDIR)/OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTFileInputStream.cpp

OSRTFileOutputStream$(OBJ): $(RTXSRCDIR)/OSRTFileOutputStream.cpp \
 $(RTXSRCDIR)/rtxStreamFile.h $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/OSRTFileOutputStream.h \
 $(RTXSRCDIR)/OSRTOutputStream.h $(RTXSRCDIR)/OSRTOutputStreamIF.h \
 $(RTXSRCDIR)/OSRTStreamIF.h $(RTXSRCDIR)/OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)/OSRTContext.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/OSRTStream.h \
 $(RTXSRCDIR)/OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTFileOutputStream.cpp

OSRTHexTextInputStream$(OBJ): $(RTXSRCDIR)/OSRTHexTextInputStream.cpp \
 $(RTXSRCDIR)/rtxStreamHexText.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxMemBuf.h \
 $(RTXSRCDIR)/OSRTHexTextInputStream.h $(RTXSRCDIR)/OSRTInputStream.h \
 $(RTXSRCDIR)/OSRTInputStreamIF.h $(RTXSRCDIR)/OSRTStreamIF.h \
 $(RTXSRCDIR)/OSRTCtxtHolderIF.h $(RTXSRCDIR)/OSRTContext.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/OSRTStream.h $(RTXSRCDIR)/OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTHexTextInputStream.cpp

OSRTInputStream$(OBJ): $(RTXSRCDIR)/OSRTInputStream.cpp $(RTXSRCDIR)/rtxStream.h \
 $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/OSRTInputStream.h \
 $(RTXSRCDIR)/OSRTInputStreamIF.h $(RTXSRCDIR)/OSRTStreamIF.h \
 $(RTXSRCDIR)/OSRTCtxtHolderIF.h $(RTXSRCDIR)/OSRTContext.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h \
 $(RTXSRCDIR)/OSRTStream.h $(RTXSRCDIR)/OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTInputStream.cpp

OSRTMemBuf$(OBJ): $(RTXSRCDIR)/OSRTMemBuf.cpp $(RTXSRCDIR)/OSRTMemBuf.h \
 $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTMemBuf.cpp

OSRTMemoryInputStream$(OBJ): $(RTXSRCDIR)/OSRTMemoryInputStream.cpp \
 $(RTXSRCDIR)/rtxStreamMemory.h $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/OSRTMemoryInputStream.h \
 $(RTXSRCDIR)/OSRTInputStream.h $(RTXSRCDIR)/OSRTInputStreamIF.h \
 $(RTXSRCDIR)/OSRTStreamIF.h $(RTXSRCDIR)/OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)/OSRTContext.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/OSRTStream.h \
 $(RTXSRCDIR)/OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTMemoryInputStream.cpp

OSRTMemoryOutputStream$(OBJ): $(RTXSRCDIR)/OSRTMemoryOutputStream.cpp \
 $(RTXSRCDIR)/rtxStreamMemory.h $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/OSRTMemoryOutputStream.h \
 $(RTXSRCDIR)/OSRTOutputStream.h $(RTXSRCDIR)/OSRTOutputStreamIF.h \
 $(RTXSRCDIR)/OSRTStreamIF.h $(RTXSRCDIR)/OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)/OSRTContext.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/OSRTStream.h \
 $(RTXSRCDIR)/OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTMemoryOutputStream.cpp

OSRTMsgBuf$(OBJ): $(RTXSRCDIR)/OSRTMsgBuf.cpp $(RTXSRCDIR)/OSRTMsgBuf.h \
 $(RTXSRCDIR)/OSRTCtxtHolder.h $(RTXSRCDIR)/OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)/OSRTContext.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/OSRTMsgBufIF.h $(RTXSRCDIR)/OSRTMemBuf.h \
 $(RTXSRCDIR)/rtxMemBuf.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTMsgBuf.cpp

OSRTOutputStream$(OBJ): $(RTXSRCDIR)/OSRTOutputStream.cpp $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h $(RTXSRCDIR)/rtxMemBuf.h \
 $(RTXSRCDIR)/OSRTOutputStream.h $(RTXSRCDIR)/OSRTOutputStreamIF.h \
 $(RTXSRCDIR)/OSRTStreamIF.h $(RTXSRCDIR)/OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)/OSRTContext.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/OSRTStream.h \
 $(RTXSRCDIR)/OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTOutputStream.cpp

OSRTSocket$(OBJ): $(RTXSRCDIR)/OSRTSocket.cpp $(RTXSRCDIR)/OSRTSocket.h \
 $(RTXSRCDIR)/rtxSocket.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTSocket.cpp

OSRTSocketInputStream$(OBJ): $(RTXSRCDIR)/OSRTSocketInputStream.cpp \
 $(RTXSRCDIR)/rtxStreamSocket.h $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/rtxSocket.h \
 $(RTXSRCDIR)/OSRTSocketInputStream.h $(RTXSRCDIR)/OSRTSocket.h \
 $(RTXSRCDIR)/OSRTInputStream.h $(RTXSRCDIR)/OSRTInputStreamIF.h \
 $(RTXSRCDIR)/OSRTStreamIF.h $(RTXSRCDIR)/OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)/OSRTContext.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/OSRTStream.h \
 $(RTXSRCDIR)/OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTSocketInputStream.cpp

OSRTSocketOutputStream$(OBJ): $(RTXSRCDIR)/OSRTSocketOutputStream.cpp \
 $(RTXSRCDIR)/rtxStreamSocket.h $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/rtxSocket.h \
 $(RTXSRCDIR)/OSRTSocketOutputStream.h $(RTXSRCDIR)/OSRTSocket.h \
 $(RTXSRCDIR)/OSRTOutputStream.h $(RTXSRCDIR)/OSRTOutputStreamIF.h \
 $(RTXSRCDIR)/OSRTStreamIF.h $(RTXSRCDIR)/OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)/OSRTContext.h $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h \
 $(RTXSRCDIR)/rtxErrCodes.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/OSRTStream.h \
 $(RTXSRCDIR)/OSRTCtxtHolder.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTSocketOutputStream.cpp

OSRTStream$(OBJ): $(RTXSRCDIR)/OSRTStream.cpp $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxStream.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxMemBuf.h $(RTXSRCDIR)/OSRTStream.h $(RTXSRCDIR)/OSRTCtxtHolder.h \
 $(RTXSRCDIR)/OSRTCtxtHolderIF.h $(RTXSRCDIR)/OSRTContext.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/OSRTStreamIF.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTStream.cpp

OSRTString$(OBJ): $(RTXSRCDIR)/OSRTString.cpp $(RTXSRCDIR)/OSRTString.h \
 $(RTXSRCDIR)/rtxCommon.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/osMacros.h \
 $(RTXSRCDIR)/rtxExternDefs.h $(RTXSRCDIR)/rtxBigInt.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxBitString.h $(RTXSRCDIR)/rtxBuffer.h $(RTXSRCDIR)/rtxSList.h \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxDateTime.h $(RTXSRCDIR)/rtxDiag.h \
 $(RTXSRCDIR)/rtxEnum.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxFile.h $(RTXSRCDIR)/rtxMemory.h $(RTXSRCDIR)/rtxPattern.h \
 $(RTXSRCDIR)/rtxReal.h $(RTXSRCDIR)/rtxUTF8.h $(RTXSRCDIR)/rtxUtil.h \
 $(RTXSRCDIR)/rtxPrint.h $(RTXSRCDIR)/rtxHexDump.h $(RTXSRCDIR)/OSRTStringIF.h \
 $(RTXSRCDIR)/rtxCtype.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTString.cpp

OSRTStringTokenizer$(OBJ): $(RTXSRCDIR)/OSRTStringTokenizer.cpp \
 $(RTXSRCDIR)/rtxCharStr.h $(RTXSRCDIR)/rtxContext.h $(RTXSRCDIR)/rtxDList.h \
 $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/OSRTStringTokenizer.h $(RTXSRCDIR)/OSRTVoidPtrList.h \
 $(RTXSRCDIR)/osSysTypes.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTStringTokenizer.cpp

OSRTVoidPtrList$(OBJ): $(RTXSRCDIR)/OSRTVoidPtrList.cpp \
 $(RTXSRCDIR)/OSRTVoidPtrList.h $(RTXSRCDIR)/osSysTypes.h
	$(CCC) $(RTXCCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/OSRTVoidPtrList.cpp

rtxCppDList$(OBJ): $(RTXSRCDIR)/rtxCppDList.cpp $(RTXSRCDIR)/rtxCppDList.h \
 $(RTXSRCDIR)/OSRTBaseType.h $(RTXSRCDIR)/OSRTContext.h $(RTXSRCDIR)/rtxContext.h \
 $(RTXSRCDIR)/rtxDList.h $(RTXSRCDIR)/osSysTypes.h $(RTXSRCDIR)/rtxExternDefs.h \
 $(RTXSRCDIR)/rtxCommonDefs.h $(RTXSRCDIR)/osMacros.h $(RTXSRCDIR)/rtxStack.h \
 $(RTXSRCDIR)/rtxDiag.h $(RTXSRCDIR)/rtxError.h $(RTXSRCDIR)/rtxErrCodes.h \
 $(RTXSRCDIR)/rtxMemory.h
	$(CCC) $(CCFLAGS) -c $(IPATHS) $(RTXSRCDIR)/rtxCppDList.cpp
