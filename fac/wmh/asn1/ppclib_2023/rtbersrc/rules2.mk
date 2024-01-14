# C/C++ to object file compilation rules

$(OBJDIR)$(PS)berConvertDefIndef$(OBJ): $(BERSRCDIR)$(PS)berConvertDefIndef.c \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTSRCDIR)$(PS)asn1intl.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berConvertDefIndef.c


$(OBJDIR)$(PS)berError$(OBJ): $(BERSRCDIR)$(PS)berError.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berError.c


$(OBJDIR)$(PS)berGetLibInfo$(OBJ): $(BERSRCDIR)$(PS)berGetLibInfo.c $(RTSRCDIR)$(PS)asn1version.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berGetLibInfo.c


$(OBJDIR)$(PS)berParseTagLen$(OBJ): $(BERSRCDIR)$(PS)berParseTagLen.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berParseTagLen.c


$(OBJDIR)$(PS)berTagToString$(OBJ): $(BERSRCDIR)$(PS)berTagToString.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berTagToString.c


$(OBJDIR)$(PS)xd_real$(OBJ): $(BERSRCDIR)$(PS)xd_real.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h $(RTXSRCDIR)$(PS)rtxReal.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_real.c


$(OBJDIR)$(PS)xd_indeflen$(OBJ): $(BERSRCDIR)$(PS)xd_indeflen.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_indeflen.c


$(OBJDIR)$(PS)xd_enum$(OBJ): $(BERSRCDIR)$(PS)xd_enum.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_enum.c


$(OBJDIR)$(PS)xd_count$(OBJ): $(BERSRCDIR)$(PS)xd_count.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_count.c


$(OBJDIR)$(PS)xd_common$(OBJ): $(BERSRCDIR)$(PS)xd_common.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_common.c


$(OBJDIR)$(PS)xd_match$(OBJ): $(BERSRCDIR)$(PS)xd_match.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_match.c


$(OBJDIR)$(PS)xd_match1$(OBJ): $(BERSRCDIR)$(PS)xd_match1.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_match1.c


$(OBJDIR)$(PS)xd_chkend$(OBJ): $(BERSRCDIR)$(PS)xd_chkend.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_chkend.c


$(OBJDIR)$(PS)xd_bcdstr$(OBJ): $(BERSRCDIR)$(PS)xd_bcdstr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTSRCDIR)$(PS)rtBCD.h $(RTXSRCDIR)$(PS)rtxTBCD.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_bcdstr.c


$(OBJDIR)$(PS)xd_16BitCharStr$(OBJ): $(BERSRCDIR)$(PS)xd_16BitCharStr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTSRCDIR)$(PS)asn1intl.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_16BitCharStr.c


$(OBJDIR)$(PS)xd_32BitCharStr$(OBJ): $(BERSRCDIR)$(PS)xd_32BitCharStr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTSRCDIR)$(PS)asn1intl.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_32BitCharStr.c


$(OBJDIR)$(PS)xd_OpenTypeExt$(OBJ): $(BERSRCDIR)$(PS)xd_OpenTypeExt.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_OpenTypeExt.c


$(OBJDIR)$(PS)xd_OpenTypeAppend$(OBJ): $(BERSRCDIR)$(PS)xd_OpenTypeAppend.c \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTSRCDIR)$(PS)asn1intl.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_OpenTypeAppend.c


$(OBJDIR)$(PS)xd_octstr_s$(OBJ): $(BERSRCDIR)$(PS)xd_octstr_s.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_octstr_s.c


$(OBJDIR)$(PS)xd_oid64$(OBJ): $(BERSRCDIR)$(PS)xd_oid64.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_oid64.c


$(OBJDIR)$(PS)xd_reloid$(OBJ): $(BERSRCDIR)$(PS)xd_reloid.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_reloid.c


$(OBJDIR)$(PS)xd_null$(OBJ): $(BERSRCDIR)$(PS)xd_null.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_null.c


$(OBJDIR)$(PS)xd_NextElement$(OBJ): $(BERSRCDIR)$(PS)xd_NextElement.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_NextElement.c


$(OBJDIR)$(PS)xd_MovePastEOC$(OBJ): $(BERSRCDIR)$(PS)xd_MovePastEOC.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_MovePastEOC.c


$(OBJDIR)$(PS)xd_memcpy$(OBJ): $(BERSRCDIR)$(PS)xd_memcpy.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_memcpy.c


$(OBJDIR)$(PS)xd_len$(OBJ): $(BERSRCDIR)$(PS)xd_len.c $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_len.c


$(OBJDIR)$(PS)xd_int16$(OBJ): $(BERSRCDIR)$(PS)xd_int16.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_int16.c


$(OBJDIR)$(PS)xd_int64$(OBJ): $(BERSRCDIR)$(PS)xd_int64.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_int64.c


