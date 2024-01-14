# C to object file compilation rules

$(OBJDIR)$(PS)DOCBparser$(OBJ): $(LIBXML2SRCDIR)/DOCBparser.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/SAX.h $(LIBXML2INC)/libxml/xlink.h \
 $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/DOCBparser.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/DOCBparser.c

$(OBJDIR)$(PS)HTMLparser$(OBJ): $(LIBXML2SRCDIR)/HTMLparser.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/HTMLtree.h \
 $(LIBXML2INC)/libxml/uri.h $(LIBXML2SRCDIR)/buf.h \
 $(LIBXML2SRCDIR)/enc.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/HTMLparser.c

$(OBJDIR)$(PS)HTMLtree$(OBJ): $(LIBXML2SRCDIR)/HTMLtree.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/HTMLtree.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/uri.h $(LIBXML2SRCDIR)/buf.h \
 $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/HTMLtree.c

$(OBJDIR)$(PS)SAX$(OBJ): $(LIBXML2SRCDIR)/SAX.c $(LIBXML2SRCDIR)/libxml.h $(LIBXML2SRCDIR)/config.h \
 $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/debugXML.h \
 $(LIBXML2INC)/libxml/xpath.h $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/HTMLtree.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/SAX.c

$(OBJDIR)$(PS)SAX2$(OBJ): $(LIBXML2SRCDIR)/SAX2.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/debugXML.h \
 $(LIBXML2INC)/libxml/xpath.h $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/HTMLtree.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/SAX2.c

$(OBJDIR)$(PS)buf$(OBJ): $(LIBXML2SRCDIR)/buf.c $(LIBXML2SRCDIR)/libxml.h $(LIBXML2SRCDIR)/config.h \
 $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/hash.h $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h $(LIBXML2SRCDIR)/buf.h \
 $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/buf.c

$(OBJDIR)$(PS)c14n$(OBJ): $(LIBXML2SRCDIR)/c14n.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/hash.h $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/xpathInternals.h \
 $(LIBXML2INC)/libxml/xpath.h $(LIBXML2INC)/libxml/c14n.h \
 $(LIBXML2SRCDIR)/buf.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/c14n.c

$(OBJDIR)$(PS)catalog$(OBJ): $(LIBXML2SRCDIR)/catalog.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/catalog.h $(LIBXML2SRCDIR)/buf.h \
 $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/catalog.c

$(OBJDIR)$(PS)chvalid$(OBJ): $(LIBXML2SRCDIR)/chvalid.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/xmlstring.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/chvalid.c

$(OBJDIR)$(PS)debugXML$(OBJ): $(LIBXML2SRCDIR)/debugXML.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/debugXML.h \
 $(LIBXML2INC)/libxml/xpath.h \
 $(LIBXML2INC)/libxml/HTMLtree.h \
 $(LIBXML2INC)/libxml/xpathInternals.h \
 $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/relaxng.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/debugXML.c

$(OBJDIR)$(PS)dict$(OBJ): $(LIBXML2SRCDIR)/dict.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/hash.h $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/dict.c

$(OBJDIR)$(PS)encoding$(OBJ): $(LIBXML2SRCDIR)/encoding.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/hash.h $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/HTMLparser.h $(LIBXML2SRCDIR)/buf.h \
 $(LIBXML2SRCDIR)/enc.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/encoding.c

$(OBJDIR)$(PS)entities$(OBJ): $(LIBXML2SRCDIR)/entities.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h $(LIBXML2SRCDIR)/save.h \
 $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/entities.c

$(OBJDIR)$(PS)error$(OBJ): $(LIBXML2SRCDIR)/error.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/SAX.h $(LIBXML2INC)/libxml/xlink.h \
 $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/error.c

$(OBJDIR)$(PS)globals$(OBJ): $(LIBXML2SRCDIR)/globals.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/globals.c

$(OBJDIR)$(PS)hash$(OBJ): $(LIBXML2SRCDIR)/hash.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/SAX.h $(LIBXML2INC)/libxml/xlink.h \
 $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/hash.c

$(OBJDIR)$(PS)legacy$(OBJ): $(LIBXML2SRCDIR)/legacy.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/hash.h $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/legacy.c

$(OBJDIR)$(PS)list$(OBJ): $(LIBXML2SRCDIR)/list.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/list.c

$(OBJDIR)$(PS)nanoftp$(OBJ): $(LIBXML2SRCDIR)/nanoftp.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/wsockcompat.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/nanoftp.c

