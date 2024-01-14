# Build shared run-time libraries

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

COMFLAGS = $(CVARSMDD_) $(CELF_) $(MCFLAGS) $(CDEV_) $(MCOMFLAGS)  -D_NO_LICENSE_CHECK -DCPP
CFLAGS   = $(COMFLAGS) $(CFLAGS_)
CCFLAGS  = $(COMFLAGS) $(CCFLAGS_)
IPATHS   = \
-I$(RTSRCDIR) -I$(BERSRCDIR) -I$(PERSRCDIR) -I$(XERSRCDIR) \
-I$(OSROOTDIR) $(IPATHS_)
EXPATIPATHS = -I$(OSROOTDIR) -I$(EXPATSRCDIR) $(IPATHS_)
RTLIBPATH = $(LPPFX)$(DESTDIR)
XMLIFOBJ = $(OBJ)

RTXCFLAGS = $(CFLAGS)
RTCFLAGS = $(CFLAGS)
RTXCCFLAGS = $(CFLAGS)
RTXCPPFLAGS = $(CFLAGS)
BERCFLAGS = $(CFLAGS)
JSONCFLAGS = $(CFLAGS)
OERCFLAGS = $(CFLAGS) -I$(OERSRCDIR)
PERCFLAGS = $(CFLAGS)
XERCFLAGS = $(CFLAGS) $(XMLDDEFS) $(XMLINC)
XMLCFLAGS = $(CFLAGS) $(XMLDDEFS) -I$(XMLSRCDIR) -I$(LIBXML2INC)
XMLCCFLAGS = $(CCFLAGS) $(XMLDDEFS) -I$(XMLSRCDIR) -I$(LIBXML2INC)
EXPATDEFS =  -DHAVE_EXPAT_CONFIG_H -DXML_NS
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
$(DESTDIR)$(PS)$(LIBPFX)expat$(SO) \
$(DESTDIR)$(PS)libxml2$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1rt$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1ber$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1json$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1oer$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1per$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1xer$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1xml$(SO) \
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

$(DESTDIR)$(PS)$(LIBPFX)asn1rt$(SO) : $(ASN1RT_OBJECTS) $(STUB_OBJECTS_RT)
	$(LINK) $(LINKELF_) $(ASN1RT_OBJECTS) $(STUB_OBJECTS_RT) $(RTLIBPATH) $(LLSYS)

$(DESTDIR)$(PS)$(LIBPFX)asn1ber$(SO) : $(BER_CPP_OBJECTS) $(STUB_OBJECTS_BER)
	$(LINK) $(LINKELF_) $(BER_CPP_OBJECTS) $(STUB_OBJECTS_BER) $(RTLIBPATH) $(LLSYS) $(LLRT)

$(DESTDIR)$(PS)$(LIBPFX)asn1json$(SO) : $(OSRT_JSON_RTOBJECTS) $(OSRT_JSON_CPPRTOBJECTS) $(ASN1_JSON_RTOBJECTS) 
	$(LINK) $(LINKELF_) $(OSRT_JSON_RTOBJECTS) $(OSRT_JSON_CPPRTOBJECTS) $(ASN1_JSON_RTOBJECTS)  $(RTLIBPATH) $(LLSYS) $(LLRT) $(LLXML)

$(DESTDIR)$(PS)$(LIBPFX)asn1oer$(SO) : $(OER_CPP_OBJECTS) 
	$(LINK) $(LINKELF_) $(OER_CPP_OBJECTS) $(RTLIBPATH) $(LLSYS) $(LLRT)

$(DESTDIR)$(PS)$(LIBPFX)asn1per$(SO) : $(PER_CPP_OBJECTS) $(STUB_OBJECTS_PER)
	$(LINK) $(LINKELF_) $(PER_CPP_OBJECTS) $(STUB_OBJECTS_PER) $(RTLIBPATH) $(LLSYS) $(LLRT)

$(DESTDIR)$(PS)$(LIBPFX)asn1xer$(SO) : \
$(ASN1XER_OBJECTS) $(STUB_OBJECTS_XER)
	$(LINK) $(LINKELF_) $(ASN1XER_OBJECTS) $(STUB_OBJECTS_XER) $(RTLIBPATH) $(LLSYS) $(LLRT)

$(DESTDIR)$(PS)$(LIBPFX)asn1xml$(SO) : \
$(OSXML_CPP_RTOBJECTS) $(ASN1XML_CPP_RTOBJECTS)
	$(LINK) $(LINKELF_) $(OSXML_CPP_RTOBJECTS) $(ASN1XML_CPP_RTOBJECTS) $(RTLIBPATH) $(LLSYS) $(LLRT)

$(DESTDIR)$(PS)$(LIBPFX)expat$(SO) : $(EXPAT_COMMON_RTOBJECTS)
	$(LINK) $(LINKELF_) $(EXPAT_COMMON_RTOBJECTS) $(RTLIBPATH) $(LLSYS)

$(DESTDIR)$(PS)libxml2$(SO) : $(LIBXML2OBJ)
	$(LINK) $(LINKELF_) $(LIBXML2OBJ) $(RTLIBPATH) $(LLSYS)


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
CFLAGSIF = -I. -I$(OSROOTDIR) $(IPATHS_) $(CFLAGS)  -DLIBXML_STATIC $(OBJOUT)

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
	$(RM) $(DESTDIR)$(PS)*$(SO) 
	$(RM) $(DESTDIR)$(PS)rtXml*IF$(XMLIFOBJ)
	$(RM) *.pdb 
	$(RM) *~ 