$(OBJDIR)$(PS)xd_uint64$(OBJ): $(BERSRCDIR)$(PS)xd_uint64.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_uint64.c


$(OBJDIR)$(PS)xd_setp$(OBJ): $(BERSRCDIR)$(PS)xd_setp.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_setp.c


$(OBJDIR)$(PS)xd_Tag1AndLen$(OBJ): $(BERSRCDIR)$(PS)xd_Tag1AndLen.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_Tag1AndLen.c


$(OBJDIR)$(PS)xd_consstr$(OBJ): $(BERSRCDIR)$(PS)xd_consstr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_consstr.c


$(OBJDIR)$(PS)xdf_TagAndLen$(OBJ): $(BERSRCDIR)$(PS)xdf_TagAndLen.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xdf_TagAndLen.c


$(OBJDIR)$(PS)xdf_ReadContents$(OBJ): $(BERSRCDIR)$(PS)xdf_ReadContents.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xdf_ReadContents.c


$(OBJDIR)$(PS)xe_real$(OBJ): $(BERSRCDIR)$(PS)xe_real.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_real.c


$(OBJDIR)$(PS)xe_cer16BitCharStr$(OBJ): $(BERSRCDIR)$(PS)xe_cer16BitCharStr.c \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_cer16BitCharStr.c


$(OBJDIR)$(PS)xe_cer32BitCharStr$(OBJ): $(BERSRCDIR)$(PS)xe_cer32BitCharStr.c \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_cer32BitCharStr.c


$(OBJDIR)$(PS)xe_cerbitstr$(OBJ): $(BERSRCDIR)$(PS)xe_cerbitstr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_cerbitstr.c


$(OBJDIR)$(PS)xe_cercharstr$(OBJ): $(BERSRCDIR)$(PS)xe_cercharstr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_cercharstr.c


$(OBJDIR)$(PS)xe_ceroctstr$(OBJ): $(BERSRCDIR)$(PS)xe_ceroctstr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_ceroctstr.c


$(OBJDIR)$(PS)xe_common$(OBJ): $(BERSRCDIR)$(PS)xe_common.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_common.c


$(OBJDIR)$(PS)xe_charstr$(OBJ): $(BERSRCDIR)$(PS)xe_charstr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_charstr.c


$(OBJDIR)$(PS)xe_datestr$(OBJ): $(BERSRCDIR)$(PS)xe_datestr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_datestr.c


$(OBJDIR)$(PS)xe_timestr$(OBJ): $(BERSRCDIR)$(PS)xe_timestr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_timestr.c


$(OBJDIR)$(PS)xe_datetimestr$(OBJ): $(BERSRCDIR)$(PS)xe_datetimestr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_datetimestr.c


$(OBJDIR)$(PS)xe_timeofdaystr$(OBJ): $(BERSRCDIR)$(PS)xe_timeofdaystr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDateTime.hh $(RTXSRCDIR)$(PS)rtxDateTime.h $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_timeofdaystr.c


$(OBJDIR)$(PS)xe_durationstr$(OBJ): $(BERSRCDIR)$(PS)xe_durationstr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCtype.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_durationstr.c


$(OBJDIR)$(PS)xe_bcdstr$(OBJ): $(BERSRCDIR)$(PS)xe_bcdstr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTSRCDIR)$(PS)rtBCD.h $(RTXSRCDIR)$(PS)rtxTBCD.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_bcdstr.c


$(OBJDIR)$(PS)xe_16BitCharStr$(OBJ): $(BERSRCDIR)$(PS)xe_16BitCharStr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_16BitCharStr.c


$(OBJDIR)$(PS)xe_32BitCharStr$(OBJ): $(BERSRCDIR)$(PS)xe_32BitCharStr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_32BitCharStr.c


$(OBJDIR)$(PS)xe_TagAndIndefLen$(OBJ): $(BERSRCDIR)$(PS)xe_TagAndIndefLen.c \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_TagAndIndefLen.c


$(OBJDIR)$(PS)xe_OpenType$(OBJ): $(BERSRCDIR)$(PS)xe_OpenType.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_OpenType.c


$(OBJDIR)$(PS)xe_OpenTypeExt$(OBJ): $(BERSRCDIR)$(PS)xe_OpenTypeExt.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_OpenTypeExt.c


$(OBJDIR)$(PS)xe_oid64$(OBJ): $(BERSRCDIR)$(PS)xe_oid64.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_oid64.c


$(OBJDIR)$(PS)xe_reloid$(OBJ): $(BERSRCDIR)$(PS)xe_reloid.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_reloid.c


$(OBJDIR)$(PS)xe_null$(OBJ): $(BERSRCDIR)$(PS)xe_null.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_null.c


$(OBJDIR)$(PS)xe_memcpy$(OBJ): $(BERSRCDIR)$(PS)xe_memcpy.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_memcpy.c


