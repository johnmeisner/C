# Build shared run-time libraries

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

COMFLAGS = $(CVARSMDD_) $(CELF_) $(MCFLAGS) $(CDEV_)  -D_NO_LICENSE_CHECK $(CDEBUG_)
CFLAGS	 = $(COMFLAGS) $(CFLAGS_)
IPATHS	 = -I$(OSROOTDIR) -I$(RTSRCDIR) -I$(RTXSRCDIR) $(IPATHS_)
RTLIBPATH = $(LPPFX)$(DESTDIR)
XMLIFOBJ = $(OBJ)

RTCFLAGS = $(CFLAGS)
RTXCFLAGS = $(CFLAGS)
RTXCCFLAGS = $(CFLAGS)
RTXCPPFLAGS = $(CFLAGS)
BERCFLAGS = $(CFLAGS) -I$(BERSRCDIR)
JSONCFLAGS = $(CFLAGS) -I$(JSONSRCDIR)
OERCFLAGS = $(CFLAGS) -I$(OERSRCDIR)
PERCFLAGS = $(CFLAGS) -I$(PERSRCDIR)
MDERCFLAGS = $(CFLAGS) -I$(MDERSRCDIR)
XMLCFLAGS = $(CFLAGS) $(XMLDDEFS) -I$(XMLSRCDIR)

all : outdirs \
$(DESTDIR)$(PS)$(LIBPFX)asn1rt$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1ber$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1json$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1oer$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1per$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1xml$(SO) \
$(DESTDIR)$(PS)$(LIBPFX)asn1mder$(SO) \
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

$(DESTDIR)$(PS)$(LIBPFX)asn1rt$(SO) : \
$(OSRT_C_RTOBJECTS) $(OORT_C_RTOBJECTS) $(ASN1RT_C_OBJECTS) $(STUB_OBJECTS_RT)
	$(LINK) $(LINKELF_) $(OSRT_C_RTOBJECTS) $(OORT_C_RTOBJECTS) $(ASN1RT_C_OBJECTS) $(STUB_OBJECTS_RT) $(RTLIBPATH) $(LLSYS)


$(DESTDIR)$(PS)$(LIBPFX)asn1ber$(SO) : \
$(BER_C_OBJECTS) $(STUB_OBJECTS_BER)
	$(LINK) $(LINKELF_) $(BER_C_OBJECTS) $(STUB_OBJECTS_BER) $(RTLIBPATH) $(LLSYS) $(LLRT)

$(DESTDIR)$(PS)$(LIBPFX)asn1json$(SO) : \
$(OSRT_JSON_RTOBJECTS) $(ASN1_JSON_RTOBJECTS)
	$(LINK) $(LINKELF_) $(OSRT_JSON_RTOBJECTS) $(ASN1_JSON_RTOBJECTS) $(RTLIBPATH) $(LLSYS) $(LLRT) $(LLXML)

$(DESTDIR)$(PS)$(LIBPFX)asn1oer$(SO) : $(OER_C_OBJECTS) 
	$(LINK) $(LINKELF_) $(OER_C_OBJECTS) $(RTLIBPATH) $(LLSYS) $(LLRT)

$(DESTDIR)$(PS)$(LIBPFX)asn1per$(SO) : \
$(PER_C_OBJECTS) $(STUB_OBJECTS_PER)
	$(LINK) $(LINKELF_) $(PER_C_OBJECTS)  $(RTLIBPATH) $(LLSYS) $(LLRT)

$(DESTDIR)$(PS)$(LIBPFX)asn1xml$(SO) : \
$(OSXML_C_RTOBJECTS) $(ASN1XML_RTOBJECTS)
	$(LINK) $(LINKELF_) $(OSXML_C_RTOBJECTS) $(ASN1XML_RTOBJECTS) $(RTLIBPATH) $(LLSYS) $(LLRT)

$(DESTDIR)$(PS)$(LIBPFX)asn1mder$(SO) : $(OSRT_MDER_RTOBJECTS)
	$(LINK) $(LINKELF_) $(OSRT_MDER_RTOBJECTS) $(RTLIBPATH) $(LLSYS) $(LLRT)



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

CFLAGSIF = -I. -I$(OSROOTDIR) $(IPATHS_) $(CFLAGS)  -DLIBXML_STATIC $(OBJOUT)

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
	$(RM) $(DESTDIR)$(PS)*$(SO) 
	$(RM) *~ 