$(OBJDIR)$(PS)nanohttp$(OBJ): $(LIBXML2SRCDIR)/nanohttp.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/wsockcompat.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/nanohttp.c

$(OBJDIR)$(PS)parser$(OBJ): $(LIBXML2SRCDIR)/parser.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/catalog.h \
 $(LIBXML2INC)/libxml/xmlschemastypes.h \
 $(LIBXML2INC)/libxml/schemasInternals.h \
 $(LIBXML2INC)/libxml/xmlschemas.h \
 $(LIBXML2INC)/libxml/relaxng.h $(LIBXML2SRCDIR)/buf.h \
 $(LIBXML2SRCDIR)/enc.h $(LIBXML2INC)/libxml/xpath.h \
 $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/parser.c

$(OBJDIR)$(PS)parserInternals$(OBJ): $(LIBXML2SRCDIR)/parserInternals.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/catalog.h $(LIBXML2SRCDIR)/buf.h \
 $(LIBXML2SRCDIR)/enc.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/parserInternals.c

$(OBJDIR)$(PS)pattern$(OBJ): $(LIBXML2SRCDIR)/pattern.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/pattern.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/pattern.c

$(OBJDIR)$(PS)relaxng$(OBJ): $(LIBXML2SRCDIR)/relaxng.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/relaxng.h \
 $(LIBXML2INC)/libxml/xmlschemastypes.h \
 $(LIBXML2INC)/libxml/schemasInternals.h \
 $(LIBXML2INC)/libxml/xmlschemas.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/relaxng.c

$(OBJDIR)$(PS)runsuite$(OBJ): $(LIBXML2SRCDIR)/runsuite.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/SAX.h $(LIBXML2INC)/libxml/xlink.h \
 $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/xmlreader.h \
 $(LIBXML2INC)/libxml/relaxng.h \
 $(LIBXML2INC)/libxml/xmlschemas.h \
 $(LIBXML2INC)/libxml/xpath.h \
 $(LIBXML2INC)/libxml/xpathInternals.h \
 $(LIBXML2INC)/libxml/xmlschemastypes.h \
 $(LIBXML2INC)/libxml/schemasInternals.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/runsuite.c

$(OBJDIR)$(PS)runtest$(OBJ): $(LIBXML2SRCDIR)/runtest.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/SAX.h $(LIBXML2INC)/libxml/xlink.h \
 $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/xmlreader.h \
 $(LIBXML2INC)/libxml/relaxng.h \
 $(LIBXML2INC)/libxml/xmlschemas.h \
 $(LIBXML2INC)/libxml/xinclude.h \
 $(LIBXML2INC)/libxml/xpath.h \
 $(LIBXML2INC)/libxml/xpathInternals.h \
 $(LIBXML2INC)/libxml/xpointer.h \
 $(LIBXML2INC)/libxml/xmlschemastypes.h \
 $(LIBXML2INC)/libxml/schemasInternals.h \
 $(LIBXML2INC)/libxml/pattern.h \
 $(LIBXML2INC)/libxml/c14n.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/HTMLtree.h \
 $(LIBXML2INC)/libxml/catalog.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/runtest.c

$(OBJDIR)$(PS)runxmlconf$(OBJ): $(LIBXML2SRCDIR)/runxmlconf.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/SAX.h $(LIBXML2INC)/libxml/xlink.h \
 $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/xmlreader.h \
 $(LIBXML2INC)/libxml/relaxng.h \
 $(LIBXML2INC)/libxml/xmlschemas.h \
 $(LIBXML2INC)/libxml/xpath.h \
 $(LIBXML2INC)/libxml/xpathInternals.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/runxmlconf.c

$(OBJDIR)$(PS)schematron$(OBJ): $(LIBXML2SRCDIR)/schematron.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/SAX.h $(LIBXML2INC)/libxml/xlink.h \
 $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/uri.h $(LIBXML2INC)/libxml/xpath.h \
 $(LIBXML2INC)/libxml/xpathInternals.h \
 $(LIBXML2INC)/libxml/pattern.h \
 $(LIBXML2INC)/libxml/schematron.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/schematron.c

$(OBJDIR)$(PS)threads$(OBJ): $(LIBXML2SRCDIR)/threads.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/xmlmemory.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/threads.c

$(OBJDIR)$(PS)tree$(OBJ): $(LIBXML2SRCDIR)/tree.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/HTMLtree.h \
 $(LIBXML2INC)/libxml/debugXML.h \
 $(LIBXML2INC)/libxml/xpath.h $(LIBXML2SRCDIR)/buf.h \
 $(LIBXML2SRCDIR)/save.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/tree.c