$(OBJDIR)$(PS)xe_int16$(OBJ): $(BERSRCDIR)$(PS)xe_int16.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_int16.c


$(OBJDIR)$(PS)xe_int64$(OBJ): $(BERSRCDIR)$(PS)xe_int64.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_int64.c


$(OBJDIR)$(PS)xe_uint64$(OBJ): $(BERSRCDIR)$(PS)xe_uint64.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_uint64.c


$(OBJDIR)$(PS)xe_enum$(OBJ): $(BERSRCDIR)$(PS)xe_enum.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_enum.c


$(OBJDIR)$(PS)xe_ptr$(OBJ): $(BERSRCDIR)$(PS)xe_ptr.c $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_ptr.c


$(OBJDIR)$(PS)xe_free$(OBJ): $(BERSRCDIR)$(PS)xe_free.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_free.c


$(OBJDIR)$(PS)xu_BufferState$(OBJ): $(BERSRCDIR)$(PS)xu_BufferState.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xu_BufferState.c


$(OBJDIR)$(PS)xu_dump2$(OBJ): $(BERSRCDIR)$(PS)xu_dump2.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxPrintStream.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xu_dump2.c


$(OBJDIR)$(PS)xu_fdump$(OBJ): $(BERSRCDIR)$(PS)xu_fdump.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xu_fdump.c


$(OBJDIR)$(PS)xu_fmt_tag$(OBJ): $(BERSRCDIR)$(PS)xu_fmt_tag.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xu_fmt_tag.c


$(OBJDIR)$(PS)xu_fmt_contents$(OBJ): $(BERSRCDIR)$(PS)xu_fmt_contents.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxHexDump.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xu_fmt_contents.c


$(OBJDIR)$(PS)berDecDecimal$(OBJ): $(BERSRCDIR)$(PS)berDecDecimal.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxDecimal.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecDecimal.c


$(OBJDIR)$(PS)berDecReal10$(OBJ): $(BERSRCDIR)$(PS)berDecReal10.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecReal10.c


$(OBJDIR)$(PS)berEncReal10$(OBJ): $(BERSRCDIR)$(PS)berEncReal10.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncReal10.c


$(OBJDIR)$(PS)derEncReal10$(OBJ): $(BERSRCDIR)$(PS)derEncReal10.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)derEncReal10.c


$(OBJDIR)$(PS)berStrmInitContext$(OBJ): $(BERSRCDIR)$(PS)berStrmInitContext.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berStrmInitContext.c


$(OBJDIR)$(PS)berStrmFreeContext$(OBJ): $(BERSRCDIR)$(PS)berStrmFreeContext.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berStrmFreeContext.c


$(OBJDIR)$(PS)berEncStrmBigInt$(OBJ): $(BERSRCDIR)$(PS)berEncStrmBigInt.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmBigInt.c


$(OBJDIR)$(PS)berEncStrmBitStr$(OBJ): $(BERSRCDIR)$(PS)berEncStrmBitStr.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmBitStr.c


$(OBJDIR)$(PS)berEncStrmBMPStr$(OBJ): $(BERSRCDIR)$(PS)berEncStrmBMPStr.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmBMPStr.c


$(OBJDIR)$(PS)berEncStrmBool$(OBJ): $(BERSRCDIR)$(PS)berEncStrmBool.c $(BERSRCDIR)$(PS)xse_common.hh \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmBool.c


$(OBJDIR)$(PS)berEncStrmCharStr$(OBJ): $(BERSRCDIR)$(PS)berEncStrmCharStr.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmCharStr.c


$(OBJDIR)$(PS)berEncStrmDateStr$(OBJ): $(BERSRCDIR)$(PS)berEncStrmDateStr.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmDateStr.c


$(OBJDIR)$(PS)berEncStrmDateTimeStr$(OBJ): $(BERSRCDIR)$(PS)berEncStrmDateTimeStr.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmDateTimeStr.c


$(OBJDIR)$(PS)berEncStrmDurationStr$(OBJ): $(BERSRCDIR)$(PS)berEncStrmDurationStr.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmDurationStr.c


$(OBJDIR)$(PS)berEncStrmTimeStr$(OBJ): $(BERSRCDIR)$(PS)berEncStrmTimeStr.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmTimeStr.c


$(OBJDIR)$(PS)berEncStrmTimeOfDayStr$(OBJ): $(BERSRCDIR)$(PS)berEncStrmTimeOfDayStr.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmTimeOfDayStr.c


$(OBJDIR)$(PS)berEncStrmCommon$(OBJ): $(BERSRCDIR)$(PS)berEncStrmCommon.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmCommon.c


$(OBJDIR)$(PS)berEncStrmEnum$(OBJ): $(BERSRCDIR)$(PS)berEncStrmEnum.c $(BERSRCDIR)$(PS)xse_common.hh \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmEnum.c


