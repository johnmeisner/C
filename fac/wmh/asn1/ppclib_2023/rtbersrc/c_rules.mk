berConvertDefIndef$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/berConvertDefIndef.c \
    $(OSROOTDIR)/rtsrc/asn1intl.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berConvertDefIndef.c

berDecCharArray$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/berDecCharArray.c \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecCharArray.c

berDecDecimal$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxDecimal.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(BERSRCDIR)/berDecDecimal.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecDecimal.c

berDecReal10$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(BERSRCDIR)/berDecReal10.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecReal10.c

berDecStrmBMPStr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(BERSRCDIR)/berDecStrmBMPStr.c \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmBMPStr.c

berDecStrmBigInt$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/berDecStrmBigInt.c \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmBigInt.c

berDecStrmBitStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(BERSRCDIR)/berDecStrmBitStr.c \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmBitStr.c

berDecStrmBool$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(BERSRCDIR)/berDecStrmBool.c \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmBool.c

berDecStrmCharStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/berDecStrmCharStr.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmCharStr.c

berDecStrmCommon$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/berDecStrmCommon.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmCommon.c

berDecStrmDateStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/berDecStrmDateStr.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmDateStr.c

berDecStrmDateTimeStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(BERSRCDIR)/berDecStrmDateTimeStr.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmDateTimeStr.c

berDecStrmDecimal$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(BERSRCDIR)/berDecStrmDecimal.c \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDecimal.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmDecimal.c

berDecStrmDurationStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(BERSRCDIR)/berDecStrmDurationStr.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmDurationStr.c

berDecStrmDynBitStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(BERSRCDIR)/berDecStrmDynBitStr.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmDynBitStr.c

berDecStrmDynObjId$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(BERSRCDIR)/berDecStrmDynObjId.c \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmDynObjId.c

berDecStrmDynOctStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(BERSRCDIR)/berDecStrmDynOctStr.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmDynOctStr.c

berDecStrmEnum$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(BERSRCDIR)/berDecStrmEnum.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmEnum.c

berDecStrmInt$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/berDecStrmInt.c \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmInt.c

berDecStrmInt16$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/berDecStrmInt16.c \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmInt16.c

berDecStrmInt64$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/berDecStrmInt64.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmInt64.c

berDecStrmInt8$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/berDecStrmInt8.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmInt8.c

berDecStrmNextElement$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/berDecStrmNextElement.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmNextElement.c

berDecStrmNull$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(BERSRCDIR)/berDecStrmNull.c \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmNull.c

berDecStrmOIDSubIds$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/berDecStrmOIDSubIds.c \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmOIDSubIds.c

berDecStrmObjId$(OBJ): \
    $(BERSRCDIR)/berDecStrmObjId.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmObjId.c

berDecStrmObjId64$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(BERSRCDIR)/berDecStrmObjId64.c \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmObjId64.c

berDecStrmOctStr$(OBJ): \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/berDecStrmOctStr.c \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmOctStr.c

berDecStrmOpenType$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/berDecStrmOpenType.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmOpenType.c

berDecStrmOpenTypeAppend$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(BERSRCDIR)/berDecStrmOpenTypeAppend.c \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmOpenTypeAppend.c

berDecStrmOpenTypeExt$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(BERSRCDIR)/berDecStrmOpenTypeExt.c \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmOpenTypeExt.c

berDecStrmPeekTagAndLen$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(BERSRCDIR)/berDecStrmPeekTagAndLen.c \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmPeekTagAndLen.c

berDecStrmReadTLV$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/berDecStrmReadTLV.c \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmReadTLV.c

berDecStrmReal$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(BERSRCDIR)/berDecStrmReal.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmReal.c

berDecStrmReal10$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/berDecStrmReal10.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmReal10.c

berDecStrmRelativeOID$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/berDecStrmRelativeOID.c \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmRelativeOID.c

berDecStrmTimeOfDayStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/berDecStrmTimeOfDayStr.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmTimeOfDayStr.c

berDecStrmTimeStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/berDecStrmTimeStr.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmTimeStr.c

