ASN1RT_C_OBJECTS_NLC = \
   $(OBJDIR)$(PS)asn1CEvtHndlr$(OBJ) \
   $(OBJDIR)$(PS)asn1CRawEvtHndlr$(OBJ) \
   $(OBJDIR)$(PS)cerEncCanonicalSort$(OBJ) \
   $(OBJDIR)$(PS)rt16BitChars$(OBJ) \
   $(OBJDIR)$(PS)rt32BitChars$(OBJ) \
   $(OBJDIR)$(PS)rtAvnLookupIdentifier$(OBJ) \
   $(OBJDIR)$(PS)rtAvnMatchToken$(OBJ) \
   $(OBJDIR)$(PS)rtAvnReadBoolean$(OBJ) \
   $(OBJDIR)$(PS)rtAvnReadBin$(OBJ) \
   $(OBJDIR)$(PS)rtAvnReadIdentifier$(OBJ) \
   $(OBJDIR)$(PS)rtAvnReadOID$(OBJ) \
   $(OBJDIR)$(PS)rtAvnReadReal$(OBJ) \
   $(OBJDIR)$(PS)rtAvnReadStr$(OBJ) \
   $(OBJDIR)$(PS)rtAvnReadUCS2$(OBJ) \
   $(OBJDIR)$(PS)rtAvnReadUCS4$(OBJ) \
   $(OBJDIR)$(PS)rtAvnReadValue$(OBJ) \
   $(OBJDIR)$(PS)rtAvnWriteBitStr$(OBJ) \
   $(OBJDIR)$(PS)rtAvnWriteBMPStr$(OBJ) \
   $(OBJDIR)$(PS)rtAvnWriteCharStr$(OBJ) \
   $(OBJDIR)$(PS)rtAvnWriteReal$(OBJ) \
   $(OBJDIR)$(PS)rtAvnWriteOctStr$(OBJ) \
   $(OBJDIR)$(PS)rtAvnWriteOID$(OBJ) \
   $(OBJDIR)$(PS)rtAvnWriteUnivStr$(OBJ) \
   $(OBJDIR)$(PS)rtAvnWriteUTF8Str$(OBJ) \
   $(OBJDIR)$(PS)rtBCD$(OBJ) \
   $(OBJDIR)$(PS)rtPLMNidentity$(OBJ) \
   $(OBJDIR)$(PS)rtcompare$(OBJ) \
   $(OBJDIR)$(PS)rtconv$(OBJ) \
   $(OBJDIR)$(PS)rtcopy$(OBJ) \
   $(OBJDIR)$(PS)rtError$(OBJ) \
   $(OBJDIR)$(PS)rtMakeGeneralizedTime$(OBJ) \
   $(OBJDIR)$(PS)rtMakeTime$(OBJ) \
   $(OBJDIR)$(PS)rtMakeUTCTime$(OBJ) \
   $(OBJDIR)$(PS)rtNewContext$(OBJ) \
   $(OBJDIR)$(PS)rtObjIdDecode$(OBJ) \
   $(OBJDIR)$(PS)rtOID$(OBJ) \
   $(OBJDIR)$(PS)rtParseGeneralizedTime$(OBJ) \
   $(OBJDIR)$(PS)rtParseTime$(OBJ) \
   $(OBJDIR)$(PS)rtParseUTCTime$(OBJ) \
   $(OBJDIR)$(PS)rtPrint$(OBJ) \
   $(OBJDIR)$(PS)rtPrintToStream$(OBJ) \
   $(OBJDIR)$(PS)rtPrintToString$(OBJ) \
   $(OBJDIR)$(PS)rtStream$(OBJ) \
   $(OBJDIR)$(PS)rtTable$(OBJ) \
   $(OBJDIR)$(PS)rtUTF8StrToDynBitStr$(OBJ) \
   $(OBJDIR)$(PS)rtValidateStr$(OBJ) \
   $(OBJDIR)$(PS)rt3GPPTS32297$(OBJ) \
   $(OBJDIR)$(PS)rt3GPPTS32297PrtToStrm$(OBJ)