$(OBJDIR)$(PS)berEncStrmInt8$(OBJ): $(BERSRCDIR)$(PS)berEncStrmInt8.c $(BERSRCDIR)$(PS)xse_common.hh \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmInt8.c


$(OBJDIR)$(PS)berEncStrmInt16$(OBJ): $(BERSRCDIR)$(PS)berEncStrmInt16.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmInt16.c


$(OBJDIR)$(PS)berEncStrmInt64$(OBJ): $(BERSRCDIR)$(PS)berEncStrmInt64.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmInt64.c


$(OBJDIR)$(PS)berEncStrmIntValue$(OBJ): $(BERSRCDIR)$(PS)berEncStrmIntValue.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmIntValue.c


$(OBJDIR)$(PS)berEncStrmNull$(OBJ): $(BERSRCDIR)$(PS)berEncStrmNull.c $(BERSRCDIR)$(PS)xse_common.hh \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmNull.c


$(OBJDIR)$(PS)berEncStrmObjId64$(OBJ): $(BERSRCDIR)$(PS)berEncStrmObjId64.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmObjId64.c


$(OBJDIR)$(PS)berEncStrmReal$(OBJ): $(BERSRCDIR)$(PS)berEncStrmReal.c $(BERSRCDIR)$(PS)xse_common.hh \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxReal.h $(RTXSRCDIR)$(PS)rtxUtil.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmReal.c


$(OBJDIR)$(PS)berEncStrmRelativeOID$(OBJ): $(BERSRCDIR)$(PS)berEncStrmRelativeOID.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxUtil.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmRelativeOID.c


$(OBJDIR)$(PS)berEncStrmUInt$(OBJ): $(BERSRCDIR)$(PS)berEncStrmUInt.c $(BERSRCDIR)$(PS)xse_common.hh \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmUInt.c


$(OBJDIR)$(PS)berEncStrmUInt8$(OBJ): $(BERSRCDIR)$(PS)berEncStrmUInt8.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmUInt8.c


$(OBJDIR)$(PS)berEncStrmUInt64$(OBJ): $(BERSRCDIR)$(PS)berEncStrmUInt64.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmUInt64.c


$(OBJDIR)$(PS)berEncStrmUnivStr$(OBJ): $(BERSRCDIR)$(PS)berEncStrmUnivStr.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmUnivStr.c


$(OBJDIR)$(PS)berEncStrmOpenTypeExt$(OBJ): $(BERSRCDIR)$(PS)berEncStrmOpenTypeExt.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmOpenTypeExt.c


$(OBJDIR)$(PS)berEncStrmXSDAny$(OBJ): $(BERSRCDIR)$(PS)berEncStrmXSDAny.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmXSDAny.c


$(OBJDIR)$(PS)berDecStrmBitStr$(OBJ): $(BERSRCDIR)$(PS)berDecStrmBitStr.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmBitStr.c


$(OBJDIR)$(PS)berDecStrmBigInt$(OBJ): $(BERSRCDIR)$(PS)berDecStrmBigInt.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmBigInt.c


$(OBJDIR)$(PS)berDecStrmBMPStr$(OBJ): $(BERSRCDIR)$(PS)berDecStrmBMPStr.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmBMPStr.c


$(OBJDIR)$(PS)berDecStrmCharStr$(OBJ): $(BERSRCDIR)$(PS)berDecStrmCharStr.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmCharStr.c


$(OBJDIR)$(PS)berDecStrmDateStr$(OBJ): $(BERSRCDIR)$(PS)berDecStrmDateStr.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmDateStr.c


$(OBJDIR)$(PS)berDecStrmDateTimeStr$(OBJ): $(BERSRCDIR)$(PS)berDecStrmDateTimeStr.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmDateTimeStr.c


$(OBJDIR)$(PS)berDecStrmDurationStr$(OBJ): $(BERSRCDIR)$(PS)berDecStrmDurationStr.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmDurationStr.c


$(OBJDIR)$(PS)berDecStrmTimeStr$(OBJ): $(BERSRCDIR)$(PS)berDecStrmTimeStr.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmTimeStr.c


$(OBJDIR)$(PS)berDecStrmTimeOfDayStr$(OBJ): $(BERSRCDIR)$(PS)berDecStrmTimeOfDayStr.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmTimeOfDayStr.c


$(OBJDIR)$(PS)berDecStrmCommon$(OBJ): $(BERSRCDIR)$(PS)berDecStrmCommon.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmCommon.c


$(OBJDIR)$(PS)berDecStrmDynBitStr$(OBJ): $(BERSRCDIR)$(PS)berDecStrmDynBitStr.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmDynBitStr.c


$(OBJDIR)$(PS)berDecStrmEnum$(OBJ): $(BERSRCDIR)$(PS)berDecStrmEnum.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmEnum.c


