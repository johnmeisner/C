AttributesImpl$(OBJ): $(EXPATSRCDIR)/sax/AttributesImpl.cpp $(EXPATSRCDIR)/sax/AttributesImpl.hpp
	$(CCC) $(CCFLAGS) -c $(IPATHS) $(XMLINC) $(XMLDEFS) $(EXPATSRCDIR)/sax/AttributesImpl.cpp

BinFileInputStream$(OBJ): $(EXPATSRCDIR)/sax/BinFileInputStream.cpp $(EXPATSRCDIR)/sax/BinFileInputStream.hpp
	$(CCC) $(CCFLAGS) -c $(IPATHS) $(XMLINC) $(XMLDEFS) $(EXPATSRCDIR)/sax/BinFileInputStream.cpp

BinInputStream$(OBJ): $(EXPATSRCDIR)/sax/BinInputStream.cpp $(EXPATSRCDIR)/sax/BinInputStream.hpp
	$(CCC) $(CCFLAGS) -c $(IPATHS) $(XMLINC) $(XMLDEFS) $(EXPATSRCDIR)/sax/BinInputStream.cpp

BinMemInputStream$(OBJ): $(EXPATSRCDIR)/sax/BinMemInputStream.cpp $(EXPATSRCDIR)/sax/BinMemInputStream.hpp
	$(CCC) $(CCFLAGS) -c $(IPATHS) $(XMLINC) $(XMLDEFS) $(EXPATSRCDIR)/sax/BinMemInputStream.cpp

InputSource$(OBJ): $(EXPATSRCDIR)/sax/InputSource.cpp $(EXPATSRCDIR)/sax/InputSource.hpp
	$(CCC) $(CCFLAGS) -c $(IPATHS) $(XMLINC) $(XMLDEFS) $(EXPATSRCDIR)/sax/InputSource.cpp

LocalFileInputSource$(OBJ): $(EXPATSRCDIR)/sax/LocalFileInputSource.cpp $(EXPATSRCDIR)/sax/LocalFileInputSource.hpp
	$(CCC) $(CCFLAGS) -c $(IPATHS) $(XMLINC) $(XMLDEFS) $(EXPATSRCDIR)/sax/LocalFileInputSource.cpp

MemBufInputSource$(OBJ): $(EXPATSRCDIR)/sax/MemBufInputSource.cpp $(EXPATSRCDIR)/sax/MemBufInputSource.hpp
	$(CCC) $(CCFLAGS) -c $(IPATHS) $(XMLINC) $(XMLDEFS) $(EXPATSRCDIR)/sax/MemBufInputSource.cpp

PlatformUtils$(OBJ): $(EXPATSRCDIR)/sax/PlatformUtils.cpp $(EXPATSRCDIR)/sax/PlatformUtils.hpp
	$(CCC) $(CCFLAGS) -c $(IPATHS) $(XMLINC) $(XMLDEFS) $(EXPATSRCDIR)/sax/PlatformUtils.cpp

SAX2XMLReaderImpl$(OBJ): $(EXPATSRCDIR)/sax/SAX2XMLReaderImpl.cpp \
$(EXPATSRCDIR)/sax/SAX2XMLReaderImpl.hpp $(EXPATSRCDIR)/sax/SAX2XMLReader.hpp
	$(CCC) $(CCFLAGS) -c $(IPATHS) $(XMLINC) $(XMLDEFS) $(EXPATSRCDIR)/sax/SAX2XMLReaderImpl.cpp

SAXException$(OBJ): $(EXPATSRCDIR)/sax/SAXException.cpp $(EXPATSRCDIR)/sax/SAXException.hpp
	$(CCC) $(CCFLAGS) -c $(IPATHS) $(XMLINC) $(XMLDEFS) $(EXPATSRCDIR)/sax/SAXException.cpp

XMLString$(OBJ): $(EXPATSRCDIR)/sax/XMLString.cpp $(EXPATSRCDIR)/sax/XMLString.hpp
	$(CCC) $(CCFLAGS) -c $(IPATHS) $(XMLINC) $(XMLDEFS) $(EXPATSRCDIR)/sax/XMLString.cpp