ASN1RT_C_OBJECTS_LC = \
   $(LCOBJDIR)$(PS)rtContext$(OBJ) \
   $(LCOBJDIR)$(PS)rtGetLibInfo$(OBJ)

ASN1RT_C_OBJECTS = \
$(ASN1RT_C_OBJECTS_NLC) \
$(ASN1RT_C_OBJECTS_LC)

ASN1RT_CPPOBJECTS = \
   $(OBJDIR)$(PS)asn1AvnCppTypes$(OBJ) \
   $(OBJDIR)$(PS)asn1CppTypes$(OBJ) \
   $(OBJDIR)$(PS)asn1CppEvtHndlr$(OBJ) \
   $(OBJDIR)$(PS)asn1CppEvtHndlr64$(OBJ) \
   $(OBJDIR)$(PS)asn1CppRawEvtHndlr$(OBJ) \
   $(OBJDIR)$(PS)ASN1CBitStr$(OBJ) \
   $(OBJDIR)$(PS)ASN1CSeqOfList$(OBJ) \
   $(OBJDIR)$(PS)ASN1CTime$(OBJ) \
   $(OBJDIR)$(PS)ASN1CGeneralizedTime$(OBJ) \
   $(OBJDIR)$(PS)ASN1CUTCTime$(OBJ) \
   $(OBJDIR)$(PS)ASN1TDynObjId$(OBJ) \
   $(OBJDIR)$(PS)ASN1TObjId$(OBJ) \
   $(OBJDIR)$(PS)ASN1TObjId64$(OBJ) \
   $(OBJDIR)$(PS)ASN1TOctStr$(OBJ) \
   $(OBJDIR)$(PS)ASN1TOctStr64$(OBJ) \
   $(OBJDIR)$(PS)ASN1TTime$(OBJ) \
   $(OBJDIR)$(PS)ASN1TGeneralizedTime$(OBJ) \
   $(OBJDIR)$(PS)ASN1TUTCTime$(OBJ)

# START OSBUILD
LIC_OBJECTS1 = \
$(LCOBJDIR)$(PS)obj$(OBJ)

LIC_OBJECTS_RT = \
$(LCOBJDIR)$(PS)obj$(OBJ) \
$(LCOBJDIR)$(PS)rtx$(OBJ) \
$(LCOBJDIR)$(PS)lmdebug$(OBJ)

# LIC_OBJECTS_RLM_RT should be the same as LIC_OBJECTS_RT except that includes
# RLM support by using obj_rlm object file rather than obj object file.
LIC_OBJECTS_RLM_RT = \
$(LCOBJDIR)$(PS)obj_rlm$(OBJ) \
$(LCOBJDIR)$(PS)rtx$(OBJ) \
$(LCOBJDIR)$(PS)lmdebug$(OBJ)

LIC_OBJECTS2 = \
$(LIC_OBJECTS1) \
lic$(PS)xd_bitstr_s$(OBJ) \
lic$(PS)xd_charstr$(OBJ) \
lic$(PS)xd_chkend$(OBJ) \
lic$(PS)xd_consstr$(OBJ) \
lic$(PS)xd_count$(OBJ) \
lic$(PS)xd_common$(OBJ) \
lic$(PS)xd_integer$(OBJ) \
lic$(PS)xd_octstr_s$(OBJ) \
lic$(PS)xd_setp$(OBJ) \
lic$(PS)xd_memcpy$(OBJ) \
lic$(PS)xu_malloc$(OBJ) \
lic$(PS)xd_NextElement$(OBJ) \
lic$(PS)xd_len$(OBJ) \
lic$(PS)xd_MovePastEOC$(OBJ) \
lic$(PS)xu_BufferState$(OBJ) 
# END OSBUILD

STUB_OBJECTS_RT = \
$(OBJDIR)$(PS)rtevalstub$(OBJ)