$(OBJDIR)$(PS)berDecStrmInt8$(OBJ): $(BERSRCDIR)$(PS)berDecStrmInt8.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmInt8.c


$(OBJDIR)$(PS)berDecStrmInt16$(OBJ): $(BERSRCDIR)$(PS)berDecStrmInt16.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmInt16.c


$(OBJDIR)$(PS)berDecStrmInt64$(OBJ): $(BERSRCDIR)$(PS)berDecStrmInt64.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmInt64.c


$(OBJDIR)$(PS)berDecStrmNextElement$(OBJ): $(BERSRCDIR)$(PS)berDecStrmNextElement.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmNextElement.c


$(OBJDIR)$(PS)berDecStrmNull$(OBJ): $(BERSRCDIR)$(PS)berDecStrmNull.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmNull.c


$(OBJDIR)$(PS)berDecStrmObjId64$(OBJ): $(BERSRCDIR)$(PS)berDecStrmObjId64.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmObjId64.c


$(OBJDIR)$(PS)berDecStrmOIDSubIds$(OBJ): $(BERSRCDIR)$(PS)berDecStrmOIDSubIds.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmOIDSubIds.c


$(OBJDIR)$(PS)berDecStrmOpenType$(OBJ): $(BERSRCDIR)$(PS)berDecStrmOpenType.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmOpenType.c


$(OBJDIR)$(PS)berDecStrmOpenTypeAppend$(OBJ): $(BERSRCDIR)$(PS)berDecStrmOpenTypeAppend.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmOpenTypeAppend.c


$(OBJDIR)$(PS)berDecStrmOpenTypeExt$(OBJ): $(BERSRCDIR)$(PS)berDecStrmOpenTypeExt.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmOpenTypeExt.c


$(OBJDIR)$(PS)berDecStrmPeekTagAndLen$(OBJ): $(BERSRCDIR)$(PS)berDecStrmPeekTagAndLen.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmPeekTagAndLen.c


$(OBJDIR)$(PS)berDecStrmReadTLV$(OBJ): $(BERSRCDIR)$(PS)berDecStrmReadTLV.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmReadTLV.c


$(OBJDIR)$(PS)berDecStrmReal$(OBJ): $(BERSRCDIR)$(PS)berDecStrmReal.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmReal.c


$(OBJDIR)$(PS)berDecStrmRelativeOID$(OBJ): $(BERSRCDIR)$(PS)berDecStrmRelativeOID.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmRelativeOID.c


$(OBJDIR)$(PS)berDecStrmUInt8$(OBJ): $(BERSRCDIR)$(PS)berDecStrmUInt8.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmUInt8.c


$(OBJDIR)$(PS)berDecStrmUInt64$(OBJ): $(BERSRCDIR)$(PS)berDecStrmUInt64.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmUInt64.c


$(OBJDIR)$(PS)berDecStrmUnivStr$(OBJ): $(BERSRCDIR)$(PS)berDecStrmUnivStr.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmUnivStr.c


$(OBJDIR)$(PS)berDecStrmBool$(OBJ): $(BERSRCDIR)$(PS)berDecStrmBool.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmBool.c


$(OBJDIR)$(PS)berSocket$(OBJ): $(BERSRCDIR)$(PS)berSocket.c $(BERSRCDIR)$(PS)asn1berSocket.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxSocket.h $(RTSRCDIR)$(PS)asn1intl.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berSocket.c


$(OBJDIR)$(PS)berDecStrmDecimal$(OBJ): $(BERSRCDIR)$(PS)berDecStrmDecimal.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxDecimal.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmDecimal.c


$(OBJDIR)$(PS)berDecStrmReal10$(OBJ): $(BERSRCDIR)$(PS)berDecStrmReal10.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmReal10.c


$(OBJDIR)$(PS)berEncStrmReal10$(OBJ): $(BERSRCDIR)$(PS)berEncStrmReal10.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmReal10.c


$(OBJDIR)$(PS)cerEncStrmBMPStr$(OBJ): $(BERSRCDIR)$(PS)cerEncStrmBMPStr.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)cerEncStrmBMPStr.c


$(OBJDIR)$(PS)cerEncStrmBitStr$(OBJ): $(BERSRCDIR)$(PS)cerEncStrmBitStr.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)cerEncStrmBitStr.c


$(OBJDIR)$(PS)cerEncStrmCharStr$(OBJ): $(BERSRCDIR)$(PS)cerEncStrmCharStr.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)cerEncStrmCharStr.c


$(OBJDIR)$(PS)cerEncStrmOctStr$(OBJ): $(BERSRCDIR)$(PS)cerEncStrmOctStr.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)cerEncStrmOctStr.c


$(OBJDIR)$(PS)cerEncStrmUnivStr$(OBJ): $(BERSRCDIR)$(PS)cerEncStrmUnivStr.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)cerEncStrmUnivStr.c