berDecStrmUInt$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(BERSRCDIR)/berDecStrmUInt.c \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmUInt.c

berDecStrmUInt16$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/berDecStrmUInt16.c \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmUInt16.c

berDecStrmUInt64$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/berDecStrmUInt64.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmUInt64.c

berDecStrmUInt8$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(BERSRCDIR)/berDecStrmUInt8.c \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmUInt8.c

berDecStrmUnivStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(BERSRCDIR)/berDecStrmUnivStr.c \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berDecStrmUnivStr.c

berEncReal10$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(BERSRCDIR)/berEncReal10.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncReal10.c

berEncStrmBMPStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/berEncStrmBMPStr.c \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmBMPStr.c

berEncStrmBigInt$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxBigInt.h \
    $(BERSRCDIR)/berEncStrmBigInt.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmBigInt.c

berEncStrmBitStr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(BERSRCDIR)/berEncStrmBitStr.c \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmBitStr.c

berEncStrmBool$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/berEncStrmBool.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmBool.c

berEncStrmCharStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/berEncStrmCharStr.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmCharStr.c

berEncStrmCommon$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/berEncStrmCommon.c \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxUtil.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmCommon.c

berEncStrmDateStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/berEncStrmDateStr.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmDateStr.c

berEncStrmDateTimeStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(BERSRCDIR)/berEncStrmDateTimeStr.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmDateTimeStr.c

berEncStrmDurationStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(BERSRCDIR)/berEncStrmDurationStr.c \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmDurationStr.c

berEncStrmEnum$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/berEncStrmEnum.c \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmEnum.c

berEncStrmInt$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/berEncStrmInt.c \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmInt.c

berEncStrmInt16$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/berEncStrmInt16.c \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmInt16.c

berEncStrmInt64$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(BERSRCDIR)/berEncStrmInt64.c \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmInt64.c

berEncStrmInt8$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(BERSRCDIR)/berEncStrmInt8.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmInt8.c

berEncStrmIntValue$(OBJ): \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(BERSRCDIR)/berEncStrmIntValue.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmIntValue.c

berEncStrmNull$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/berEncStrmNull.c \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmNull.c

berEncStrmObjId$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/berEncStrmObjId.c \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxUtil.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmObjId.c

berEncStrmObjId64$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/berEncStrmObjId64.c \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxUtil.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmObjId64.c

berEncStrmOctStr$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(BERSRCDIR)/berEncStrmOctStr.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmOctStr.c

berEncStrmOpenTypeExt$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(BERSRCDIR)/berEncStrmOpenTypeExt.c \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmOpenTypeExt.c

berEncStrmReal$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/berEncStrmReal.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxReal.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxUtil.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmReal.c

berEncStrmReal10$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(BERSRCDIR)/berEncStrmReal10.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmReal10.c

berEncStrmRelativeOID$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxUtil.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/berEncStrmRelativeOID.c \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmRelativeOID.c

berEncStrmTimeOfDayStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(BERSRCDIR)/berEncStrmTimeOfDayStr.c \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmTimeOfDayStr.c

berEncStrmTimeStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/berEncStrmTimeStr.c \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmTimeStr.c

berEncStrmUInt$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/berEncStrmUInt.c \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmUInt.c

berEncStrmUInt16$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(BERSRCDIR)/berEncStrmUInt16.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmUInt16.c

berEncStrmUInt64$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(BERSRCDIR)/berEncStrmUInt64.c \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmUInt64.c

berEncStrmUInt8$(OBJ): \
    $(BERSRCDIR)/berEncStrmUInt8.c \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmUInt8.c

berEncStrmUnivStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/berEncStrmUnivStr.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmUnivStr.c

berEncStrmXSDAny$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/berEncStrmXSDAny.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berEncStrmXSDAny.c

berError$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(BERSRCDIR)/berError.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berError.c

berGetLibInfo$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1version.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/berGetLibInfo.c \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berGetLibInfo.c

berParseTagLen$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/berParseTagLen.c \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berParseTagLen.c

