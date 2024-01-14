# makefile to build generated files

include ../../platform.mk
include ../../../sample.mk

SPECSDIR = .

# include directories
OSROOTDIR = ..$(PS)..$(PS)..
LIBDIR  = $(OSROOTDIR)$(PS)c$(RTDIRSFX)$(PS)lib_compact
RTSRCDIR = $(OSROOTDIR)$(PS)rtsrc
PERSRCDIR = $(OSROOTDIR)$(PS)rtpersrc
XMLSRCDIR = $(OSROOTDIR)$(PS)rtxmlsrc
SRCDIR = .
HFILESDIR = .
# compiler defs
COMFLAGS = $(CVARSMTR_) $(MCFLAGS) $(CSPACEOPT_) -D_MEMCOMPACT -D_OS_NOPATTERN -D_COMPACT -D_NO_STREAM
CFLAGS	 = $(COMFLAGS) $(CFLAGS_)
IPATHS = -I$(SRCDIR) -I$(OSROOTDIR) -I$(RTSRCDIR) -I$(PERSRCDIR) $(IPATHS_)
LINKOPT = $(LINKOPT_)

# run-time libraries
LIBS = $(LIBDIR)/$(LIBPFX)asn1per$(A) $(LIBDIR)/$(LIBPFX)asn1rt$(A)
LLIBS = $(LLPFX)asn1per$(LLAEXT) $(LLPFX)asn1rt$(LLAEXT) $(LLSYS)
LPATHS = $(LPPFX)$(LIBDIR)

OBJECTS =  \
EUTRA-InterNodeDefinitions$(OBJ) \
EUTRA-RRC-Definitions$(OBJ) \
EUTRA-Sidelink-Preconf$(OBJ) \
EUTRA-UE-Variables$(OBJ) \
NBIOT-InterNodeDefinitions$(OBJ) \
NBIOT-RRC-Definitions$(OBJ) \
NBIOT-UE-Variables$(OBJ) \
PC5-RRC-Definitions$(OBJ)

ENCOBJECTS =  \
EUTRA-InterNodeDefinitionsEnc$(OBJ) \
EUTRA-RRC-DefinitionsEnc$(OBJ) \
EUTRA-Sidelink-PreconfEnc$(OBJ) \
EUTRA-UE-VariablesEnc$(OBJ) \
NBIOT-InterNodeDefinitionsEnc$(OBJ) \
NBIOT-RRC-DefinitionsEnc$(OBJ) \
NBIOT-UE-VariablesEnc$(OBJ) \
PC5-RRC-DefinitionsEnc$(OBJ)

DECOBJECTS =  \
EUTRA-InterNodeDefinitionsDec$(OBJ) \
EUTRA-RRC-DefinitionsDec$(OBJ) \
EUTRA-Sidelink-PreconfDec$(OBJ) \
EUTRA-UE-VariablesDec$(OBJ) \
NBIOT-InterNodeDefinitionsDec$(OBJ) \
NBIOT-RRC-DefinitionsDec$(OBJ) \
NBIOT-UE-VariablesDec$(OBJ) \
PC5-RRC-DefinitionsDec$(OBJ)

P2MOBJECTS =  \
EUTRA-InterNodeDefinitionsPrtToStrm$(OBJ) \
EUTRA-RRC-DefinitionsPrtToStrm$(OBJ) \
EUTRA-Sidelink-PreconfPrtToStrm$(OBJ) \
EUTRA-UE-VariablesPrtToStrm$(OBJ) \
NBIOT-InterNodeDefinitionsPrtToStrm$(OBJ) \
NBIOT-RRC-DefinitionsPrtToStrm$(OBJ) \
NBIOT-UE-VariablesPrtToStrm$(OBJ) \
PC5-RRC-DefinitionsPrtToStrm$(OBJ)

all : $(OBJECTS) $(ENCOBJECTS) $(DECOBJECTS) $(P2MOBJECTS) \
writer$(EXE) reader$(EXE)

WRITEROBJ = writer$(OBJ) $(OBJECTS) $(ENCOBJECTS) $(DECOBJECTS) $(P2MOBJECTS)

# Note: add -Xlinker -Map=writer.map on Linux to get map
writer$(EXE) : $(WRITEROBJ) $(LIBS)
	$(LINK) $(WRITEROBJ) $(LINKOPT) $(LPATHS) $(LLIBS)