$(OBJDIR)$(PS)cerEncStrmReal10$(OBJ): $(BERSRCDIR)$(PS)cerEncStrmReal10.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)cerEncStrmReal10.c


$(OBJDIR)$(PS)derEncBitString$(OBJ): $(BERSRCDIR)$(PS)derEncBitString.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)derEncBitString.c


$(LCOBJDIR)$(PS)berDecCharArray$(OBJ): $(BERSRCDIR)$(PS)berDecCharArray.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecCharArray.c


$(LCOBJDIR)$(PS)berDecStrmDynObjId$(OBJ): $(BERSRCDIR)$(PS)berDecStrmDynObjId.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmDynObjId.c


$(LCOBJDIR)$(PS)berDecStrmDynOctStr$(OBJ): $(BERSRCDIR)$(PS)berDecStrmDynOctStr.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmDynOctStr.c


$(LCOBJDIR)$(PS)berDecStrmInt$(OBJ): $(BERSRCDIR)$(PS)berDecStrmInt.c $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmInt.c


$(LCOBJDIR)$(PS)berDecStrmUInt$(OBJ): $(BERSRCDIR)$(PS)berDecStrmUInt.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmUInt.c


$(LCOBJDIR)$(PS)berDecStrmObjId$(OBJ): $(BERSRCDIR)$(PS)berDecStrmObjId.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmObjId.c


$(LCOBJDIR)$(PS)berDecStrmOctStr$(OBJ): $(BERSRCDIR)$(PS)berDecStrmOctStr.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmOctStr.c


$(LCOBJDIR)$(PS)berDecStrmUInt16$(OBJ): $(BERSRCDIR)$(PS)berDecStrmUInt16.c \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berDecStrmUInt16.c


$(LCOBJDIR)$(PS)berEncStrmInt$(OBJ): $(BERSRCDIR)$(PS)berEncStrmInt.c $(BERSRCDIR)$(PS)xse_common.hh \
 $(BERSRCDIR)$(PS)asn1berStream.h $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h \
 $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h \
 $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h \
 $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h \
 $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmInt.c


$(LCOBJDIR)$(PS)berEncStrmObjId$(OBJ): $(BERSRCDIR)$(PS)berEncStrmObjId.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh $(RTXSRCDIR)$(PS)rtxUtil.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmObjId.c


$(LCOBJDIR)$(PS)berEncStrmOctStr$(OBJ): $(BERSRCDIR)$(PS)berEncStrmOctStr.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmOctStr.c


$(LCOBJDIR)$(PS)berEncStrmUInt16$(OBJ): $(BERSRCDIR)$(PS)berEncStrmUInt16.c \
 $(BERSRCDIR)$(PS)xse_common.hh $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxStream.h $(RTXSRCDIR)$(PS)rtxMemBuf.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)berEncStrmUInt16.c


$(LCOBJDIR)$(PS)xd_bigint$(OBJ): $(BERSRCDIR)$(PS)xd_bigint.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_bigint.c


$(LCOBJDIR)$(PS)xd_bitstr$(OBJ): $(BERSRCDIR)$(PS)xd_bitstr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_bitstr.c


$(LCOBJDIR)$(PS)xd_bitstr_s$(OBJ): $(BERSRCDIR)$(PS)xd_bitstr_s.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_bitstr_s.c


$(LCOBJDIR)$(PS)xd_boolean$(OBJ): $(BERSRCDIR)$(PS)xd_boolean.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_boolean.c


$(LCOBJDIR)$(PS)xd_charstr$(OBJ): $(BERSRCDIR)$(PS)xd_charstr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxCharStr.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_charstr.c


$(LCOBJDIR)$(PS)xd_datestr$(OBJ): $(BERSRCDIR)$(PS)xd_datestr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_datestr.c


$(LCOBJDIR)$(PS)xd_dynObjId$(OBJ): $(BERSRCDIR)$(PS)xd_dynObjId.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_dynObjId.c


$(LCOBJDIR)$(PS)xd_timestr$(OBJ): $(BERSRCDIR)$(PS)xd_timestr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_timestr.c


$(LCOBJDIR)$(PS)xd_datetimestr$(OBJ): $(BERSRCDIR)$(PS)xd_datetimestr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_datetimestr.c


$(LCOBJDIR)$(PS)xd_timeofdaystr$(OBJ): $(BERSRCDIR)$(PS)xd_timeofdaystr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_timeofdaystr.c


$(LCOBJDIR)$(PS)xd_durationstr$(OBJ): $(BERSRCDIR)$(PS)xd_durationstr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_durationstr.c


$(LCOBJDIR)$(PS)xd_integer$(OBJ): $(BERSRCDIR)$(PS)xd_integer.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_integer.c


