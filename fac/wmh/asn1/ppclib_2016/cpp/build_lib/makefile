# Build static run-time libraries

include ../platform.mk

OSROOTDIR = ..$(PS)..
BERSRCDIR = $(OSROOTDIR)$(PS)rtbersrc
EXPATSRCDIR = $(OSROOTDIR)$(PS)expatsrc
JSONSRCDIR = $(OSROOTDIR)$(PS)rtjsonsrc
LIBXML2SRCDIR = $(OSROOTDIR)$(PS)libxml2src
MDERSRCDIR = $(OSROOTDIR)$(PS)rtmdersrc
OERSRCDIR = $(OSROOTDIR)$(PS)rtoersrc
PERSRCDIR = $(OSROOTDIR)$(PS)rtpersrc
RTSRCDIR  = $(OSROOTDIR)$(PS)rtsrc
RTXSRCDIR = $(OSROOTDIR)$(PS)rtxsrc
XERSRCDIR = $(OSROOTDIR)$(PS)rtxersrc
XMLSRCDIR = $(OSROOTDIR)$(PS)rtxmlsrc

OBJDIR    = .
LCOBJDIR  = .
DESTDIR   = ..$(PS)lib

COMFLAGS = $(CVARSMTD_) $(MCFLAGS) $(CDEV_) $(MCOMFLAGS)  -D_NO_LICENSE_CHECK -DCPP
CFLAGS   = $(COMFLAGS) $(CFLAGS_)
CCFLAGS  = $(COMFLAGS) $(CCFLAGS_)
IPATHS   = \
-I$(RTSRCDIR) -I$(BERSRCDIR) -I$(PERSRCDIR) -I$(XERSRCDIR) \
-I$(OSROOTDIR) $(IPATHS_)
EXPATIPATHS = -I$(OSROOTDIR) -I$(EXPATSRCDIR) $(IPATHS_)
XMLIFOBJ = _a$(OBJ)

RTXCFLAGS = $(CFLAGS)
RTCFLAGS = $(CFLAGS)
RTXCCFLAGS = $(CFLAGS)
RTXCPPFLAGS = $(CFLAGS)
BERCFLAGS = $(CFLAGS)
JSONCFLAGS = $(CFLAGS)
OERCFLAGS = $(CFLAGS) -I$(OERSRCDIR)
PERCFLAGS = $(CFLAGS)
XERCFLAGS = $(CFLAGS) $(XMLDEFS) $(XMLINC)
XMLCFLAGS = $(CFLAGS) $(XMLDEFS) -I$(XMLSRCDIR) -I$(LIBXML2INC)
XMLCCFLAGS = $(CCFLAGS) $(XMLDEFS) -I$(XMLSRCDIR) -I$(LIBXML2INC)
EXPATDEFS = -DXML_STATIC -DHAVE_EXPAT_CONFIG_H -DXML_NS
LIBXML2DEFS = -DHAVE_LIBXML2_CONFIG_H -DXML_NS 

ASN1RT_OBJECTS = \
$(ASN1RT_C_OBJECTS) \
$(ASN1RT_CPPOBJECTS) \
$(OSRT_C_RTOBJECTS) \
$(OSRT_CPP_OBJECTS)

ASN1XER_OBJECTS = \
$(XER_CPP_OBJECTS) \
$(OSXML_CPP_RTOBJECTS) \
$(ASN1XML_RTOBJECTS)

all : outdirs \
$(DESTDIR)$(PS)$(LIBPFX)expat$(A) \
$(DESTDIR)$(PS)libxml2$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1rt$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1ber$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1json$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1oer$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1per$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1xer$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1xml$(A) \
$(DESTDIR)/rtXmlExpatIF$(XMLIFOBJ) \
$(DESTDIR)/rtXmlLibxml2IF$(XMLIFOBJ)

include ../../rtsrc/objects2.mk
include ../../rtxsrc/objects2.mk
include ../../rtbersrc/objects2.mk
include ../../rtjsonsrc/objects.mk
include ../../rtoersrc/objects.mk
include ../../rtpersrc/objects2.mk
include ../../rtxersrc/objects2.mk
include ../../rtxmlsrc/objects2.mk
include ../../rtxmlsrc/asn1Objects2.mk
include ../../expatsrc/objects.mk
include ../../libxml2src/objects.mk