READEROBJ = reader$(OBJ) $(OBJECTS) $(DECOBJECTS) $(ENCOBJECTS) $(P2MOBJECTS)

reader$(EXE) : $(READEROBJ) $(LIBS)
	$(LINK) $(READEROBJ) $(LINKOPT) $(LPATHS) $(LLIBS)

# .c -> .o dependencies
# these can be updated by executing 'make depends'
include depends.mk

# .asn -> .c .h rules
$(SRCDIR)/EUTRA-InterNodeDefinitionsEnc.c \
$(SRCDIR)/EUTRA-InterNodeDefinitionsDec.c \
$(SRCDIR)/EUTRA-InterNodeDefinitions.c \
$(SRCDIR)/EUTRA-InterNodeDefinitionsPrtToStrm.c \
$(HFILESDIR)$(PS)EUTRA-InterNodeDefinitions.h \
$(SRCDIR)/EUTRA-RRC-DefinitionsEnc.c \
$(SRCDIR)/EUTRA-RRC-DefinitionsDec.c \
$(SRCDIR)/EUTRA-RRC-Definitions.c \
$(SRCDIR)/EUTRA-RRC-DefinitionsPrtToStrm.c \
$(HFILESDIR)$(PS)EUTRA-RRC-Definitions.h \
$(SRCDIR)/EUTRA-Sidelink-PreconfEnc.c \
$(SRCDIR)/EUTRA-Sidelink-PreconfDec.c \
$(SRCDIR)/EUTRA-Sidelink-Preconf.c \
$(SRCDIR)/EUTRA-Sidelink-PreconfPrtToStrm.c \
$(HFILESDIR)$(PS)EUTRA-Sidelink-Preconf.h \
$(SRCDIR)/EUTRA-UE-VariablesEnc.c \
$(SRCDIR)/EUTRA-UE-VariablesDec.c \
$(SRCDIR)/EUTRA-UE-Variables.c \
$(SRCDIR)/EUTRA-UE-VariablesPrtToStrm.c \
$(HFILESDIR)$(PS)EUTRA-UE-Variables.h \
$(SRCDIR)/NBIOT-InterNodeDefinitionsEnc.c \
$(SRCDIR)/NBIOT-InterNodeDefinitionsDec.c \
$(SRCDIR)/NBIOT-InterNodeDefinitions.c \
$(SRCDIR)/NBIOT-InterNodeDefinitionsPrtToStrm.c \
$(HFILESDIR)$(PS)NBIOT-InterNodeDefinitions.h \
$(SRCDIR)/NBIOT-RRC-DefinitionsEnc.c \
$(SRCDIR)/NBIOT-RRC-DefinitionsDec.c \
$(SRCDIR)/NBIOT-RRC-Definitions.c \
$(SRCDIR)/NBIOT-RRC-DefinitionsPrtToStrm.c \
$(HFILESDIR)$(PS)NBIOT-RRC-Definitions.h \
$(SRCDIR)/NBIOT-UE-VariablesEnc.c \
$(SRCDIR)/NBIOT-UE-VariablesDec.c \
$(SRCDIR)/NBIOT-UE-Variables.c \
$(SRCDIR)/NBIOT-UE-VariablesPrtToStrm.c \
$(HFILESDIR)$(PS)NBIOT-UE-Variables.h \
$(SRCDIR)/PC5-RRC-DefinitionsEnc.c \
$(SRCDIR)/PC5-RRC-DefinitionsDec.c \
$(SRCDIR)/PC5-RRC-Definitions.c \
$(SRCDIR)/PC5-RRC-DefinitionsPrtToStrm.c \
$(HFILESDIR)$(PS)PC5-RRC-Definitions.h \
$(HFILESDIR)$(PS)rtkey.h \
: $(SPECSDIR)/EUTRA-InterNodeDefinitions.asn $(SPECSDIR)/EUTRA-RRC-Definitions.asn $(SPECSDIR)/EUTRA-Sidelink-Preconf.asn $(SPECSDIR)/EUTRA-UE-Variables.asn $(SPECSDIR)/NBIOT-InterNodeDefinitions.asn $(SPECSDIR)/NBIOT-RRC-Definitions.asn $(SPECSDIR)/NBIOT-UE-Variables.asn $(SPECSDIR)/PC5-RRC-Definitions.asn $(OSROOTDIR)$(PS)bin$(PS)asn1c$(EXE)
	$(ASN1C) $(SPECSDIR)/EUTRA-InterNodeDefinitions.asn $(SPECSDIR)/EUTRA-RRC-Definitions.asn $(SPECSDIR)/EUTRA-Sidelink-Preconf.asn $(SPECSDIR)/EUTRA-UE-Variables.asn $(SPECSDIR)/NBIOT-InterNodeDefinitions.asn $(SPECSDIR)/NBIOT-RRC-Definitions.asn $(SPECSDIR)/NBIOT-UE-Variables.asn $(SPECSDIR)/PC5-RRC-Definitions.asn -c -uper -I $(SPECSDIR) -pdu DL-CCCH-Message -prttostrm -compact -fqtvalue

