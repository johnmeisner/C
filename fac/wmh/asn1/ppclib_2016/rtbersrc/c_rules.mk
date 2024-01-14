berConvertDefIndef$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/berConvertDefIndef.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/asn1intl.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berConvertDefIndef.c

berDecCharArray$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(BERSRCDIR)/berDecCharArray.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecCharArray.c

berDecDecimal$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxDecimal.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/berDecDecimal.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecDecimal.c

berDecReal10$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(BERSRCDIR)/berDecReal10.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecReal10.c

berDecStrmBigInt$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(BERSRCDIR)/berDecStrmBigInt.c \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmBigInt.c

berDecStrmBitStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/berDecStrmBitStr.c \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmBitStr.c

berDecStrmBMPStr$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/berDecStrmBMPStr.c \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmBMPStr.c

berDecStrmBool$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(BERSRCDIR)/berDecStrmBool.c \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmBool.c

berDecStrmCharStr$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(BERSRCDIR)/berDecStrmCharStr.c \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmCharStr.c

berDecStrmCommon$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(BERSRCDIR)/berDecStrmCommon.c \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmCommon.c

berDecStrmDateStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(BERSRCDIR)/berDecStrmDateStr.c \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmDateStr.c

berDecStrmDateTimeStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/berDecStrmDateTimeStr.c \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmDateTimeStr.c

berDecStrmDecimal$(OBJ): \
    $(BERSRCDIR)/berDecStrmDecimal.c \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxDecimal.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmDecimal.c

berDecStrmDurationStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(BERSRCDIR)/berDecStrmDurationStr.c \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmDurationStr.c

berDecStrmDynBitStr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(BERSRCDIR)/berDecStrmDynBitStr.c \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmDynBitStr.c

berDecStrmDynOctStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(BERSRCDIR)/berDecStrmDynOctStr.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmDynOctStr.c

berDecStrmEnum$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(BERSRCDIR)/berDecStrmEnum.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmEnum.c

berDecStrmInt$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/berDecStrmInt.c \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmInt.c

berDecStrmInt16$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(BERSRCDIR)/berDecStrmInt16.c \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmInt16.c

berDecStrmInt64$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(BERSRCDIR)/berDecStrmInt64.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmInt64.c

berDecStrmInt8$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(BERSRCDIR)/berDecStrmInt8.c \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmInt8.c

berDecStrmNextElement$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(BERSRCDIR)/berDecStrmNextElement.c \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmNextElement.c

berDecStrmNull$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(BERSRCDIR)/berDecStrmNull.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmNull.c

berDecStrmObjId$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/berDecStrmObjId.c \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmObjId.c

berDecStrmObjId64$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(BERSRCDIR)/berDecStrmObjId64.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmObjId64.c

berDecStrmOctStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/berDecStrmOctStr.c \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmOctStr.c

berDecStrmOpenType$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/berDecStrmOpenType.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmOpenType.c

berDecStrmOpenTypeAppend$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/berDecStrmOpenTypeAppend.c \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmOpenTypeAppend.c

berDecStrmOpenTypeExt$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/berDecStrmOpenTypeExt.c \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmOpenTypeExt.c

berDecStrmPeekTagAndLen$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(BERSRCDIR)/berDecStrmPeekTagAndLen.c \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmPeekTagAndLen.c

berDecStrmReadTLV$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(BERSRCDIR)/berDecStrmReadTLV.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmReadTLV.c

berDecStrmReal$(OBJ): \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(BERSRCDIR)/berDecStrmReal.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmReal.c

berDecStrmReal10$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(BERSRCDIR)/berDecStrmReal10.c \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmReal10.c

berDecStrmRelativeOID$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(BERSRCDIR)/berDecStrmRelativeOID.c \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmRelativeOID.c

berDecStrmTimeOfDayStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/berDecStrmTimeOfDayStr.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmTimeOfDayStr.c

berDecStrmTimeStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/berDecStrmTimeStr.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmTimeStr.c

berDecStrmUInt$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(BERSRCDIR)/berDecStrmUInt.c \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmUInt.c

berDecStrmUInt16$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/berDecStrmUInt16.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmUInt16.c

berDecStrmUInt64$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/berDecStrmUInt64.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmUInt64.c

