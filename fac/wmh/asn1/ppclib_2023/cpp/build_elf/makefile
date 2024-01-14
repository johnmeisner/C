# Build shared run-time libraries

include ../platform.mk

OSROOTDIR = ..$(PS)..
BERSRCDIR = $(OSROOTDIR)$(PS)rtbersrc
JSONSRCDIR = $(OSROOTDIR)$(PS)rtjsonsrc
MDERSRCDIR = $(OSROOTDIR)$(PS)rtmdersrc
OERSRCDIR = $(OSROOTDIR)$(PS)rtoersrc
PERSRCDIR = $(OSROOTDIR)$(PS)rtpersrc
RTSRCDIR  = $(OSROOTDIR)$(PS)rtsrc
RTXSRCDIR = $(OSROOTDIR)$(PS)rtxsrc
XMLSRCDIR = $(OSROOTDIR)$(PS)rtxmlsrc

OBJDIR    = .
LCOBJDIR  = .
DESTDIR   = ..$(PS)lib

COMFLAGS = $(CVARSMDD_) $(CELF_) $(MCFLAGS) $(CDEV_) $(MCOMFLAGS)  -D_NO_LICENSE_CHECK -DCPP $(CDEBUG_)
CFLAGS   = $(COMFLAGS) $(CFLAGS_)
CCFLAGS  = $(COMFLAGS) $(CCFLAGS_)
IPATHS   = \
-I$(RTSRCDIR) -I$(BERSRCDIR) -I$(PERSRCDIR) \
-I$(OSROOTDIR) $(IPATHS_)
RTLIBPATH = $(LPPFX)$(DESTDIR)
XMLIFOBJ = $(OBJ)

RTXCFLAGS = $(CFLAGS)
RTCFLAGS = $(CFLAGS)
RTCPPFLAGS = $(CCFLAGS)
RTXCCFLAGS = $(CCFLAGS)
RTXCPPFLAGS = $(CCFLAGS)
BERCFLAGS = $(CFLAGS)
BERCPPFLAGS = $(CCFLAGS)
JSONCFLAGS = $(CFLAGS)
JSONCPPFLAGS = $(CFLAGS)
OERCFLAGS = $(CFLAGS) -I$(OERSRCDIR)
PERCFLAGS = $(CFLAGS)
PERCPPFLAGS = $(CCFLAGS)
XMLCFLAGS = $(CFLAGS) $(XMLDDEFS) -I$(XMLSRCDIR)
XMLCCFLAGS = $(CCFLAGS) $(XMLDDEFS) -I$(XMLSRCDIR)

ASN1RT_OBJECTS = \
$(ASN1RT_C_OBJECTS) \
$(ASN1RT_CPPOBJECTS) \
$(OORT_C_RTOBJECTS) \
$(OSRT_C_RTOBJECTS) \
$(OSRT_CPP_OBJECTS)

all : outdirs \
$(DESTDIR)$(PS)$(LIBPFX)asn1rt$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1ber$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1json$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1oer$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1per$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1xml$(SO)

include ../../rtsrc/objects2.mk
include ../../rtxsrc/objects2.mk
include ../../rtbersrc/objects2.mk
include ../../rtjsonsrc/objects.mk
include ../../rtoersrc/objects.mk
include ../../rtpersrc/objects2.mk
include ../../rtxmlsrc/objects2.mk
include ../../rtxmlsrc/asn1Objects2.mk

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

$(DESTDIR)$(PS)$(LIBPFX)asn1xml$(SO) : \
$(OSXML_CPP_RTOBJECTS) $(ASN1XML_CPP_RTOBJECTS)
	$(LINK) $(LINKELF_) $(OSXML_CPP_RTOBJECTS) $(ASN1XML_CPP_RTOBJECTS) $(RTLIBPATH) $(LLSYS) $(LLRT)

$(DESTDIR)$(PS)rlm:
	perl -e "mkdir ('$(DESTDIR)/rlm', 0777) if (! -e '$(DESTDIR)/rlm')"


include ../../rtsrc/rules2.mk
include ../../rtxsrc/rules2.mk
include ../../rtbersrc/rules2.mk
include ../../rtjsonsrc/rules2.mk
include ../../rtoersrc/rules.mk
include ../../rtpersrc/rules2.mk
include ../../rtxmlsrc/rules2.mk

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
	$(RM) $(DESTDIR)$(PS)*$(SO) 
	$(RM) $(DESTDIR)$(PS)rtXml*IF$(XMLIFOBJ)
	$(RM_MSVC_FILES)
	$(RM) *~ 
