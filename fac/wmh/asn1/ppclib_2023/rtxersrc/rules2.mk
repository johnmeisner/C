# C/C++ to object file compilation rules

$(OBJDIR)$(PS)xerCmpText$(OBJ): $(XERSRCDIR)$(PS)xerCmpText.c $(XERSRCDIR)$(PS)xed_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerCmpText.c


$(OBJDIR)$(PS)xerContext$(OBJ): $(XERSRCDIR)$(PS)xerContext.c $(XERSRCDIR)$(PS)xed_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XMLSRCDIR)$(PS)rtXmlCtxtAppInfo.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerContext.c


$(OBJDIR)$(PS)xerCopyText$(OBJ): $(XERSRCDIR)$(PS)xerCopyText.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerCopyText.c


$(OBJDIR)$(PS)xerDecBMPStr$(OBJ): $(XERSRCDIR)$(PS)xerDecBMPStr.c $(XERSRCDIR)$(PS)asn1xer.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecBMPStr.c


$(OBJDIR)$(PS)xerDecBase64StrValue$(OBJ): $(XERSRCDIR)$(PS)xerDecBase64StrValue.c \
 $(XERSRCDIR)$(PS)xed_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecBase64StrValue.c


$(OBJDIR)$(PS)xerDecBigInt$(OBJ): $(XERSRCDIR)$(PS)xerDecBigInt.c $(XERSRCDIR)$(PS)xed_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecBigInt.c


$(OBJDIR)$(PS)xerDecBinStrValue$(OBJ): $(XERSRCDIR)$(PS)xerDecBinStrValue.c \
 $(XERSRCDIR)$(PS)xed_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecBinStrValue.c


$(OBJDIR)$(PS)xerDecBitStr$(OBJ): $(XERSRCDIR)$(PS)xerDecBitStr.c $(XERSRCDIR)$(PS)xed_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecBitStr.c


$(OBJDIR)$(PS)xerDecBitStrMemBuf$(OBJ): $(XERSRCDIR)$(PS)xerDecBitStrMemBuf.c \
 $(XERSRCDIR)$(PS)xed_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecBitStrMemBuf.c


$(OBJDIR)$(PS)xerDecBool$(OBJ): $(XERSRCDIR)$(PS)xerDecBool.c $(XERSRCDIR)$(PS)xed_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecBool.c


$(OBJDIR)$(PS)xerDecCopyBitStr$(OBJ): $(XERSRCDIR)$(PS)xerDecCopyBitStr.c \
 $(XERSRCDIR)$(PS)xed_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecCopyBitStr.c


$(OBJDIR)$(PS)xerDecCopyDynBitStr$(OBJ): $(XERSRCDIR)$(PS)xerDecCopyDynBitStr.c \
 $(XERSRCDIR)$(PS)xed_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecCopyDynBitStr.c


$(OBJDIR)$(PS)xerDecCopyDynOctStr$(OBJ): $(XERSRCDIR)$(PS)xerDecCopyDynOctStr.c \
 $(XERSRCDIR)$(PS)xed_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecCopyDynOctStr.c


$(OBJDIR)$(PS)xerDecCopyOctStr$(OBJ): $(XERSRCDIR)$(PS)xerDecCopyOctStr.c \
 $(XERSRCDIR)$(PS)xed_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecCopyOctStr.c


$(OBJDIR)$(PS)xerDecDynBase64Str$(OBJ): $(XERSRCDIR)$(PS)xerDecDynBase64Str.c \
 $(XERSRCDIR)$(PS)xed_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecDynBase64Str.c


$(OBJDIR)$(PS)xerDecDynBitStr$(OBJ): $(XERSRCDIR)$(PS)xerDecDynBitStr.c \
 $(XERSRCDIR)$(PS)xed_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecDynBitStr.c


$(OBJDIR)$(PS)xerDecDynUTF8Str$(OBJ): $(XERSRCDIR)$(PS)xerDecDynUTF8Str.c $(XERSRCDIR)$(PS)asn1xer.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h \
 $(RTSRCDIR)$(PS)asn1intl.h $(XERSRCDIR)$(PS)xed_common.hh $(XERSRCDIR)$(PS)asn1xer.h \
 $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(XERSRCDIR)$(PS)xee_common.hh
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecDynUTF8Str.c


$(OBJDIR)$(PS)xerDecHexStrValue$(OBJ): $(XERSRCDIR)$(PS)xerDecHexStrValue.c \
 $(XERSRCDIR)$(PS)xed_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecHexStrValue.c


