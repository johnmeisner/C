# Build static run-time libraries

# This makefile must be compatible with GNU make. In particular, recipie
# lines must begin with a tab character. This requirement extends to all files
# that are included.

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

COMFLAGS = $(CVARSMTD_) $(MCFLAGS) $(CDEV_)  -D_NO_LICENSE_CHECK $(CDEBUG_)
CFLAGS	 = $(COMFLAGS) $(CFLAGS_)
IPATHS	 = -I$(OSROOTDIR) -I$(RTSRCDIR) -I$(RTXSRCDIR) $(IPATHS_)
XMLIFOBJ = _a$(OBJ)

RTCFLAGS = $(CFLAGS)
RTXCFLAGS = $(CFLAGS)
RTXCCFLAGS = $(CFLAGS)
RTXCPPFLAGS = $(CFLAGS)
BERCFLAGS = $(CFLAGS) -I$(BERSRCDIR)
JSONCFLAGS = $(CFLAGS) -I$(JSONSRCDIR)
OERCFLAGS = $(CFLAGS) -I$(OERSRCDIR)
PERCFLAGS = $(CFLAGS) -I$(PERSRCDIR)
MDERCFLAGS = $(CFLAGS) -I$(MDERSRCDIR)
XMLCFLAGS = $(CFLAGS) $(XMLDEFS) -I$(XMLSRCDIR)

all : outdirs \
$(DESTDIR)$(PS)$(LIBPFX)asn1rt$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1ber$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1json$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1oer$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1per$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1xml$(A) \
$(DESTDIR)$(PS)$(LIBPFX)asn1mder$(A) \
$(ASN1RT3GPPLIB) $(CBORLIBPATH) $(FULLNAS) bench_objects


include ../../rtsrc/objects2.mk
include ../../rtxsrc/objects2.mk
include ../../rtbersrc/objects2.mk
include ../../rtjsonsrc/objects.mk
include ../../rtoersrc/objects.mk
include ../../rtpersrc/objects2.mk
include ../../rtxmlsrc/objects2.mk
include ../../rtxmlsrc/asn1Objects2.mk
include ../../rtmdersrc/objects.mk

$(DESTDIR)$(PS)$(LIBPFX)asn1rt$(A) : \
$(OSRT_C_RTOBJECTS) $(OORT_C_RTOBJECTS) $(ASN1RT_C_OBJECTS) $(STUB_OBJECTS_RT)
	$(LIBCMD) $(OSRT_C_RTOBJECTS) $(OORT_C_RTOBJECTS) $(ASN1RT_C_OBJECTS) $(STUB_OBJECTS_RT)


$(DESTDIR)$(PS)$(LIBPFX)asn1ber$(A) : \
$(BER_C_OBJECTS) $(STUB_OBJECTS_BER)
	$(LIBCMD) $(BER_C_OBJECTS) $(STUB_OBJECTS_BER)

$(DESTDIR)$(PS)$(LIBPFX)asn1json$(A) : \
$(OSRT_JSON_RTOBJECTS) $(ASN1_JSON_RTOBJECTS)
	$(LIBCMD) $(OSRT_JSON_RTOBJECTS) $(ASN1_JSON_RTOBJECTS)

$(DESTDIR)$(PS)$(LIBPFX)asn1oer$(A) : $(OER_C_OBJECTS) 
	$(LIBCMD) $(OER_C_OBJECTS)

$(DESTDIR)$(PS)$(LIBPFX)asn1per$(A) : \
$(PER_C_OBJECTS) $(STUB_OBJECTS_PER)
	$(LIBCMD) $(PER_C_OBJECTS) 

$(DESTDIR)$(PS)$(LIBPFX)asn1xml$(A) : \
$(OSXML_C_RTOBJECTS) $(ASN1XML_RTOBJECTS)
	$(LIBCMD) $(OSXML_C_RTOBJECTS) $(ASN1XML_RTOBJECTS)

$(DESTDIR)$(PS)$(LIBPFX)asn1mder$(A) : $(OSRT_MDER_RTOBJECTS)
	$(LIBCMD) $(OSRT_MDER_RTOBJECTS)



bench_objects : $(OSRT_C_BENCH_OBJECTS)

$(DESTDIR)$(PS)rlm:
	perl -e "mkdir ('$(DESTDIR)/rlm', 0777) if (! -e '$(DESTDIR)/rlm')"


include ../../rtsrc/rules2.mk
include ../../rtxsrc/rules2.mk
include ../../rtbersrc/rules2.mk
include ../../rtjsonsrc/rules2.mk
include ../../rtoersrc/rules.mk
include ../../rtpersrc/rules2.mk
include ../../rtxmlsrc/rules2.mk
include ../../rtmdersrc/c_rules.mk

CFLAGSIF = -I. -I$(OSROOTDIR) $(IPATHS_) $(CFLAGS) -DXML_STATIC -DLIBXML_STATIC $(OBJOUT)

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
	$(RM) $(SRCDIR)$(PS)*.h 
	$(RM) $(SRCDIR)$(PS)*.c 
	$(RM) $(DESTDIR)$(PS)*$(A) 
	$(RM) *~ 