$(LCOBJDIR)$(PS)xd_int8$(OBJ): $(BERSRCDIR)$(PS)xd_int8.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_int8.c


$(LCOBJDIR)$(PS)xd_objid$(OBJ): $(BERSRCDIR)$(PS)xd_objid.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_objid.c


$(LCOBJDIR)$(PS)xd_octstr$(OBJ): $(BERSRCDIR)$(PS)xd_octstr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_octstr.c


$(LCOBJDIR)$(PS)xd_OpenType$(OBJ): $(BERSRCDIR)$(PS)xd_OpenType.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_OpenType.c


$(LCOBJDIR)$(PS)xd_uint8$(OBJ): $(BERSRCDIR)$(PS)xd_uint8.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_uint8.c


$(LCOBJDIR)$(PS)xd_uint16$(OBJ): $(BERSRCDIR)$(PS)xd_uint16.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_uint16.c


$(LCOBJDIR)$(PS)xd_unsigned$(OBJ): $(BERSRCDIR)$(PS)xd_unsigned.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xd_unsigned.c


$(LCOBJDIR)$(PS)xe_bigint$(OBJ): $(BERSRCDIR)$(PS)xe_bigint.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxBigInt.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_bigint.c


$(LCOBJDIR)$(PS)xe_bitstr$(OBJ): $(BERSRCDIR)$(PS)xe_bitstr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_bitstr.c


$(LCOBJDIR)$(PS)xe_boolean$(OBJ): $(BERSRCDIR)$(PS)xe_boolean.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_boolean.c


$(LCOBJDIR)$(PS)xe_derCanonicalSort$(OBJ): $(BERSRCDIR)$(PS)xe_derCanonicalSort.c \
 $(BERSRCDIR)$(PS)asn1ber.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxSList.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h \
 $(RTXSRCDIR)$(PS)rtxStack.h $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h \
 $(RTSRCDIR)$(PS)asn1ErrCodes.h $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h \
 $(RTSRCDIR)$(PS)rtContext.h $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h \
 $(RTXSRCDIR)$(PS)rtxErrCodes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h \
 $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_derCanonicalSort.c


$(LCOBJDIR)$(PS)xe_integer$(OBJ): $(BERSRCDIR)$(PS)xe_integer.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_integer.c


$(LCOBJDIR)$(PS)xe_int8$(OBJ): $(BERSRCDIR)$(PS)xe_int8.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_int8.c


$(LCOBJDIR)$(PS)xe_objid$(OBJ): $(BERSRCDIR)$(PS)xe_objid.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_objid.c


$(LCOBJDIR)$(PS)xe_octstr$(OBJ): $(BERSRCDIR)$(PS)xe_octstr.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_octstr.c


$(LCOBJDIR)$(PS)xe_uint8$(OBJ): $(BERSRCDIR)$(PS)xe_uint8.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_uint8.c


$(LCOBJDIR)$(PS)xe_uint16$(OBJ): $(BERSRCDIR)$(PS)xe_uint16.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_uint16.c


$(LCOBJDIR)$(PS)xe_unsigned$(OBJ): $(BERSRCDIR)$(PS)xe_unsigned.c $(BERSRCDIR)$(PS)asn1ber.h \
 $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxExternDefs.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxContext.hh
	$(CC) $(BERCFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)xe_unsigned.c


$(OBJDIR)$(PS)asn1BerCppTypes$(OBJ): $(BERSRCDIR)$(PS)asn1BerCppTypes.cpp \
 $(BERSRCDIR)$(PS)asn1BerCppTypes.h $(RTSRCDIR)$(PS)asn1CppTypes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.h \
 $(RTSRCDIR)$(PS)asn1CppEvtHndlr.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTSRCDIR)$(PS)asn1CppRawEvtHndlr.h \
 $(RTSRCDIR)$(PS)asn1CRawEvtHndlr.h $(RTSRCDIR)$(PS)ASN1Context.h \
 $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)OSRTMsgBuf.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h $(RTSRCDIR)$(PS)ASN1TOctStr.h $(RTSRCDIR)$(PS)ASN1TOctStr64.h \
 $(RTSRCDIR)$(PS)ASN1TObjId.h $(RTSRCDIR)$(PS)rt3GPPTS32297.h $(BERSRCDIR)$(PS)asn1ber.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxPrint.h \
 $(RTXSRCDIR)$(PS)rtxHexDump.h $(RTSRCDIR)$(PS)asn1intl.h $(RTXSRCDIR)$(PS)rtxContext.hh \
 $(RTXSRCDIR)$(PS)rtxFile.h
	$(CCC) $(BERCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)asn1BerCppTypes.cpp