$(OBJDIR)$(PS)xerDecInt64$(OBJ): $(XERSRCDIR)$(PS)xerDecInt64.c $(XERSRCDIR)$(PS)xed_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecInt64.c


$(OBJDIR)$(PS)xerDecObjId64$(OBJ): $(XERSRCDIR)$(PS)xerDecObjId64.c $(XERSRCDIR)$(PS)xed_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecObjId64.c


$(OBJDIR)$(PS)xerDecOctStrMemBuf$(OBJ): $(XERSRCDIR)$(PS)xerDecOctStrMemBuf.c \
 $(XERSRCDIR)$(PS)xed_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecOctStrMemBuf.c


$(OBJDIR)$(PS)xerDecOpenType$(OBJ): $(XERSRCDIR)$(PS)xerDecOpenType.c $(XERSRCDIR)$(PS)xed_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecOpenType.c


$(OBJDIR)$(PS)xerDecReal$(OBJ): $(XERSRCDIR)$(PS)xerDecReal.c $(XERSRCDIR)$(PS)xed_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxReal.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecReal.c


$(OBJDIR)$(PS)xerDecReal10$(OBJ): $(XERSRCDIR)$(PS)xerDecReal10.c $(XERSRCDIR)$(PS)xed_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecReal10.c


$(OBJDIR)$(PS)xerDecRelativeOID$(OBJ): $(XERSRCDIR)$(PS)xerDecRelativeOID.c \
 $(XERSRCDIR)$(PS)xed_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecRelativeOID.c


$(OBJDIR)$(PS)xerDecUInt$(OBJ): $(XERSRCDIR)$(PS)xerDecUInt.c $(XERSRCDIR)$(PS)xed_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecUInt.c


$(OBJDIR)$(PS)xerDecUInt64$(OBJ): $(XERSRCDIR)$(PS)xerDecUInt64.c $(XERSRCDIR)$(PS)xed_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecUInt64.c


$(OBJDIR)$(PS)xerDecUnivStr$(OBJ): $(XERSRCDIR)$(PS)xerDecUnivStr.c $(XERSRCDIR)$(PS)asn1xer.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecUnivStr.c


$(OBJDIR)$(PS)xerEncAscCharStr$(OBJ): $(XERSRCDIR)$(PS)xerEncAscCharStr.c \
 $(XERSRCDIR)$(PS)xee_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncAscCharStr.c


$(OBJDIR)$(PS)xerEncBMPStr$(OBJ): $(XERSRCDIR)$(PS)xerEncBMPStr.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncBMPStr.c


$(OBJDIR)$(PS)xerEncBigInt$(OBJ): $(XERSRCDIR)$(PS)xerEncBigInt.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxBigInt.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncBigInt.c


$(OBJDIR)$(PS)xerEncBinStrValue$(OBJ): $(XERSRCDIR)$(PS)xerEncBinStrValue.c \
 $(XERSRCDIR)$(PS)xee_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncBinStrValue.c


$(OBJDIR)$(PS)xerEncBitStr$(OBJ): $(XERSRCDIR)$(PS)xerEncBitStr.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncBitStr.c


$(OBJDIR)$(PS)xerEncBool$(OBJ): $(XERSRCDIR)$(PS)xerEncBool.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncBool.c


$(OBJDIR)$(PS)xerEncChar$(OBJ): $(XERSRCDIR)$(PS)xerEncChar.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncChar.c


$(OBJDIR)$(PS)xerEncEndDocument$(OBJ): $(XERSRCDIR)$(PS)xerEncEndDocument.c \
 $(XERSRCDIR)$(PS)xee_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h \
 $(RTXSRCDIR)$(PS)rtxStream.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncEndDocument.c


$(OBJDIR)$(PS)xerEncEndElement$(OBJ): $(XERSRCDIR)$(PS)xerEncEndElement.c \
 $(XERSRCDIR)$(PS)xee_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncEndElement.c


$(OBJDIR)$(PS)xerEncHexStrValue$(OBJ): $(XERSRCDIR)$(PS)xerEncHexStrValue.c \
 $(XERSRCDIR)$(PS)xee_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncHexStrValue.c


$(OBJDIR)$(PS)xerEncIndent$(OBJ): $(XERSRCDIR)$(PS)xerEncIndent.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncIndent.c


$(OBJDIR)$(PS)xerEncInt64$(OBJ): $(XERSRCDIR)$(PS)xerEncInt64.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncInt64.c


$(OBJDIR)$(PS)xerEncNamedValue$(OBJ): $(XERSRCDIR)$(PS)xerEncNamedValue.c \
 $(XERSRCDIR)$(PS)xee_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncNamedValue.c


