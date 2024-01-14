ASN1BERDecodeStream$(OBJ): \
    $(OSROOTDIR)/rtbersrc/ASN1BERDecodeStream.h \
    $(OSROOTDIR)/rtxsrc/OSRTContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/OSRTMsgBufIF.h \
    $(OSROOTDIR)/rtxsrc/OSRTCtxtHolder.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/OSRTCtxtHolderIF.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtbersrc/asn1BerCppTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CppEvtHndlr64.h \
    $(BERSRCDIR)/ASN1BERDecodeStream.cpp \
    $(OSROOTDIR)/rtsrc/ASN1Context.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(BERSRCDIR)/asn1CRawEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rt3GPPTS32297.h \
    $(OSROOTDIR)/rtxsrc/OSRTInputStreamIF.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDiag.h \
    $(OSROOTDIR)/rtxsrc/OSRTMsgBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/ASN1TObjId.h \
    $(OSROOTDIR)/rtsrc/asn1CppEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxPrint.h \
    $(OSROOTDIR)/rtsrc/ASN1TOctStr64.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/asn1CppRawEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1CppTypes.h \
    $(OSROOTDIR)/rtxsrc/OSRTStreamIF.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxHexDump.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/ASN1TOctStr.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h
	$(CCC) $(BERCFLAGS) $(RTCPPFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/ASN1BERDecodeStream.cpp

ASN1BEREncodeStream$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1CppRawEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/ASN1TOctStr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/OSRTStreamIF.h \
    $(OSROOTDIR)/rtsrc/asn1CppTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/ASN1BEREncodeStream.cpp \
    $(OSROOTDIR)/rtsrc/ASN1TObjId.h \
    $(OSROOTDIR)/rtsrc/asn1CppEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/ASN1TOctStr64.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1CppEvtHndlr64.h \
    $(OSROOTDIR)/rtsrc/ASN1Context.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/OSRTCtxtHolderIF.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxDiag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/OSRTMsgBuf.h \
    $(OSROOTDIR)/rtbersrc/ASN1BEREncodeStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(BERSRCDIR)/asn1CRawEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/OSRTContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/OSRTMsgBufIF.h \
    $(OSROOTDIR)/rtxsrc/OSRTOutputStreamIF.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/OSRTCtxtHolder.h
	$(CCC) $(BERCFLAGS) $(RTCPPFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/ASN1BEREncodeStream.cpp

asn1BerCppTypes$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CppTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxHexDump.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/ASN1TOctStr.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1CppRawEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxPrint.h \
    $(OSROOTDIR)/rtsrc/ASN1TOctStr64.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/ASN1TObjId.h \
    $(BERSRCDIR)/asn1BerCppTypes.cpp \
    $(OSROOTDIR)/rtsrc/asn1CppEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxFile.h \
    $(OSROOTDIR)/rtsrc/rt3GPPTS32297.h \
    $(BERSRCDIR)/asn1CRawEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDiag.h \
    $(OSROOTDIR)/rtxsrc/OSRTMsgBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/OSRTCtxtHolderIF.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CppEvtHndlr64.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtbersrc/asn1BerCppTypes.h \
    $(OSROOTDIR)/rtsrc/ASN1Context.h \
    $(OSROOTDIR)/rtxsrc/OSRTCtxtHolder.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/OSRTContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/OSRTMsgBufIF.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/asn1intl.h
	$(CCC) $(BERCFLAGS) $(RTCPPFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/asn1BerCppTypes.cpp