$(DESTDIR)$(PS)$(LIBPFX)asn1rt$(A) : $(ASN1RT_OBJECTS) $(STUB_OBJECTS_RT)
	$(LIBCMD) $(ASN1RT_OBJECTS) $(STUB_OBJECTS_RT)

$(DESTDIR)$(PS)$(LIBPFX)asn1ber$(A) : $(BER_CPP_OBJECTS) $(STUB_OBJECTS_BER)
	$(LIBCMD) $(BER_CPP_OBJECTS) $(STUB_OBJECTS_BER)

$(DESTDIR)$(PS)$(LIBPFX)asn1json$(A) : $(OSRT_JSON_RTOBJECTS) $(OSRT_JSON_CPPRTOBJECTS) $(ASN1_JSON_RTOBJECTS) 
	$(LIBCMD) $(OSRT_JSON_RTOBJECTS) $(OSRT_JSON_CPPRTOBJECTS) $(ASN1_JSON_RTOBJECTS) 

$(DESTDIR)$(PS)$(LIBPFX)asn1oer$(A) : $(OER_CPP_OBJECTS) 
	$(LIBCMD) $(OER_CPP_OBJECTS)

$(DESTDIR)$(PS)$(LIBPFX)asn1per$(A) : $(PER_CPP_OBJECTS) $(STUB_OBJECTS_PER)
	$(LIBCMD) $(PER_CPP_OBJECTS) $(STUB_OBJECTS_PER)

$(DESTDIR)$(PS)$(LIBPFX)asn1xer$(A) : \
$(ASN1XER_OBJECTS) $(STUB_OBJECTS_XER)
	$(LIBCMD) $(ASN1XER_OBJECTS) $(STUB_OBJECTS_XER)

$(DESTDIR)$(PS)$(LIBPFX)asn1xml$(A) : \
$(OSXML_CPP_RTOBJECTS) $(ASN1XML_CPP_RTOBJECTS)
	$(LIBCMD) $(OSXML_CPP_RTOBJECTS) $(ASN1XML_CPP_RTOBJECTS)

$(DESTDIR)$(PS)$(LIBPFX)expat$(A) : $(EXPAT_COMMON_RTOBJECTS)
	$(LIBCMD) $(EXPAT_COMMON_RTOBJECTS)

$(DESTDIR)$(PS)libxml2$(A) : $(LIBXML2OBJ)
	$(LIBCMD) $(LIBXML2OBJ)


include ../../rtsrc/rules2.mk
include ../../rtxsrc/rules2.mk
include ../../rtbersrc/rules2.mk
include ../../rtjsonsrc/rules2.mk
include ../../rtoersrc/rules.mk
include ../../rtpersrc/rules2.mk
include ../../rtxersrc/rules2.mk
include ../../rtxmlsrc/rules2.mk
include ../../expatsrc/c_rules.mk
include ../../libxml2src/c_rules.mk

EXPATSRCDIR = $(OSROOTDIR)$(PS)expatsrc
CFLAGSIF = -I. -I$(OSROOTDIR) $(IPATHS_) $(CFLAGS) -DXML_STATIC -DLIBXML_STATIC $(OBJOUT)

include ../../rtxmlsrc/cppif_rules.mk

outdirs :
	perl -e "mkdir ('$(DESTDIR)', 0777) if (! -e '$(DESTDIR)')"
	perl -e "mkdir ('./lcobj', 0777) if (! -e './lcobj')"

clean :
	$(RM) *.c
	$(RM) $(OBJDIR)$(PS)*$(OBJ) 
	$(RM) $(LCOBJDIR)$(PS)*$(OBJ) 
	$(RM) $(OBJDIR)$(PS)*.pdb 
	$(RM) $(LCOBJDIR)$(PS)*.pdb 
	$(RM) *~ 

realclean :
	$(MAKE) clean
	$(RM) $(SRCDIR)$(PS)asn1*.h 
	$(RM) $(SRCDIR)$(PS)asn1*.cpp 
	$(RM) $(DESTDIR)$(PS)*$(A) 
	$(RM) $(DESTDIR)$(PS)rtXml*IF$(XMLIFOBJ)
	$(RM) *.pdb 
	$(RM) *~ 