$(OBJDIR)$(PS)xerEncNewLine$(OBJ): $(XERSRCDIR)$(PS)xerEncNewLine.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncNewLine.c


$(OBJDIR)$(PS)xerEncNull$(OBJ): $(XERSRCDIR)$(PS)xerEncNull.c $(XERSRCDIR)$(PS)asn1xer.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h \
 $(RTSRCDIR)$(PS)asn1intl.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncNull.c


$(OBJDIR)$(PS)xerEncObjId64$(OBJ): $(XERSRCDIR)$(PS)xerEncObjId64.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncObjId64.c


$(OBJDIR)$(PS)xerEncOpenType$(OBJ): $(XERSRCDIR)$(PS)xerEncOpenType.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncOpenType.c


$(OBJDIR)$(PS)xerEncOpenTypeExt$(OBJ): $(XERSRCDIR)$(PS)xerEncOpenTypeExt.c \
 $(XERSRCDIR)$(PS)xee_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncOpenTypeExt.c


$(OBJDIR)$(PS)xerEncReal$(OBJ): $(XERSRCDIR)$(PS)xerEncReal.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxReal.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncReal.c


$(OBJDIR)$(PS)xerEncReal10$(OBJ): $(XERSRCDIR)$(PS)xerEncReal10.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncReal10.c


$(OBJDIR)$(PS)xerEncRelativeOID$(OBJ): $(XERSRCDIR)$(PS)xerEncRelativeOID.c \
 $(XERSRCDIR)$(PS)xee_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncRelativeOID.c


$(OBJDIR)$(PS)xerEncStartDocument$(OBJ): $(XERSRCDIR)$(PS)xerEncStartDocument.c \
 $(XERSRCDIR)$(PS)xee_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncStartDocument.c


$(OBJDIR)$(PS)xerEncStartElement$(OBJ): $(XERSRCDIR)$(PS)xerEncStartElement.c \
 $(XERSRCDIR)$(PS)xee_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncStartElement.c


$(OBJDIR)$(PS)xerEncUInt$(OBJ): $(XERSRCDIR)$(PS)xerEncUInt.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncUInt.c


$(OBJDIR)$(PS)xerEncUInt64$(OBJ): $(XERSRCDIR)$(PS)xerEncUInt64.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncUInt64.c


$(OBJDIR)$(PS)xerEncUniCharData$(OBJ): $(XERSRCDIR)$(PS)xerEncUniCharData.c \
 $(XERSRCDIR)$(PS)xee_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncUniCharData.c


$(OBJDIR)$(PS)xerEncUniCharStr$(OBJ): $(XERSRCDIR)$(PS)xerEncUniCharStr.c \
 $(XERSRCDIR)$(PS)xee_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncUniCharStr.c


$(OBJDIR)$(PS)xerEncUnivStr$(OBJ): $(XERSRCDIR)$(PS)xerEncUnivStr.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncUnivStr.c


$(OBJDIR)$(PS)xerFinalizeMemBuf$(OBJ): $(XERSRCDIR)$(PS)xerFinalizeMemBuf.c \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerFinalizeMemBuf.c


$(OBJDIR)$(PS)xerGetElemIdx$(OBJ): $(XERSRCDIR)$(PS)xerGetElemIdx.c $(XERSRCDIR)$(PS)asn1xer.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerGetElemIdx.c


$(OBJDIR)$(PS)xerGetLibInfo$(OBJ): $(XERSRCDIR)$(PS)xerGetLibInfo.c $(RTSRCDIR)$(PS)asn1version.h \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerGetLibInfo.c


$(OBJDIR)$(PS)xerGetMsgPtrLen$(OBJ): $(XERSRCDIR)$(PS)xerGetMsgPtrLen.c $(XERSRCDIR)$(PS)asn1xer.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h \
 $(RTSRCDIR)$(PS)asn1intl.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerGetMsgPtrLen.c


$(OBJDIR)$(PS)xerGetSeqElemIdx$(OBJ): $(XERSRCDIR)$(PS)xerGetSeqElemIdx.c $(XERSRCDIR)$(PS)asn1xer.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerGetSeqElemIdx.c


$(OBJDIR)$(PS)xerPutCharStr$(OBJ): $(XERSRCDIR)$(PS)xerPutCharStr.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerPutCharStr.c


$(OBJDIR)$(PS)xerSetDecBufPtr$(OBJ): $(XERSRCDIR)$(PS)xerSetDecBufPtr.c $(XERSRCDIR)$(PS)asn1xer.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h \
 $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerSetDecBufPtr.c


