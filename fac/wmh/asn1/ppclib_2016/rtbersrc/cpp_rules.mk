asn1BerCppTypes$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/OSRTCtxtHolderIF.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/OSRTMsgBufIF.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxHexDump.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1CppTypes.h \
    $(OSROOTDIR)/rtsrc/ASN1TObjId.h \
    $(OSROOTDIR)/rtsrc/asn1CppEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1intl.h \
    $(OSROOTDIR)/rtxsrc/rtxFile.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1CppEvtHndlr64.h \
    $(OSROOTDIR)/rtxsrc/rtxDiag.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1BerCppTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(BERSRCDIR)/asn1BerCppTypes.cpp \
    $(OSROOTDIR)/rtxsrc/OSRTMsgBuf.h \
    $(OSROOTDIR)/rtsrc/ASN1TOctStr.h \
    $(OSROOTDIR)/rtxsrc/OSRTContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/OSRTCtxtHolder.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxPrint.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/ASN1TOctStr64.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/ASN1Context.h
	$(CCC) $(BERCFLAGS) $(RTCPPFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/asn1BerCppTypes.cpp

ASN1BERDecodeStream$(OBJ): \
    $(BERSRCDIR)/ASN1BERDecodeStream.cpp \
    $(OSROOTDIR)/rtxsrc/OSRTContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/OSRTCtxtHolder.h \
    $(OSROOTDIR)/rtbersrc/ASN1BERDecodeStream.h \
    $(OSROOTDIR)/rtxsrc/OSRTInputStreamIF.h \
    $(OSROOTDIR)/rtxsrc/rtxPrint.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/ASN1TOctStr64.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/ASN1Context.h \
    $(OSROOTDIR)/rtxsrc/rtxDiag.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/asn1BerCppTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/OSRTMsgBuf.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/ASN1TOctStr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxHexDump.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1CppTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CppEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/ASN1TObjId.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1CppEvtHndlr64.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/OSRTCtxtHolderIF.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/OSRTMsgBufIF.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/OSRTStreamIF.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CCC) $(BERCFLAGS) $(RTCPPFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/ASN1BERDecodeStream.cpp

ASN1BEREncodeStream$(OBJ): \
    $(OSROOTDIR)/rtbersrc/ASN1BEREncodeStream.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/OSRTContext.h \
    $(OSROOTDIR)/rtsrc/ASN1Context.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/ASN1TOctStr64.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/OSRTCtxtHolder.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxDiag.h \
    $(OSROOTDIR)/rtsrc/ASN1TOctStr.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/OSRTMsgBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(BERSRCDIR)/ASN1BEREncodeStream.cpp \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1CppEvtHndlr64.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/ASN1TObjId.h \
    $(OSROOTDIR)/rtsrc/asn1CppEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1CppTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/OSRTMsgBufIF.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/OSRTCtxtHolderIF.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/OSRTStreamIF.h \
    $(OSROOTDIR)/rtxsrc/OSRTOutputStreamIF.h
	$(CCC) $(BERCFLAGS) $(RTCPPFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/ASN1BEREncodeStream.cpp
