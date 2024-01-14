
$(DESTDIR)$(PS)rtXmlExpatIF$(XMLIFOBJ): $(XMLSRCDIR)$(PS)rtXmlExpatIF.cpp \
 $(XMLSRCDIR)$(PS)rtSaxCppParserIF.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)OSRTInputStreamIF.h $(RTXSRCDIR)$(PS)OSRTStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h $(RTXSRCDIR)$(PS)OSRTContext.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h \
 $(RTXSRCDIR)$(PS)rtxEnum.h $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h \
 $(XMLSRCDIR)$(PS)rtSaxDefs.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(XMLSRCDIR)$(PS)rtXmlExternDefs.h $(XMLSRCDIR)$(PS)rtXmlErrCodes.h \
 $(XMLSRCDIR)$(PS)rtXmlNamespace.h $(RTXSRCDIR)$(PS)rtxDynPtrArray.h \
 $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxStreamBuffered.h $(EXPATSRCDIR)$(PS)expat.h \
 $(EXPATSRCDIR)$(PS)expat_external.h
	$(CCC) $(CFLAGSIF) -I$(EXPATSRCDIR) -c $(IPATHS)  $(XMLSRCDIR)/rtXmlExpatIF.cpp

$(DESTDIR)$(PS)rtXmlLibxml2IF$(XMLIFOBJ): $(XMLSRCDIR)$(PS)rtXmlLibxml2IF.cpp \
 $(LIBXML2INC)$(PS)libxml$(PS)parser.h \
 $(LIBXML2INC)$(PS)libxml$(PS)xmlversion.h \
 $(LIBXML2INC)$(PS)libxml$(PS)xmlexports.h \
 $(LIBXML2INC)$(PS)libxml$(PS)tree.h \
 $(LIBXML2INC)$(PS)libxml$(PS)xmlstring.h \
 $(LIBXML2INC)$(PS)libxml$(PS)xmlregexp.h \
 $(LIBXML2INC)$(PS)libxml$(PS)dict.h $(LIBXML2INC)$(PS)libxml$(PS)hash.h \
 $(LIBXML2INC)$(PS)libxml$(PS)valid.h \
 $(LIBXML2INC)$(PS)libxml$(PS)xmlerror.h \
 $(LIBXML2INC)$(PS)libxml$(PS)list.h \
 $(LIBXML2INC)$(PS)libxml$(PS)xmlautomata.h \
 $(LIBXML2INC)$(PS)libxml$(PS)entities.h \
 $(LIBXML2INC)$(PS)libxml$(PS)encoding.h \
 $(LIBXML2INC)$(PS)libxml$(PS)xmlIO.h \
 $(LIBXML2INC)$(PS)libxml$(PS)globals.h \
 $(LIBXML2INC)$(PS)libxml$(PS)SAX.h $(LIBXML2INC)$(PS)libxml$(PS)xlink.h \
 $(LIBXML2INC)$(PS)libxml$(PS)SAX2.h \
 $(LIBXML2INC)$(PS)libxml$(PS)xmlmemory.h \
 $(LIBXML2INC)$(PS)libxml$(PS)threads.h \
 $(LIBXML2INC)$(PS)libxml$(PS)parserInternals.h \
 $(LIBXML2INC)$(PS)libxml$(PS)HTMLparser.h \
 $(LIBXML2INC)$(PS)libxml$(PS)chvalid.h $(XMLSRCDIR)$(PS)rtSaxCppParserIF.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)OSRTInputStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTStreamIF.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(XMLSRCDIR)$(PS)osrtxml.h $(RTXSRCDIR)$(PS)rtxCommon.h $(RTXSRCDIR)$(PS)rtxBigInt.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxEnum.h \
 $(RTXSRCDIR)$(PS)rtxFile.h $(RTXSRCDIR)$(PS)rtxPattern.h $(RTXSRCDIR)$(PS)rtxReal.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTXSRCDIR)$(PS)rtxUtil.h $(XMLSRCDIR)$(PS)rtSaxDefs.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(XMLSRCDIR)$(PS)rtXmlExternDefs.h \
 $(XMLSRCDIR)$(PS)rtXmlErrCodes.h $(XMLSRCDIR)$(PS)rtXmlNamespace.h \
 $(RTXSRCDIR)$(PS)rtxDynPtrArray.h $(RTXSRCDIR)$(PS)rtxXmlQName.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CCC) $(CFLAGSIF) -I$(LIBXML2INC) -c $(IPATHS)  $(XMLSRCDIR)/rtXmlLibxml2IF.cpp
