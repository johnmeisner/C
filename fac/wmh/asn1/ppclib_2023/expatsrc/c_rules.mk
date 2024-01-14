$(OBJDIR)$(PS)xmlparse$(OBJ): $(EXPATSRCDIR)/xmlparse.c $(EXPATSRCDIR)/expat.h
	$(CC) $(CFLAGS) $(EXPATDEFS) -c $(EXPATIPATHS) $(EXPATSRCDIR)/xmlparse.c

$(OBJDIR)$(PS)xmlrole$(OBJ): $(EXPATSRCDIR)/xmlrole.c $(EXPATSRCDIR)/expat.h
	$(CC) $(CFLAGS) $(EXPATDEFS) -c $(EXPATIPATHS) $(EXPATSRCDIR)/xmlrole.c

$(OBJDIR)$(PS)xmltok$(OBJ): $(EXPATSRCDIR)/xmltok.c $(EXPATSRCDIR)/expat.h
	$(CC) $(CFLAGS) $(EXPATDEFS) -c $(EXPATIPATHS) $(EXPATSRCDIR)/xmltok.c

$(OBJDIR)$(PS)xmltok_impl$(OBJ): $(EXPATSRCDIR)/xmltok_impl.c $(EXPATSRCDIR)/expat.h
	$(CC) $(CFLAGS) $(EXPATDEFS) -c $(EXPATIPATHS) $(EXPATSRCDIR)/xmltok_impl.c

$(OBJDIR)$(PS)xmltok_ns$(OBJ): $(EXPATSRCDIR)/xmltok_ns.c $(EXPATSRCDIR)/expat.h
	$(CC) $(CFLAGS) $(EXPATDEFS) -c $(EXPATIPATHS) $(EXPATSRCDIR)/xmltok_ns.c