$(OBJDIR)$(PS)trio$(OBJ): $(LIBXML2SRCDIR)/trio.c $(LIBXML2SRCDIR)/triodef.h $(LIBXML2SRCDIR)/trio.h \
 $(LIBXML2SRCDIR)/triop.h $(LIBXML2SRCDIR)/trionan.h $(LIBXML2SRCDIR)/triostr.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/trio.c

$(OBJDIR)$(PS)trionan$(OBJ): $(LIBXML2SRCDIR)/trionan.c $(LIBXML2SRCDIR)/triodef.h \
 $(LIBXML2SRCDIR)/trionan.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/trionan.c

$(OBJDIR)$(PS)triostr$(OBJ): $(LIBXML2SRCDIR)/triostr.c $(LIBXML2SRCDIR)/triodef.h \
 $(LIBXML2SRCDIR)/triostr.h $(LIBXML2SRCDIR)/triop.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/triostr.c

$(OBJDIR)$(PS)uri$(OBJ): $(LIBXML2SRCDIR)/uri.c $(LIBXML2SRCDIR)/libxml.h $(LIBXML2SRCDIR)/config.h \
 $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/uri.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/uri.c

$(OBJDIR)$(PS)valid$(OBJ): $(LIBXML2SRCDIR)/valid.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/valid.c

$(OBJDIR)$(PS)xinclude$(OBJ): $(LIBXML2SRCDIR)/xinclude.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/uri.h $(LIBXML2INC)/libxml/xpath.h \
 $(LIBXML2INC)/libxml/xpointer.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/xinclude.h $(LIBXML2SRCDIR)/buf.h \
 $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/xinclude.c

$(OBJDIR)$(PS)xlink$(OBJ): $(LIBXML2SRCDIR)/xlink.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/xlink.c

$(OBJDIR)$(PS)xmlIO$(OBJ): $(LIBXML2SRCDIR)/xmlIO.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/nanohttp.h \
 $(LIBXML2INC)/libxml/nanoftp.h \
 $(LIBXML2INC)/libxml/catalog.h $(LIBXML2SRCDIR)/buf.h \
 $(LIBXML2SRCDIR)/enc.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/xmlIO.c

$(OBJDIR)$(PS)xmlcatalog$(OBJ): $(LIBXML2SRCDIR)/xmlcatalog.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/catalog.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/xmlcatalog.c

$(OBJDIR)$(PS)xmllint$(OBJ): $(LIBXML2SRCDIR)/xmllint.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/HTMLtree.h \
 $(LIBXML2INC)/libxml/xpath.h \
 $(LIBXML2INC)/libxml/debugXML.h \
 $(LIBXML2INC)/libxml/xinclude.h \
 $(LIBXML2INC)/libxml/catalog.h \
 $(LIBXML2INC)/libxml/xmlreader.h \
 $(LIBXML2INC)/libxml/relaxng.h \
 $(LIBXML2INC)/libxml/xmlschemas.h \
 $(LIBXML2INC)/libxml/schematron.h \
 $(LIBXML2INC)/libxml/pattern.h \
 $(LIBXML2INC)/libxml/c14n.h \
 $(LIBXML2INC)/libxml/xmlsave.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/xmllint.c

$(OBJDIR)$(PS)xmlmemory$(OBJ): $(LIBXML2SRCDIR)/xmlmemory.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/xmlmemory.c

$(OBJDIR)$(PS)xmlmodule$(OBJ): $(LIBXML2SRCDIR)/xmlmodule.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/xmlmodule.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/xmlmodule.c

$(OBJDIR)$(PS)xmlreader$(OBJ): $(LIBXML2SRCDIR)/xmlreader.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/xmlreader.h \
 $(LIBXML2INC)/libxml/relaxng.h \
 $(LIBXML2INC)/libxml/xmlschemas.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/xinclude.h \
 $(LIBXML2INC)/libxml/pattern.h $(LIBXML2SRCDIR)/buf.h \
 $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/xmlreader.c

$(OBJDIR)$(PS)xmlregexp$(OBJ): $(LIBXML2SRCDIR)/xmlregexp.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/hash.h $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/xmlunicode.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/xmlregexp.c

$(OBJDIR)$(PS)xmlsave$(OBJ): $(LIBXML2SRCDIR)/xmlsave.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/xmlsave.h \
 $(LIBXML2INC)/libxml/HTMLtree.h $(LIBXML2SRCDIR)/buf.h \
 $(LIBXML2SRCDIR)/enc.h $(LIBXML2SRCDIR)/save.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/xmlsave.c