berDecStrmUInt8$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/berDecStrmUInt8.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmUInt8.c

berDecStrmUnivStr$(OBJ): \
    $(BERSRCDIR)/berDecStrmUnivStr.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmUnivStr.c

berEncReal10$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/berEncReal10.c \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncReal10.c

berEncStrmBigInt$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxBigInt.h \
    $(BERSRCDIR)/berEncStrmBigInt.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmBigInt.c

berEncStrmBitStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/berEncStrmBitStr.c \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmBitStr.c

berEncStrmBMPStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/berEncStrmBMPStr.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmBMPStr.c

berEncStrmBool$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(BERSRCDIR)/berEncStrmBool.c \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmBool.c

berEncStrmCharStr$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(BERSRCDIR)/berEncStrmCharStr.c \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmCharStr.c

berEncStrmCommon$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/berEncStrmCommon.c \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxUtil.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxSList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmCommon.c

berEncStrmDateStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(BERSRCDIR)/berEncStrmDateStr.c \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmDateStr.c

berEncStrmDateTimeStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/berEncStrmDateTimeStr.c \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmDateTimeStr.c

berEncStrmDurationStr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(BERSRCDIR)/berEncStrmDurationStr.c \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmDurationStr.c

berEncStrmEnum$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/berEncStrmEnum.c \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmEnum.c

berEncStrmInt$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/berEncStrmInt.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmInt.c

berEncStrmInt16$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/berEncStrmInt16.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmInt16.c

berEncStrmInt64$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(BERSRCDIR)/berEncStrmInt64.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmInt64.c

berEncStrmInt8$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/berEncStrmInt8.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmInt8.c

berEncStrmIntValue$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/berEncStrmIntValue.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmIntValue.c

berEncStrmNull$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(BERSRCDIR)/berEncStrmNull.c \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmNull.c

berEncStrmObjId$(OBJ): \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxUtil.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/berEncStrmObjId.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmObjId.c

berEncStrmObjId64$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxUtil.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(BERSRCDIR)/berEncStrmObjId64.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmObjId64.c

berEncStrmOctStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/berEncStrmOctStr.c \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmOctStr.c

berEncStrmOpenTypeExt$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/berEncStrmOpenTypeExt.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmOpenTypeExt.c

berEncStrmReal$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxUtil.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxReal.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/berEncStrmReal.c \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmReal.c

berEncStrmReal10$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(BERSRCDIR)/berEncStrmReal10.c \
    $(OSROOTDIR)/rtxsrc/rtxStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmReal10.c

berEncStrmRelativeOID$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(BERSRCDIR)/berEncStrmRelativeOID.c \
    $(OSROOTDIR)/rtxsrc/rtxUtil.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmRelativeOID.c

berEncStrmTimeOfDayStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/berEncStrmTimeOfDayStr.c \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmTimeOfDayStr.c

berEncStrmTimeStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(BERSRCDIR)/berEncStrmTimeStr.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmTimeStr.c

berEncStrmUInt$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(BERSRCDIR)/berEncStrmUInt.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmUInt.c

berEncStrmUInt16$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/berEncStrmUInt16.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxSList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmUInt16.c

berEncStrmUInt64$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/berEncStrmUInt64.c \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmUInt64.c

berEncStrmUInt8$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(BERSRCDIR)/berEncStrmUInt8.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmUInt8.c

berEncStrmUnivStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(BERSRCDIR)/berEncStrmUnivStr.c \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmUnivStr.c

berEncStrmXSDAny$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(BERSRCDIR)/berEncStrmXSDAny.c \
    $(OSROOTDIR)/rtxsrc/rtxStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmXSDAny.c

berError$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/berError.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berError.c

berGetLibInfo$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/asn1ber.h \
    $(BERSRCDIR)/berGetLibInfo.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1version.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berGetLibInfo.c

berParseTagLen$(OBJ): \
    $(BERSRCDIR)/berParseTagLen.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berParseTagLen.c

berSocket$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1intl.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSocket.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/asn1berSocket.h \
    $(BERSRCDIR)/berSocket.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berSocket.c

berStrmFreeContext$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/berStrmFreeContext.c \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berStrmFreeContext.c