berSocket$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/berSocket.c \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSocket.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/asn1intl.h \
    $(OSROOTDIR)/rtbersrc/asn1berSocket.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berSocket.c

berStrmFreeContext$(OBJ): \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(BERSRCDIR)/berStrmFreeContext.c \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berStrmFreeContext.c

berStrmInitContext$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(BERSRCDIR)/berStrmInitContext.c \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berStrmInitContext.c

berTagToString$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxCharStr.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/berTagToString.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/berTagToString.c

cerEncStrmBMPStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(BERSRCDIR)/cerEncStrmBMPStr.c \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/cerEncStrmBMPStr.c

cerEncStrmBitStr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(BERSRCDIR)/cerEncStrmBitStr.c \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/cerEncStrmBitStr.c

cerEncStrmCharStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/cerEncStrmCharStr.c \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/cerEncStrmCharStr.c

cerEncStrmOctStr$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(BERSRCDIR)/cerEncStrmOctStr.c \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/cerEncStrmOctStr.c

cerEncStrmReal10$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/cerEncStrmReal10.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/cerEncStrmReal10.c

cerEncStrmUnivStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemBuf.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(BERSRCDIR)/cerEncStrmUnivStr.c \
    $(OSROOTDIR)/rtbersrc/asn1berStream.h \
    $(OSROOTDIR)/rtxsrc/rtxStream.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtbersrc/xse_common.hh \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/cerEncStrmUnivStr.c

derEncBitString$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/derEncBitString.c \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/derEncBitString.c

derEncReal10$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/derEncReal10.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/derEncReal10.c

xd_16BitCharStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/asn1intl.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/xd_16BitCharStr.c \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_16BitCharStr.c

xd_32BitCharStr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(BERSRCDIR)/xd_32BitCharStr.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/asn1intl.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_32BitCharStr.c

xd_MovePastEOC$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/xd_MovePastEOC.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_MovePastEOC.c

xd_NextElement$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/xd_NextElement.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_NextElement.c

xd_OpenType$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/xd_OpenType.c \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_OpenType.c

xd_OpenTypeAppend$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1intl.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/xd_OpenTypeAppend.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_OpenTypeAppend.c

xd_OpenTypeExt$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(BERSRCDIR)/xd_OpenTypeExt.c \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_OpenTypeExt.c

xd_Tag1AndLen$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/xd_Tag1AndLen.c \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_Tag1AndLen.c

xd_bcdstr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/rtBCD.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxTBCD.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/xd_bcdstr.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_bcdstr.c

xd_bigint$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(BERSRCDIR)/xd_bigint.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_bigint.c

xd_bitstr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/xd_bitstr.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_bitstr.c

xd_bitstr_s$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(BERSRCDIR)/xd_bitstr_s.c \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_bitstr_s.c

xd_boolean$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/xd_boolean.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_boolean.c

xd_charstr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/xd_charstr.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxCharStr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_charstr.c

xd_chkend$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/xd_chkend.c \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_chkend.c

xd_common$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxevalsrc/expeval.hh \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(BERSRCDIR)/xd_common.c \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/wceAddon.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxevalsrc/rtxEval.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_common.c

xd_consstr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/xd_consstr.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_consstr.c

xd_count$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/xd_count.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_count.c

xd_datestr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/xd_datestr.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_datestr.c

xd_datetimestr$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/xd_datetimestr.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_datetimestr.c

xd_durationstr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(BERSRCDIR)/xd_durationstr.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_durationstr.c

xd_dynObjId$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/xd_dynObjId.c \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_dynObjId.c

xd_enum$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(BERSRCDIR)/xd_enum.c \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_enum.c

xd_indeflen$(OBJ): \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(BERSRCDIR)/xd_indeflen.c \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_indeflen.c

xd_int16$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(BERSRCDIR)/xd_int16.c \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_int16.c

xd_int64$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/xd_int64.c \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_int64.c

xd_int8$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(BERSRCDIR)/xd_int8.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_int8.c

xd_integer$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/xd_integer.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_integer.c

xd_len$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(BERSRCDIR)/xd_len.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_len.c