$(OBJDIR)$(PS)ASN1BEREncodeStream$(OBJ): $(BERSRCDIR)$(PS)ASN1BEREncodeStream.cpp \
 $(BERSRCDIR)$(PS)ASN1BEREncodeStream.h $(RTSRCDIR)$(PS)asn1CppTypes.h \
 $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxContext.h $(RTXSRCDIR)$(PS)rtxDList.h \
 $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.h \
 $(RTSRCDIR)$(PS)asn1CppEvtHndlr.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTSRCDIR)$(PS)asn1CppRawEvtHndlr.h \
 $(RTSRCDIR)$(PS)asn1CRawEvtHndlr.h $(RTSRCDIR)$(PS)ASN1Context.h \
 $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)OSRTMsgBuf.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h $(RTSRCDIR)$(PS)ASN1TOctStr.h $(RTSRCDIR)$(PS)ASN1TOctStr64.h \
 $(RTSRCDIR)$(PS)ASN1TObjId.h $(RTXSRCDIR)$(PS)OSRTOutputStreamIF.h \
 $(RTXSRCDIR)$(PS)OSRTStreamIF.h $(BERSRCDIR)$(PS)asn1berStream.h \
 $(BERSRCDIR)$(PS)asn1ber.h $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h \
 $(RTXSRCDIR)$(PS)rtxStream.h
	$(CCC) $(BERCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)ASN1BEREncodeStream.cpp


$(OBJDIR)$(PS)ASN1BERDecodeStream$(OBJ): $(BERSRCDIR)$(PS)ASN1BERDecodeStream.cpp \
 $(BERSRCDIR)$(PS)ASN1BERDecodeStream.h $(BERSRCDIR)$(PS)asn1BerCppTypes.h \
 $(RTSRCDIR)$(PS)asn1CppTypes.h $(RTXSRCDIR)$(PS)rtxMemory.h $(RTXSRCDIR)$(PS)rtxContext.h \
 $(RTXSRCDIR)$(PS)rtxDList.h $(RTXSRCDIR)$(PS)osSysTypes.h $(RTXSRCDIR)$(PS)rtxExternDefs.h \
 $(RTXSRCDIR)$(PS)rtxCommonDefs.h $(RTXSRCDIR)$(PS)osMacros.h $(RTXSRCDIR)$(PS)rtxStack.h \
 $(RTXSRCDIR)$(PS)rtxDiag.h $(RTXSRCDIR)$(PS)rtxError.h $(RTXSRCDIR)$(PS)rtxErrCodes.h \
 $(RTXSRCDIR)$(PS)rtxMemBuf.h $(RTSRCDIR)$(PS)asn1CppEvtHndlr64.h \
 $(RTSRCDIR)$(PS)asn1CppEvtHndlr.h $(RTSRCDIR)$(PS)asn1type.h $(RTXSRCDIR)$(PS)rtxSList.h \
 $(RTXSRCDIR)$(PS)rtxUTF8.h $(RTSRCDIR)$(PS)asn1tag.h $(RTSRCDIR)$(PS)asn1ErrCodes.h \
 $(RTSRCDIR)$(PS)rtExternDefs.h $(RTXSRCDIR)$(PS)rtxBitString.h $(RTSRCDIR)$(PS)rtContext.h \
 $(RTSRCDIR)$(PS)asn1CEvtHndlr.h $(RTSRCDIR)$(PS)asn1CppRawEvtHndlr.h \
 $(RTSRCDIR)$(PS)asn1CRawEvtHndlr.h $(RTSRCDIR)$(PS)ASN1Context.h \
 $(RTXSRCDIR)$(PS)OSRTContext.h $(RTXSRCDIR)$(PS)OSRTMsgBuf.h \
 $(RTXSRCDIR)$(PS)OSRTCtxtHolder.h $(RTXSRCDIR)$(PS)OSRTCtxtHolderIF.h \
 $(RTXSRCDIR)$(PS)OSRTMsgBufIF.h $(RTSRCDIR)$(PS)ASN1TOctStr.h $(RTSRCDIR)$(PS)ASN1TOctStr64.h \
 $(RTSRCDIR)$(PS)ASN1TObjId.h $(RTSRCDIR)$(PS)rt3GPPTS32297.h $(BERSRCDIR)$(PS)asn1ber.h \
 $(BERSRCDIR)$(PS)berMacros.h $(RTXSRCDIR)$(PS)rtxBuffer.h $(RTXSRCDIR)$(PS)rtxPrint.h \
 $(RTXSRCDIR)$(PS)rtxHexDump.h $(BERSRCDIR)$(PS)asn1berStream.h $(RTXSRCDIR)$(PS)rtxStream.h \
 $(RTXSRCDIR)$(PS)OSRTInputStreamIF.h $(RTXSRCDIR)$(PS)OSRTStreamIF.h
	$(CCC) $(BERCPPFLAGS) -c $(IPATHS) $(OBJOUT) $(BERSRCDIR)$(PS)ASN1BERDecodeStream.cpp

