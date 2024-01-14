# EXPAT defs
EXPATROOT = $(OSROOTDIR)/expatsrc
EXPATINC  = $(EXPATROOT)

# generic XML defs
XMLDEFS  = -DXML_STATIC
XMLDDEFS =  
XMLINC  = -I$(EXPATINC)
XMLLIBDIR = $(LIBDIR)

XMLLIBNAME = libexpat.a
XMLLIBIFOBJ = rtXmlExpatIF_a$(OBJ)
XMLLINKLIB = -lexpat
XMLLINKMDLIB = -lexpat

# dynamic libraries (for linking to DLL's)
XMLLIBDNAME = libexpat.so
XMLLIBDIFOBJ = rtXmlExpatIF$(OBJ)
XMLLINKDLIB = -lexpat