$(OBJDIR)$(PS)xerSetEncBufPtr$(OBJ): $(XERSRCDIR)$(PS)xerSetEncBufPtr.c $(XERSRCDIR)$(PS)asn1xer.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h \
 $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerSetEncBufPtr.c


$(OBJDIR)$(PS)xerTextLength$(OBJ): $(XERSRCDIR)$(PS)xerTextLength.c $(XERSRCDIR)$(PS)xed_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerTextLength.c


$(OBJDIR)$(PS)xerTextToCStr$(OBJ): $(XERSRCDIR)$(PS)xerTextToCStr.c $(XERSRCDIR)$(PS)xed_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerTextToCStr.c


$(LCOBJDIR)$(PS)xerDecBase64Str$(OBJ): $(XERSRCDIR)$(PS)xerDecBase64Str.c \
 $(XERSRCDIR)$(PS)xed_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecBase64Str.c


$(LCOBJDIR)$(PS)xerDecDynAscCharStr$(OBJ): $(XERSRCDIR)$(PS)xerDecDynAscCharStr.c \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecDynAscCharStr.c


$(LCOBJDIR)$(PS)xerDecDynOctStr$(OBJ): $(XERSRCDIR)$(PS)xerDecDynOctStr.c \
 $(XERSRCDIR)$(PS)xed_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTSRCDIR)$(PS)rt_common.hh $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecDynOctStr.c


$(LCOBJDIR)$(PS)xerDecInt$(OBJ): $(XERSRCDIR)$(PS)xerDecInt.c $(XERSRCDIR)$(PS)xed_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecInt.c


$(LCOBJDIR)$(PS)xerDecObjId$(OBJ): $(XERSRCDIR)$(PS)xerDecObjId.c $(XERSRCDIR)$(PS)xed_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecObjId.c


$(LCOBJDIR)$(PS)xerDecOctStr$(OBJ): $(XERSRCDIR)$(PS)xerDecOctStr.c $(XERSRCDIR)$(PS)xed_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerDecOctStr.c


$(LCOBJDIR)$(PS)xerEncBase64Str$(OBJ): $(XERSRCDIR)$(PS)xerEncBase64Str.c \
 $(XERSRCDIR)$(PS)xee_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncBase64Str.c


$(LCOBJDIR)$(PS)xerEncEmptyElement$(OBJ): $(XERSRCDIR)$(PS)xerEncEmptyElement.c \
 $(XERSRCDIR)$(PS)xee_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncEmptyElement.c


$(LCOBJDIR)$(PS)xerEncInt$(OBJ): $(XERSRCDIR)$(PS)xerEncInt.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncInt.c


$(LCOBJDIR)$(PS)xerEncObjId$(OBJ): $(XERSRCDIR)$(PS)xerEncObjId.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncObjId.c


$(LCOBJDIR)$(PS)xerEncOctStr$(OBJ): $(XERSRCDIR)$(PS)xerEncOctStr.c $(XERSRCDIR)$(PS)xee_common.hh \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncOctStr.c


$(LCOBJDIR)$(PS)xerEncXmlCharStr$(OBJ): $(XERSRCDIR)$(PS)xerEncXmlCharStr.c \
 $(XERSRCDIR)$(PS)xee_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)xerEncXmlCharStr.c


$(OBJDIR)$(PS)ASN1SAX_XEROpenType$(OBJ): $(XERSRCDIR)$(PS)ASN1SAX_XEROpenType.c \
 $(XERSRCDIR)$(PS)ASN1SAX_XEROpenType.h $(XERSRCDIR)$(PS)asn1XerCTypes.h \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XERSRCDIR)$(PS)rtSAXDefs.h $(XMLSRCDIR)$(PS)rtSaxCParser.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCharStr.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(RTXSRCDIR)$(PS)rtxUnicode.h $(XMLSRCDIR)$(PS)osrtxml.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(XERSRCDIR)$(PS)xee_common.hh $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxCtype.h $(RTSRCDIR)$(PS)rtBCD.h \
 $(RTXSRCDIR)$(PS)rtxTBCD.h
	$(CC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)ASN1SAX_XEROpenType.c