xd_match$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/xd_match.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_match.c

xd_match1$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/xd_match1.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_match1.c

xd_memcpy$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(BERSRCDIR)/xd_memcpy.c \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_memcpy.c

xd_null$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/xd_null.c \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_null.c

xd_objid$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/xd_objid.c \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_objid.c

xd_octstr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/xd_octstr.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/asn1intl.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_octstr.c

xd_octstr_s$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(BERSRCDIR)/xd_octstr_s.c \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_octstr_s.c

xd_oid64$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/xd_oid64.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_oid64.c

xd_real$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxevalsrc/expeval.hh \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxReal.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/wceAddon.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/xd_real.c \
    $(OSROOTDIR)/rtxsrc/rtxCtype.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxevalsrc/rtxEval.hh
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_real.c

xd_reloid$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/xd_reloid.c \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_reloid.c

xd_setp$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(BERSRCDIR)/xd_setp.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_setp.c

xd_timeofdaystr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/xd_timeofdaystr.c \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_timeofdaystr.c

xd_timestr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/xd_timestr.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_timestr.c

xd_uint16$(OBJ): \
    $(BERSRCDIR)/xd_uint16.c \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_uint16.c

xd_uint64$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(BERSRCDIR)/xd_uint64.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_uint64.c

xd_uint8$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(BERSRCDIR)/xd_uint8.c \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_uint8.c

xd_unsigned$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/xd_unsigned.c \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xd_unsigned.c

xdf_ReadContents$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/xdf_ReadContents.c \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xdf_ReadContents.c

xdf_TagAndLen$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/xdf_TagAndLen.c \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xdf_TagAndLen.c

xe_16BitCharStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(BERSRCDIR)/xe_16BitCharStr.c \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_16BitCharStr.c

xe_32BitCharStr$(OBJ): \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(BERSRCDIR)/xe_32BitCharStr.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_32BitCharStr.c

xe_OpenType$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/xe_OpenType.c \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_OpenType.c

xe_OpenTypeExt$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(BERSRCDIR)/xe_OpenTypeExt.c \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_OpenTypeExt.c

xe_TagAndIndefLen$(OBJ): \
    $(BERSRCDIR)/xe_TagAndIndefLen.c \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_TagAndIndefLen.c

xe_bcdstr$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtBCD.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/xe_bcdstr.c \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxTBCD.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_bcdstr.c

xe_bigint$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(BERSRCDIR)/xe_bigint.c \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBigInt.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_bigint.c

xe_bitstr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(BERSRCDIR)/xe_bitstr.c \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_bitstr.c

xe_boolean$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/xe_boolean.c \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_boolean.c

xe_cer16BitCharStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/xe_cer16BitCharStr.c \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_cer16BitCharStr.c

xe_cer32BitCharStr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/xe_cer32BitCharStr.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_cer32BitCharStr.c

xe_cerbitstr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(BERSRCDIR)/xe_cerbitstr.c \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_cerbitstr.c

xe_cercharstr$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxCharStr.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/xe_cercharstr.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_cercharstr.c

xe_ceroctstr$(OBJ): \
    $(BERSRCDIR)/xe_ceroctstr.c \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_ceroctstr.c

xe_charstr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxCharStr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(BERSRCDIR)/xe_charstr.c \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_charstr.c

xe_common$(OBJ): \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/xe_common.c \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxevalsrc/expeval.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxevalsrc/rtxEval.hh \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/wceAddon.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_common.c

xe_datestr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxCtype.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/xe_datestr.c \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_datestr.c

xe_datetimestr$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/xe_datetimestr.c \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_datetimestr.c

xe_derCanonicalSort$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/xe_derCanonicalSort.c \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_derCanonicalSort.c

xe_durationstr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCtype.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/xe_durationstr.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_durationstr.c

xe_enum$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/xe_enum.c \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_enum.c

xe_free$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(BERSRCDIR)/xe_free.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_free.c

xe_int16$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(BERSRCDIR)/xe_int16.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_int16.c

xe_int64$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/xe_int64.c \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_int64.c