berStrmInitContext$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(BERSRCDIR)/berStrmInitContext.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berStrmInitContext.c

berTagToString$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(BERSRCDIR)/berTagToString.c \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxCharStr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berTagToString.c

cerEncStrmBitStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/cerEncStrmBitStr.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/cerEncStrmBitStr.c

cerEncStrmBMPStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/cerEncStrmBMPStr.c \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/cerEncStrmBMPStr.c

cerEncStrmCharStr$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/cerEncStrmCharStr.c \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/cerEncStrmCharStr.c

cerEncStrmOctStr$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(BERSRCDIR)/cerEncStrmOctStr.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/cerEncStrmOctStr.c

cerEncStrmReal10$(OBJ): \
    $(BERSRCDIR)/cerEncStrmReal10.c \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/cerEncStrmReal10.c

cerEncStrmUnivStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/cerEncStrmUnivStr.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/cerEncStrmUnivStr.c

derEncBitString$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(BERSRCDIR)/derEncBitString.c \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/derEncBitString.c

derEncReal10$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/derEncReal10.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/derEncReal10.c

xd_16BitCharStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1intl.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(BERSRCDIR)/xd_16BitCharStr.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_16BitCharStr.c

xd_32BitCharStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1intl.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/xd_32BitCharStr.c \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_32BitCharStr.c

xd_bcdstr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxTBCD.h \
    $(BERSRCDIR)/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtBCD.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/xd_bcdstr.c \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_bcdstr.c

xd_bigint$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(BERSRCDIR)/xd_bigint.c \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_bigint.c

xd_bitstr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/xd_bitstr.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_bitstr.c

xd_bitstr_s$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/xd_bitstr_s.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_bitstr_s.c

xd_boolean$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(BERSRCDIR)/xd_boolean.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_boolean.c

xd_charstr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/xd_charstr.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxCharStr.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_charstr.c

xd_chkend$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(BERSRCDIR)/xd_chkend.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_chkend.c

xd_common$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/xd_common.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_common.c

xd_consstr$(OBJ): \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/xd_consstr.c \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_consstr.c

xd_count$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(BERSRCDIR)/xd_count.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_count.c

xd_datestr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/xd_datestr.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_datestr.c

xd_datetimestr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/xd_datetimestr.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_datetimestr.c

xd_durationstr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/xd_durationstr.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_durationstr.c

xd_enum$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/xd_enum.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_enum.c

xd_indeflen$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(BERSRCDIR)/xd_indeflen.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_indeflen.c

xd_int16$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/xd_int16.c \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_int16.c

xd_int64$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/xd_int64.c \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_int64.c

xd_int8$(OBJ): \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/xd_int8.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_int8.c

xd_integer$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(BERSRCDIR)/xd_integer.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_integer.c

xd_len$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(BERSRCDIR)/xd_len.c \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_len.c

xd_match$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/xd_match.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_match.c

xd_match1$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/xd_match1.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_match1.c

xd_memcpy$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/xd_memcpy.c \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_memcpy.c

xd_MovePastEOC$(OBJ): \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(BERSRCDIR)/xd_MovePastEOC.c \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_MovePastEOC.c

xd_NextElement$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/xd_NextElement.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_NextElement.c

xd_null$(OBJ): \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(BERSRCDIR)/xd_null.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_null.c

xd_objid$(OBJ): \
    $(BERSRCDIR)/xd_objid.c \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_objid.c

xd_octstr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(BERSRCDIR)/xd_octstr.c \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1intl.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_octstr.c

xd_octstr_s$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/xd_octstr_s.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_octstr_s.c

xd_oid64$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(BERSRCDIR)/xd_oid64.c \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_oid64.c

xd_OpenType$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(BERSRCDIR)/xd_OpenType.c \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_OpenType.c

xd_OpenTypeAppend$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/xd_OpenTypeAppend.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1intl.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_OpenTypeAppend.c

xd_OpenTypeExt$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/xd_OpenTypeExt.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_OpenTypeExt.c

xd_real$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxCtype.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/xd_real.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxReal.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_real.c

xd_reloid$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/xd_reloid.c \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_reloid.c

xd_setp$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/xd_setp.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_setp.c