depends: EUTRA-InterNodeDefinitions.c EUTRA-RRC-Definitions.c \
EUTRA-Sidelink-Preconf.c EUTRA-UE-Variables.c NBIOT-InterNodeDefinitions.c \
NBIOT-RRC-Definitions.c NBIOT-UE-Variables.c PC5-RRC-Definitions.c
	gcc *.c -E -MM $(IPATHS) > depends.mk
	sed -i 's/\.o/\$$(OBJ)/' depends.mk

clean:
	$(RM) EUTRA-InterNodeDefinitionsEnc.c
	$(RM) EUTRA-InterNodeDefinitionsDec.c
	$(RM) EUTRA-InterNodeDefinitions.c
	$(RM) EUTRA-InterNodeDefinitionsPrtToStrm.c
	$(RM) $(HFILESDIR)$(PS)EUTRA-InterNodeDefinitions.h 
	$(RM) EUTRA-RRC-DefinitionsEnc.c
	$(RM) EUTRA-RRC-DefinitionsDec.c
	$(RM) EUTRA-RRC-Definitions.c
	$(RM) EUTRA-RRC-DefinitionsPrtToStrm.c
	$(RM) $(HFILESDIR)$(PS)EUTRA-RRC-Definitions.h 
	$(RM) EUTRA-Sidelink-PreconfEnc.c
	$(RM) EUTRA-Sidelink-PreconfDec.c
	$(RM) EUTRA-Sidelink-Preconf.c
	$(RM) EUTRA-Sidelink-PreconfPrtToStrm.c
	$(RM) $(HFILESDIR)$(PS)EUTRA-Sidelink-Preconf.h 
	$(RM) EUTRA-UE-VariablesEnc.c
	$(RM) EUTRA-UE-VariablesDec.c
	$(RM) EUTRA-UE-Variables.c
	$(RM) EUTRA-UE-VariablesPrtToStrm.c
	$(RM) $(HFILESDIR)$(PS)EUTRA-UE-Variables.h 
	$(RM) NBIOT-InterNodeDefinitionsEnc.c
	$(RM) NBIOT-InterNodeDefinitionsDec.c
	$(RM) NBIOT-InterNodeDefinitions.c
	$(RM) NBIOT-InterNodeDefinitionsPrtToStrm.c
	$(RM) $(HFILESDIR)$(PS)NBIOT-InterNodeDefinitions.h 
	$(RM) NBIOT-RRC-DefinitionsEnc.c
	$(RM) NBIOT-RRC-DefinitionsDec.c
	$(RM) NBIOT-RRC-Definitions.c
	$(RM) NBIOT-RRC-DefinitionsPrtToStrm.c
	$(RM) $(HFILESDIR)$(PS)NBIOT-RRC-Definitions.h 
	$(RM) NBIOT-UE-VariablesEnc.c
	$(RM) NBIOT-UE-VariablesDec.c
	$(RM) NBIOT-UE-Variables.c
	$(RM) NBIOT-UE-VariablesPrtToStrm.c
	$(RM) $(HFILESDIR)$(PS)NBIOT-UE-Variables.h 
	$(RM) PC5-RRC-DefinitionsEnc.c
	$(RM) PC5-RRC-DefinitionsDec.c
	$(RM) PC5-RRC-Definitions.c
	$(RM) PC5-RRC-DefinitionsPrtToStrm.c
	$(RM) $(HFILESDIR)$(PS)PC5-RRC-Definitions.h 
	$(RM) $(HFILESDIR)$(PS)rtkey.h 
	$(RM) *$(OBJ)
	$(RM) *~
	$(RM) reader$(EXE)
	$(RM) writer$(EXE)
	$(RM) *.pdb
	$(RM) *.map
	$(RM) *.ilk
	$(RM) message.dat