xe_int8$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(BERSRCDIR)/xe_int8.c \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_int8.c

xe_integer$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(BERSRCDIR)/xe_integer.c \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_integer.c

xe_memcpy$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(BERSRCDIR)/xe_memcpy.c \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_memcpy.c

xe_null$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(BERSRCDIR)/xe_null.c \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_null.c

xe_objid$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(BERSRCDIR)/xe_objid.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_objid.c

xe_octstr$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(BERSRCDIR)/xe_octstr.c \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_octstr.c

xe_oid64$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(BERSRCDIR)/xe_oid64.c \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_oid64.c

xe_ptr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(BERSRCDIR)/xe_ptr.c \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_ptr.c

xe_real$(OBJ): \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxevalsrc/expeval.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxevalsrc/rtxEval.hh \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/xe_real.c \
    $(OSROOTDIR)/rtxsrc/wceAddon.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxUtil.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxReal.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_real.c

xe_reloid$(OBJ): \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(BERSRCDIR)/xe_reloid.c \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_reloid.c

xe_timeofdaystr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxCtype.h \
    $(OSROOTDIR)/rtxsrc/rtxDateTime.hh \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDateTime.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/xe_timeofdaystr.c \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_timeofdaystr.c

xe_timestr$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(BERSRCDIR)/xe_timestr.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_timestr.c

xe_uint16$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/xe_uint16.c \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_uint16.c

xe_uint64$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(BERSRCDIR)/xe_uint64.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_uint64.c

xe_uint8$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(BERSRCDIR)/xe_uint8.c \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_uint8.c

xe_unsigned$(OBJ): \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(BERSRCDIR)/xe_unsigned.c \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.hh \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xe_unsigned.c

xu_BufferState$(OBJ): \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(BERSRCDIR)/xu_BufferState.c
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xu_BufferState.c

xu_dump2$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(BERSRCDIR)/xu_dump2.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/rtxPrintStream.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xu_dump2.c

xu_fdump$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(BERSRCDIR)/xu_fdump.c \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xu_fdump.c

xu_fmt_contents$(OBJ): \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(BERSRCDIR)/xu_fmt_contents.c \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtxsrc/rtxHexDump.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xu_fmt_contents.c

xu_fmt_tag$(OBJ): \
    $(OSROOTDIR)/rtxsrc/rtxError.h \
    $(OSROOTDIR)/rtxsrc/osMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxContext.h \
    $(OSROOTDIR)/rtxsrc/osSysTypes.h \
    $(OSROOTDIR)/rtxsrc/rtxBuffer.h \
    $(OSROOTDIR)/rtsrc/asn1type.h \
    $(OSROOTDIR)/rtsrc/asn1CEvtHndlr.h \
    $(OSROOTDIR)/rtxsrc/rtxExternDefs.h \
    $(OSROOTDIR)/rtsrc/asn1ErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxDList.h \
    $(OSROOTDIR)/rtsrc/asn1tag.h \
    $(OSROOTDIR)/rtxsrc/rtxStack.h \
    $(OSROOTDIR)/rtbersrc/asn1ber.h \
    $(OSROOTDIR)/rtxsrc/rtxMemory.h \
    $(OSROOTDIR)/rtsrc/rtContext.h \
    $(OSROOTDIR)/rtbersrc/berMacros.h \
    $(OSROOTDIR)/rtxsrc/rtxCharStr.h \
    $(OSROOTDIR)/rtxsrc/rtxErrCodes.h \
    $(OSROOTDIR)/rtxsrc/rtxSList.h \
    $(OSROOTDIR)/rtxsrc/rtxBitString.h \
    $(BERSRCDIR)/xu_fmt_tag.c \
    $(OSROOTDIR)/rtxsrc/rtxCommonDefs.h \
    $(OSROOTDIR)/rtxsrc/rtxUTF8.h \
    $(OSROOTDIR)/rtsrc/rtExternDefs.h
	$(CC) $(BERCFLAGS) -c -I$(OSROOTDIR) $(IPATHS_) $(BERSRCDIR)/xu_fmt_tag.c

