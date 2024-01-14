# This file is only used by ASN1C.  It therefore excludes SAX-related sources
# except for rtSaxErrors, which actually is not SAX-specific.

OSXML_COMMON_RTOBJECTS = \
$(OBJDIR)$(PS)rtSaxErrors$(OBJ) \
$(OBJDIR)$(PS)rtXmlError$(OBJ) \
$(OBJDIR)$(PS)rtXmlCheckBuffer$(OBJ) \
$(OBJDIR)$(PS)rtXmlContextInit$(OBJ) \
$(OBJDIR)$(PS)rtXmlCmpBase64Str$(OBJ) \
$(OBJDIR)$(PS)rtXmlCmpHexStr$(OBJ) \
$(OBJDIR)$(PS)rtXmlCmpQName$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecBase64Binary$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecBase64Common$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecBase64StrValue$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecBase64Str$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecBigInt$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecBool$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecDateTime$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecDates$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecDecimal$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecDouble$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecDynBase64Str$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecDynHexStr$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecHexBinary$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecHexStr$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecHexStrValue$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecInt64$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecNSAttr$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecQName$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecUInt$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecUInt64$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecXSIAttr$(OBJ) \
$(OBJDIR)$(PS)rtXmlDecXSIAttrs$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncAny$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncAnyTypeValue$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncAttr$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncAttrC14N$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncBase64Binary$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncBase64BinaryAttr$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncBigInt$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncBigIntValue$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncBigIntAttr$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncBitString$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncBool$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncBoolAttr$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncComment$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncDates$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncDatesValue$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncDateTime$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncDateTimeValue$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncDecimal$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncDecimalValue$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncDecimalAttr$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncDecimalPattern$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncDouble$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncDoubleAttr$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncDoublePattern$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncDoubleValue$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncEmptyElement$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncEndDocument$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncEndElement$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncFloat$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncFloatAttr$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncHexBinary$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncHexBinaryAttr$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncIndent$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncInt$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncIntAttr$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncIntPattern$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncInt64$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncInt64Attr$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncInt64Pattern$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncNamedBits$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncNSAttrs$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncReal10$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncSoapEnvelope$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncSoapArrayTypeAttr$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncStartDocument$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncStartElement$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncUInt$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncUInt64$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncUIntAttr$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncUInt64Attr$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncUIntPattern$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncUInt64Pattern$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncUnicode$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncXSIAttrs$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncXSINilAttr$(OBJ) \
$(OBJDIR)$(PS)rtXmlEncXSITypeAttr$(OBJ) \
$(OBJDIR)$(PS)rtXmlKeyArray$(OBJ) \
$(OBJDIR)$(PS)rtXmlMatchHexStr$(OBJ) \
$(OBJDIR)$(PS)rtXmlMatchBase64Str$(OBJ) \
$(OBJDIR)$(PS)rtXmlMatchDateTime$(OBJ) \
$(OBJDIR)$(PS)rtXmlMatchDates$(OBJ) \
$(OBJDIR)$(PS)rtXmlMemFree$(OBJ) \
$(OBJDIR)$(PS)rtXmlNamespace$(OBJ) \
$(OBJDIR)$(PS)rtXmlNewQName$(OBJ) \
$(OBJDIR)$(PS)rtXmlParseElementName$(OBJ) \
$(OBJDIR)$(PS)rtXmlParseElemQName$(OBJ) \
$(OBJDIR)$(PS)rtXmlPrintNSAttrs$(OBJ) \
$(OBJDIR)$(PS)rtXmlPutChar$(OBJ) \
$(OBJDIR)$(PS)rtXmlSetEncodingStr$(OBJ) \
$(OBJDIR)$(PS)rtXmlStrCmpAsc$(OBJ) \
$(OBJDIR)$(PS)rtXmlStrnCmpAsc$(OBJ) \
$(OBJDIR)$(PS)rtXmlTreatWhitespaces$(OBJ) \
$(OBJDIR)$(PS)rtXmlWriteToFile$(OBJ) \
$(OBJDIR)$(PS)rtXmlWriteUTF16ToFile$(OBJ) \
$(OBJDIR)$(PS)rtXmlpCreateReader$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecAny$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecAnyElem$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecAnyAttrStr$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecBase64Str$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecBigInt$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecBitString$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecBool$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecDateTime$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecDecimal$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecDouble$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecDynBase64Str$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecDynHexStr$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecHexStr$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecInt$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecInt64$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecNamedBits$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecStrList$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecUInt$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecUInt64$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecUnicode$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecXmlStr$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecXSIAttr$(OBJ) \
$(OBJDIR)$(PS)rtXmlpDecXSITypeAttr$(OBJ) \
$(OBJDIR)$(PS)rtXmlpEvent$(OBJ) \
$(OBJDIR)$(PS)rtXmlpGetAttributeCount$(OBJ) \
$(OBJDIR)$(PS)rtXmlpGetAttributeID$(OBJ) \
$(OBJDIR)$(PS)rtXmlpGetContent$(OBJ) \
$(OBJDIR)$(PS)rtXmlpGetNextElem$(OBJ) \
$(OBJDIR)$(PS)rtXmlpGetNextElemID$(OBJ) \
$(OBJDIR)$(PS)rtXmlpHasAttributes$(OBJ) \
$(OBJDIR)$(PS)rtXmlpIsInGroup$(OBJ) \
$(OBJDIR)$(PS)rtXmlpIsUTF8Encoding$(OBJ) \
$(OBJDIR)$(PS)rtXmlpMatch$(OBJ) \
$(OBJDIR)$(PS)rtXmlpReadBytes$(OBJ) \
$(OBJDIR)$(PS)rtXmlpSelectAttribute$(OBJ) \
$(OBJDIR)$(PS)rtXmlWriteChars$(OBJ)

OSXML_LC_RTOBJECTS = \
$(LCOBJDIR)$(PS)rtXmlContext$(OBJ) \
$(LCOBJDIR)$(PS)rtXmlDecDynUTF8Str$(OBJ) \
$(LCOBJDIR)$(PS)rtXmlDecInt$(OBJ) \
$(LCOBJDIR)$(PS)rtXmlEncString$(OBJ) \
$(LCOBJDIR)$(PS)rtXmlPull$(OBJ) \
$(LCOBJDIR)$(PS)rtXmlSetEncBufPtr$(OBJ)

OSXML_C_BASEOBJECTS = \
$(OSXML_COMMON_RTOBJECTS) \
$(OBJDIR)$(PS)rtXmlEncAnyAttr$(OBJ)

OSXML_C_RTOBJECTS = \
$(OSXML_C_BASEOBJECTS) \
$(OSXML_LC_RTOBJECTS)

OSXML_CPP_RTOBJECTS = \
$(OSXML_C_RTOBJECTS) \
$(OBJDIR)$(PS)OSXMLMessageBuffer$(OBJ) \
$(OBJDIR)$(PS)OSXMLEncodeBuffer$(OBJ) \
$(OBJDIR)$(PS)OSXMLEncodeStream$(OBJ) \
$(OBJDIR)$(PS)OSXMLDecodeBuffer$(OBJ) 

LIBXML2EXT_OBJECTS = \
$(OBJDIR)$(PS)rtLx2DomSerializeToMem$(OBJ)