$(OBJDIR)$(PS)ASN1CXerOpenType$(OBJ): $(XERSRCDIR)$(PS)ASN1CXerOpenType.cpp \
 $(XERSRCDIR)$(PS)ASN1CXerOpenType.h $(XERSRCDIR)$(PS)asn1XerCppTypes.h \
 $(XMLSRCDIR)$(PS)rtSaxCppParser.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(XMLSRCDIR)$(PS)rtSaxCppParserIF.h $(RTXSRCDIR)$(PS)OSRTInputStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTStreamIF.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(RTSRCDIR)$(PS)asn1CppTypes.h $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.h \
 $(RTSRCDIR)$(PS)asn1CppEvtHndlr.h $(RTSRCDIR)$(PS)asn1type.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTSRCDIR)$(PS)ASN1Context.h $(RTXSRCDIR)$(PS)OSRTMsgBuf.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h \
 $(RTSRCDIR)$(PS)ASN1TOctStr.h $(RTSRCDIR)$(PS)ASN1TOctStr64.h $(RTSRCDIR)$(PS)ASN1TObjId.h \
 $(XERSRCDIR)$(PS)asn1xer.h $(XERSRCDIR)$(PS)rtSAXDefs.h
	$(CCC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)ASN1CXerOpenType.cpp


$(OBJDIR)$(PS)ASN1XERDecodeStream$(OBJ): $(XERSRCDIR)$(PS)ASN1XERDecodeStream.cpp \
 $(XERSRCDIR)$(PS)ASN1XERDecodeStream.h $(RTSRCDIR)$(PS)asn1CppTypes.h \
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
 $(RTXSRCDIR)$(PS)OSRTInputStreamIF.h $(RTXSRCDIR)$(PS)OSRTStreamIF.h \
 $(XERSRCDIR)$(PS)asn1XerCppTypes.h $(XMLSRCDIR)$(PS)rtSaxCppParser.h \
 $(XMLSRCDIR)$(PS)rtSaxCppParserIF.h $(XMLSRCDIR)$(PS)osrtxml.h \
 $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XERSRCDIR)$(PS)asn1xer.h $(XERSRCDIR)$(PS)rtSAXDefs.h
	$(CCC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)ASN1XERDecodeStream.cpp


$(OBJDIR)$(PS)ASN1XEREncodeStream$(OBJ): $(XERSRCDIR)$(PS)ASN1XEREncodeStream.cpp \
 $(XERSRCDIR)$(PS)ASN1XEREncodeStream.h $(RTSRCDIR)$(PS)asn1CppTypes.h \
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
 $(RTXSRCDIR)$(PS)OSRTOutputStreamIF.h $(RTXSRCDIR)$(PS)OSRTStreamIF.h \
 $(XERSRCDIR)$(PS)asn1XerCppTypes.h $(XMLSRCDIR)$(PS)rtSaxCppParser.h \
 $(XMLSRCDIR)$(PS)rtSaxCppParserIF.h $(RTXSRCDIR)$(PS)OSRTInputStreamIF.h \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(XERSRCDIR)$(PS)asn1xer.h $(XERSRCDIR)$(PS)rtSAXDefs.h
	$(CCC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)ASN1XEREncodeStream.cpp


$(OBJDIR)$(PS)ASN1XERString$(OBJ): $(XERSRCDIR)$(PS)ASN1XERString.cpp \
 $(XERSRCDIR)$(PS)ASN1XERString.h $(XERSRCDIR)$(PS)rtSAXDefs.h \
 $(XERSRCDIR)$(PS)asn1xer.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CCC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)ASN1XERString.cpp


$(OBJDIR)$(PS)asn1XerCppTypes$(OBJ): $(XERSRCDIR)$(PS)asn1XerCppTypes.cpp \
 $(XERSRCDIR)$(PS)asn1XerCppTypes.h $(XMLSRCDIR)$(PS)rtSaxCppParser.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(XMLSRCDIR)$(PS)rtSaxCppParserIF.h $(RTXSRCDIR)$(PS)OSRTInputStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTStreamIF.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h \
 $(RTSRCDIR)$(PS)asn1CppTypes.h $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.h \
 $(RTSRCDIR)$(PS)asn1CppEvtHndlr.h $(RTSRCDIR)$(PS)asn1type.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTSRCDIR)$(PS)ASN1Context.h $(RTXSRCDIR)$(PS)OSRTMsgBuf.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h \
 $(RTSRCDIR)$(PS)ASN1TOctStr.h $(RTSRCDIR)$(PS)ASN1TOctStr64.h $(RTSRCDIR)$(PS)ASN1TObjId.h \
 $(XERSRCDIR)$(PS)asn1xer.h $(XERSRCDIR)$(PS)rtSAXDefs.h $(RTSRCDIR)$(PS)rt_common.hh \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(XERSRCDIR)$(PS)xed_common.hh $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)OSRTInputStream.h $(RTXSRCDIR)$(PS)OSRTStream.h
	$(CCC) $(XERCFLAGS) -c $(IPATHS) $(OBJOUT) $(XERSRCDIR)$(PS)asn1XerCppTypes.cpp

