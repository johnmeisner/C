OER_C_OBJECTS_NLC = \
   $(OBJDIR)$(PS)oerDecBigInt$(OBJ) \
   $(OBJDIR)$(PS)oerDecBitStr$(OBJ) \
   $(OBJDIR)$(PS)oerDecBMPStr$(OBJ) \
   $(OBJDIR)$(PS)oerDecCharStr$(OBJ) \
   $(OBJDIR)$(PS)oerDecChoiceExt$(OBJ) \
   $(OBJDIR)$(PS)oerDecDateStr$(OBJ) \
   $(OBJDIR)$(PS)oerDecDateTimeStr$(OBJ) \
   $(OBJDIR)$(PS)oerDecDurationStr$(OBJ) \
   $(OBJDIR)$(PS)oerDecDynBitStr$(OBJ) \
   $(OBJDIR)$(PS)oerDecDynCharStr$(OBJ) \
   $(OBJDIR)$(PS)oerDecDynOctStr$(OBJ) \
   $(OBJDIR)$(PS)oerDecEnum$(OBJ) \
   $(OBJDIR)$(PS)oerDecInt$(OBJ) \
   $(OBJDIR)$(PS)oerDecInt64$(OBJ) \
   $(OBJDIR)$(PS)oerDecLen$(OBJ) \
   $(OBJDIR)$(PS)oerDecOID$(OBJ) \
   $(OBJDIR)$(PS)oerDecReal$(OBJ) \
   $(OBJDIR)$(PS)oerDecRelOID$(OBJ) \
   $(OBJDIR)$(PS)oerDecTag$(OBJ) \
   $(OBJDIR)$(PS)oerDecTimeDiffStr$(OBJ) \
   $(OBJDIR)$(PS)oerDecTimeOfDayStr$(OBJ) \
   $(OBJDIR)$(PS)oerDecUInt$(OBJ) \
   $(OBJDIR)$(PS)oerDecUInt64$(OBJ) \
   $(OBJDIR)$(PS)oerDecUnivStr$(OBJ) \
   $(OBJDIR)$(PS)oerDecUnrestInt32$(OBJ) \
   $(OBJDIR)$(PS)oerDecUnrestUInt32$(OBJ) \
   $(OBJDIR)$(PS)oerDecUnrestSize$(OBJ) \
   $(OBJDIR)$(PS)oerEncBigInt$(OBJ) \
   $(OBJDIR)$(PS)oerEncBitStr$(OBJ) \
   $(OBJDIR)$(PS)oerEncBMPStr$(OBJ) \
   $(OBJDIR)$(PS)oerEncDateStr$(OBJ) \
   $(OBJDIR)$(PS)oerEncDateTimeStr$(OBJ) \
   $(OBJDIR)$(PS)oerEncDurationStr$(OBJ) \
   $(OBJDIR)$(PS)oerEncEnum$(OBJ) \
   $(OBJDIR)$(PS)oerEncExtElem$(OBJ) \
   $(OBJDIR)$(PS)oerEncIdent$(OBJ) \
   $(OBJDIR)$(PS)oerEncInt32$(OBJ) \
   $(OBJDIR)$(PS)oerEncInt64$(OBJ) \
   $(OBJDIR)$(PS)oerEncLen$(OBJ) \
   $(OBJDIR)$(PS)oerEncOID$(OBJ) \
   $(OBJDIR)$(PS)oerEncOID64$(OBJ) \
   $(OBJDIR)$(PS)oerEncRelOID$(OBJ) \
   $(OBJDIR)$(PS)oerEncRelOID64$(OBJ) \
   $(OBJDIR)$(PS)oerEncReal$(OBJ) \
   $(OBJDIR)$(PS)oerEncTag$(OBJ) \
   $(OBJDIR)$(PS)oerEncTimeDiffStr$(OBJ) \
   $(OBJDIR)$(PS)oerEncTimeOfDayStr$(OBJ) \
   $(OBJDIR)$(PS)oerEncUInt32$(OBJ) \
   $(OBJDIR)$(PS)oerEncUInt64$(OBJ) \
   $(OBJDIR)$(PS)oerEncUnivStr$(OBJ) \
   $(OBJDIR)$(PS)oerEncUnrestInt32$(OBJ) \
   $(OBJDIR)$(PS)oerEncUnrestUInt32$(OBJ) \
   $(OBJDIR)$(PS)oerEncUnrestSize$(OBJ)

OER_C_OBJECTS = \
   $(OER_C_OBJECTS_NLC)

OER_CPP_OBJECTS = \
   $(OBJDIR)$(PS)asn1OerCppTypes$(OBJ) \
   $(OER_C_OBJECTS)
