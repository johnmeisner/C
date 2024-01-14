# Build static run-time libraries

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

COMFLAGS = $(CVARSMTD_) $(MCFLAGS) $(CDEV_) $(MCOMFLAGS)  -D_NO_LICENSE_CHECK -DCPP $(CDEBUG_)
CFLAGS   = $(COMFLAGS) $(CFLAGS_)
CCFLAGS  = $(COMFLAGS) $(CCFLAGS_)
IPATHS   = \
-I$(RTSRCDIR) -I$(BERSRCDIR) -I$(PERSRCDIR) \
-I$(OSROOTDIR) $(IPATHS_)
XMLIFOBJ = _a$(OBJ)

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
XMLCFLAGS = $(CFLAGS) $(XMLDEFS) -I$(XMLSRCDIR)
XMLCCFLAGS = $(CCFLAGS) $(XMLDEFS) -I$(XMLSRCDIR)

ASN1RT_OBJECTS = \
$(ASN1RT_C_OBJECTS) \
$(ASN1RT_CPPOBJECTS) \
$(OORT_C_RTOBJECTS) \
$(OSRT_C_RTOBJECTS) \
$(OSRT_CPP_OBJECTS)

all : outdirs \
$(DESTDIR)$(PS)$(LIBPFX)asn1rt$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1ber$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1json$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1oer$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1per$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1xml$(A)

include ../../rtsrc/objects2.mk
include ../../rtxsrc/objects2.mk
include ../../rtbersrc/objects2.mk
include ../../rtjsonsrc/objects.mk
include ../../rtoersrc/objects.mk
include ../../rtpersrc/objects2.mk
include ../../rtxmlsrc/objects2.mk
include ../../rtxmlsrc/asn1Objects2.mk

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

$(DESTDIR)$(PS)$(LIBPFX)asn1xml$(A) : \
$(OSXML_CPP_RTOBJECTS) $(ASN1XML_CPP_RTOBJECTS)
	$(LIBCMD) $(OSXML_CPP_RTOBJECTS) $(ASN1XML_CPP_RTOBJECTS)

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
	$(RM) $(DESTDIR)$(PS)*$(A) 
	$(RM) $(DESTDIR)$(PS)rtXml*IF$(XMLIFOBJ)
	$(RM_MSVC_FILES)
	$(RM) *~ 
