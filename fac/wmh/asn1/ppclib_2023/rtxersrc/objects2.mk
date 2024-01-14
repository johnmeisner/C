XER_C_OBJECTS_NLC = \
   $(OBJDIR)$(PS)xerCmpText$(OBJ) \
   $(OBJDIR)$(PS)xerContext$(OBJ) \
   $(OBJDIR)$(PS)xerCopyText$(OBJ) \
   $(OBJDIR)$(PS)xerDecBMPStr$(OBJ) \
   $(OBJDIR)$(PS)xerDecBase64StrValue$(OBJ) \
   $(OBJDIR)$(PS)xerDecBigInt$(OBJ) \
   $(OBJDIR)$(PS)xerDecBinStrValue$(OBJ) \
   $(OBJDIR)$(PS)xerDecBitStr$(OBJ) \
   $(OBJDIR)$(PS)xerDecBitStrMemBuf$(OBJ) \
   $(OBJDIR)$(PS)xerDecBool$(OBJ) \
   $(OBJDIR)$(PS)xerDecCopyBitStr$(OBJ) \
   $(OBJDIR)$(PS)xerDecCopyDynBitStr$(OBJ) \
   $(OBJDIR)$(PS)xerDecCopyDynOctStr$(OBJ) \
   $(OBJDIR)$(PS)xerDecCopyOctStr$(OBJ) \
   $(OBJDIR)$(PS)xerDecDynBase64Str$(OBJ) \
   $(OBJDIR)$(PS)xerDecDynBitStr$(OBJ) \
   $(OBJDIR)$(PS)xerDecDynUTF8Str$(OBJ) \
   $(OBJDIR)$(PS)xerDecHexStrValue$(OBJ) \
   $(OBJDIR)$(PS)xerDecInt64$(OBJ) \
   $(OBJDIR)$(PS)xerDecObjId64$(OBJ) \
   $(OBJDIR)$(PS)xerDecOctStrMemBuf$(OBJ) \
   $(OBJDIR)$(PS)xerDecOpenType$(OBJ) \
   $(OBJDIR)$(PS)xerDecReal$(OBJ) \
   $(OBJDIR)$(PS)xerDecReal10$(OBJ) \
   $(OBJDIR)$(PS)xerDecRelativeOID$(OBJ) \
   $(OBJDIR)$(PS)xerDecUInt$(OBJ) \
   $(OBJDIR)$(PS)xerDecUInt64$(OBJ) \
   $(OBJDIR)$(PS)xerDecUnivStr$(OBJ) \
   $(OBJDIR)$(PS)xerEncAscCharStr$(OBJ) \
   $(OBJDIR)$(PS)xerEncBMPStr$(OBJ) \
   $(OBJDIR)$(PS)xerEncBigInt$(OBJ) \
   $(OBJDIR)$(PS)xerEncBinStrValue$(OBJ) \
   $(OBJDIR)$(PS)xerEncBitStr$(OBJ) \
   $(OBJDIR)$(PS)xerEncBool$(OBJ) \
   $(OBJDIR)$(PS)xerEncChar$(OBJ) \
   $(OBJDIR)$(PS)xerEncEndDocument$(OBJ) \
   $(OBJDIR)$(PS)xerEncEndElement$(OBJ) \
   $(OBJDIR)$(PS)xerEncHexStrValue$(OBJ) \
   $(OBJDIR)$(PS)xerEncIndent$(OBJ) \
   $(OBJDIR)$(PS)xerEncInt64$(OBJ) \
   $(OBJDIR)$(PS)xerEncNamedValue$(OBJ) \
   $(OBJDIR)$(PS)xerEncNewLine$(OBJ) \
   $(OBJDIR)$(PS)xerEncNull$(OBJ) \
   $(OBJDIR)$(PS)xerEncObjId64$(OBJ) \
   $(OBJDIR)$(PS)xerEncOpenType$(OBJ) \
   $(OBJDIR)$(PS)xerEncOpenTypeExt$(OBJ) \
   $(OBJDIR)$(PS)xerEncReal$(OBJ) \
   $(OBJDIR)$(PS)xerEncReal10$(OBJ) \
   $(OBJDIR)$(PS)xerEncRelativeOID$(OBJ) \
   $(OBJDIR)$(PS)xerEncStartDocument$(OBJ) \
   $(OBJDIR)$(PS)xerEncStartElement$(OBJ) \
   $(OBJDIR)$(PS)xerEncUInt$(OBJ) \
   $(OBJDIR)$(PS)xerEncUInt64$(OBJ) \
   $(OBJDIR)$(PS)xerEncUniCharData$(OBJ) \
   $(OBJDIR)$(PS)xerEncUniCharStr$(OBJ) \
   $(OBJDIR)$(PS)xerEncUnivStr$(OBJ) \
   $(OBJDIR)$(PS)xerFinalizeMemBuf$(OBJ) \
   $(OBJDIR)$(PS)xerGetElemIdx$(OBJ) \
   $(OBJDIR)$(PS)xerGetLibInfo$(OBJ) \
   $(OBJDIR)$(PS)xerGetMsgPtrLen$(OBJ) \
   $(OBJDIR)$(PS)xerGetSeqElemIdx$(OBJ) \
   $(OBJDIR)$(PS)xerPutCharStr$(OBJ) \
   $(OBJDIR)$(PS)xerSetDecBufPtr$(OBJ) \
   $(OBJDIR)$(PS)xerSetEncBufPtr$(OBJ) \
   $(OBJDIR)$(PS)xerTextLength$(OBJ) \
   $(OBJDIR)$(PS)xerTextToCStr$(OBJ)

XER_C_OBJECTS_LC = \
   $(LCOBJDIR)$(PS)xerDecBase64Str$(OBJ) \
   $(LCOBJDIR)$(PS)xerDecDynAscCharStr$(OBJ) \
   $(LCOBJDIR)$(PS)xerDecDynOctStr$(OBJ) \
   $(LCOBJDIR)$(PS)xerDecInt$(OBJ) \
   $(LCOBJDIR)$(PS)xerDecObjId$(OBJ) \
   $(LCOBJDIR)$(PS)xerDecOctStr$(OBJ) \
   $(LCOBJDIR)$(PS)xerEncBase64Str$(OBJ) \
   $(LCOBJDIR)$(PS)xerEncEmptyElement$(OBJ) \
   $(LCOBJDIR)$(PS)xerEncInt$(OBJ) \
   $(LCOBJDIR)$(PS)xerEncObjId$(OBJ) \
   $(LCOBJDIR)$(PS)xerEncOctStr$(OBJ) \
   $(LCOBJDIR)$(PS)xerEncXmlCharStr$(OBJ)

XER_C_COMMON_OBJECTS = \
   $(XER_C_OBJECTS_NLC) \
   $(XER_C_OBJECTS_LC)

XER_C_OBJECTS = \
   $(XER_C_COMMON_OBJECTS) \
   $(OBJDIR)$(PS)ASN1SAX_XEROpenType$(OBJ)

XER_CPP_OBJECTS = \
   $(XER_C_COMMON_OBJECTS) \
   $(OBJDIR)$(PS)ASN1CXerOpenType$(OBJ) \
   $(OBJDIR)$(PS)ASN1XERDecodeStream$(OBJ) \
   $(OBJDIR)$(PS)ASN1XEREncodeStream$(OBJ) \
   $(OBJDIR)$(PS)ASN1XERString$(OBJ) \
   $(OBJDIR)$(PS)asn1XerCppTypes$(OBJ)