xd_Tag1AndLen$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/xd_Tag1AndLen.c \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_Tag1AndLen.c

xd_timeofdaystr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/xd_timeofdaystr.c \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_timeofdaystr.c

xd_timestr$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/xd_timestr.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_timestr.c

xd_uint16$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/xd_uint16.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_uint16.c

xd_uint64$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(BERSRCDIR)/xd_uint64.c \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_uint64.c

xd_uint8$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/xd_uint8.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_uint8.c

xd_unsigned$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/xd_unsigned.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_unsigned.c

xdf_ReadContents$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(BERSRCDIR)/xdf_ReadContents.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xdf_ReadContents.c

xdf_TagAndLen$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/xdf_TagAndLen.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xdf_TagAndLen.c

xe_16BitCharStr$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(BERSRCDIR)/xe_16BitCharStr.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_16BitCharStr.c

xe_32BitCharStr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(BERSRCDIR)/xe_32BitCharStr.c \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_32BitCharStr.c

xe_bcdstr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtBCD.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/xe_bcdstr.c \
    $(OSROOTDIR)/rtxsrc/rtxTBCD.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_bcdstr.c

xe_bigint$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(BERSRCDIR)/xe_bigint.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxBigInt.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_bigint.c

xe_bitstr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/xe_bitstr.c \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_bitstr.c

xe_boolean$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(BERSRCDIR)/xe_boolean.c \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_boolean.c

xe_charstr$(OBJ): \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCharStr.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/xe_charstr.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_charstr.c

xe_common$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/xe_common.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_common.c

xe_datestr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/xe_datestr.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxCtype.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_datestr.c

xe_datetimestr$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(BERSRCDIR)/xe_datetimestr.c \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_datetimestr.c

xe_derCanonicalSort$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(BERSRCDIR)/xe_derCanonicalSort.c \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_derCanonicalSort.c

xe_durationstr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(BERSRCDIR)/xe_durationstr.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxCtype.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_durationstr.c

xe_enum$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/xe_enum.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_enum.c

xe_free$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/xe_free.c \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_free.c

xe_int16$(OBJ): \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/xe_int16.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_int16.c

xe_int64$(OBJ): \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/xe_int64.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_int64.c

xe_int8$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/xe_int8.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_int8.c

xe_integer$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/xe_integer.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_integer.c

xe_memcpy$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/xe_memcpy.c \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_memcpy.c

xe_null$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(BERSRCDIR)/xe_null.c \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_null.c

xe_objid$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(BERSRCDIR)/xe_objid.c \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_objid.c

xe_octstr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/xe_octstr.c \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_octstr.c

xe_oid64$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/xe_oid64.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_oid64.c

xe_OpenType$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/xe_OpenType.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_OpenType.c

xe_OpenTypeExt$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(BERSRCDIR)/xe_OpenTypeExt.c \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_OpenTypeExt.c

xe_ptr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/xe_ptr.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_ptr.c

xe_real$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/xe_real.c \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxReal.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxUtil.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_real.c

xe_reloid$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/xe_reloid.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_reloid.c

xe_TagAndIndefLen$(OBJ): \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/xe_TagAndIndefLen.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_TagAndIndefLen.c

xe_timeofdaystr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDateTime.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDateTime.hh \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCtype.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/xe_timeofdaystr.c \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_timeofdaystr.c

xe_timestr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/xe_timestr.c \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_timestr.c

xe_uint16$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/xe_uint16.c \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_uint16.c

xe_uint64$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/xe_uint64.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_uint64.c

xe_uint8$(OBJ): \
    $(BERSRCDIR)/xe_uint8.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_uint8.c

xe_unsigned$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(BERSRCDIR)/xe_unsigned.c \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_unsigned.c

xu_BufferState$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/xu_BufferState.c \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xu_BufferState.c

xu_dump2$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxPrintStream.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/xu_dump2.c \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xu_dump2.c

xu_fdump$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/xu_fdump.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xu_fdump.c

xu_fmt_contents$(OBJ): \
    $(BERSRCDIR)/xu_fmt_contents.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xu_fmt_contents.c

xu_fmt_tag$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(BERSRCDIR)/xu_fmt_tag.c \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCharStr.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xu_fmt_tag.c