$(OBJDIR)$(PS)xmlschemas$(OBJ): $(LIBXML2SRCDIR)/xmlschemas.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/xmlschemas.h \
 $(LIBXML2INC)/libxml/schemasInternals.h \
 $(LIBXML2INC)/libxml/xmlschemastypes.h \
 $(LIBXML2INC)/libxml/pattern.h \
 $(LIBXML2INC)/libxml/xmlreader.h \
 $(LIBXML2INC)/libxml/relaxng.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/xmlschemas.c

$(OBJDIR)$(PS)xmlschemastypes$(OBJ): $(LIBXML2SRCDIR)/xmlschemastypes.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/xpath.h $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/xmlschemas.h \
 $(LIBXML2INC)/libxml/schemasInternals.h \
 $(LIBXML2INC)/libxml/xmlschemastypes.h \
 $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/xmlschemastypes.c

$(OBJDIR)$(PS)xmlstring$(OBJ): $(LIBXML2SRCDIR)/xmlstring.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/xmlstring.c

$(OBJDIR)$(PS)xmlunicode$(OBJ): $(LIBXML2SRCDIR)/xmlunicode.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlunicode.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/xmlstring.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/xmlunicode.c

$(OBJDIR)$(PS)xmlwriter$(OBJ): $(LIBXML2SRCDIR)/xmlwriter.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/HTMLtree.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/xmlwriter.h $(LIBXML2SRCDIR)/buf.h \
 $(LIBXML2SRCDIR)/enc.h $(LIBXML2SRCDIR)/save.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/xmlwriter.c

$(OBJDIR)$(PS)xpath$(OBJ): $(LIBXML2SRCDIR)/xpath.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h $(LIBXML2INC)/libxml/hash.h \
 $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/xpath.h \
 $(LIBXML2INC)/libxml/xpathInternals.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/xpointer.h \
 $(LIBXML2INC)/libxml/debugXML.h \
 $(LIBXML2INC)/libxml/pattern.h $(LIBXML2SRCDIR)/buf.h \
 $(LIBXML2SRCDIR)/timsort.h $(LIBXML2SRCDIR)/trionan.c $(LIBXML2SRCDIR)/triodef.h \
 $(LIBXML2SRCDIR)/trionan.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/xpath.c

$(OBJDIR)$(PS)xpointer$(OBJ): $(LIBXML2SRCDIR)/xpointer.c $(LIBXML2SRCDIR)/libxml.h \
 $(LIBXML2SRCDIR)/config.h $(LIBXML2INC)/libxml/xmlversion.h \
 $(LIBXML2INC)/libxml/xmlexports.h \
 $(LIBXML2INC)/libxml/xpointer.h \
 $(LIBXML2INC)/libxml/tree.h \
 $(LIBXML2INC)/libxml/xmlstring.h \
 $(LIBXML2INC)/libxml/xmlregexp.h \
 $(LIBXML2INC)/libxml/dict.h \
 $(LIBXML2INC)/libxml/xmlmemory.h \
 $(LIBXML2INC)/libxml/threads.h \
 $(LIBXML2INC)/libxml/globals.h \
 $(LIBXML2INC)/libxml/parser.h \
 $(LIBXML2INC)/libxml/hash.h $(LIBXML2INC)/libxml/valid.h \
 $(LIBXML2INC)/libxml/xmlerror.h \
 $(LIBXML2INC)/libxml/list.h \
 $(LIBXML2INC)/libxml/xmlautomata.h \
 $(LIBXML2INC)/libxml/entities.h \
 $(LIBXML2INC)/libxml/encoding.h \
 $(LIBXML2INC)/libxml/xmlIO.h $(LIBXML2INC)/libxml/SAX.h \
 $(LIBXML2INC)/libxml/xlink.h $(LIBXML2INC)/libxml/SAX2.h \
 $(LIBXML2INC)/libxml/xpath.h \
 $(LIBXML2INC)/libxml/parserInternals.h \
 $(LIBXML2INC)/libxml/HTMLparser.h \
 $(LIBXML2INC)/libxml/chvalid.h \
 $(LIBXML2INC)/libxml/uri.h \
 $(LIBXML2INC)/libxml/xpathInternals.h $(LIBXML2SRCDIR)/elfgcchack.h
	$(CC) $(CFLAGS) $(LIBXML2DEFS) -c -I$(LIBXML2INC) $(OBJOUT) $(LIBXML2SRCDIR)/xpointer.c
