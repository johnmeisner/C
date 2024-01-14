/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
 *
 * This software is furnished under a license and may be used and copied
 * only in accordance with the terms of such license and with the
 * inclusion of the above copyright notice. This software or any other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of the software is hereby
 * transferred.
 *
 * The information in this software is subject to change without notice
 * and should not be construed as a commitment by Objective Systems, Inc.
 *
 * PROPRIETARY NOTICE
 *
 * This software is an unpublished work subject to a confidentiality agreement
 * and is protected by copyright and trade secret law.  Unauthorized copying,
 * redistribution or other use of this work is prohibited.
 *
 * The above notice of copyright on this source code product does not indicate
 * any actual or intended publication of such source code.
 *
 *****************************************************************************/

#include <stdarg.h>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/util/XMLUTF8Transcoder.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

#include "rtxmlsrc/rtSaxCppParserIF.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/OSRTInputStreamIF.h"

#if defined (XML_UNICODE) && defined(XML_UNICODE_WCHAR_T)
#error C SAX does not support XML_UNICODE_WCHAR_T macro at the moment
#endif

#define XML_BUF_SIZE 2048

XERCES_CPP_NAMESPACE_USE

///////////////////////////////////////////////////
class OSXercesXMLReader : public OSXMLReaderClass, public DefaultHandler
{
   DECLARE_XMLBASEIMP
public:
   class StopParserException {};

   OSXMLParserCtxtIF*  mpContext;
   SAX2XMLReader*      mpParser;
   OSXMLDefaultHandlerIF* userSaxHandler;

   OSXercesXMLReader(OSXMLParserCtxtIF* pContext);
   virtual ~OSXercesXMLReader();

   virtual int parse ();

   virtual int parse (OSRTInputStreamIF& source);

   virtual int parse (const char* const pBuffer,
                      size_t bufSize);

   virtual int parse (const char* const systemId);

   void setSaxHandler(OSXMLDefaultHandlerIF* const handler);

protected:
   // interface functions for callbacks
   virtual void startElement
      (const XMLCh* const uri,
       const XMLCh* const localname,
       const XMLCh* const qname,
       const Attributes& attrs);

   virtual void characters
      (const XMLCh* const chars, const unsigned int length);

   virtual void endElement
      (const XMLCh* const uri,
       const XMLCh* const localname,
       const XMLCh* const qname);

   virtual void startCDATA ();
   virtual void endCDATA ();
};

class OSCustomBinInputStream : public BinInputStream
{
   OSRTInputStreamIF& source;
public :
   OSCustomBinInputStream (OSRTInputStreamIF& source_) : source (source_) {}

   // -----------------------------------------------------------------------
   //  Virtual destructor for derived classes
   // -----------------------------------------------------------------------
   virtual ~OSCustomBinInputStream() {}


   // -----------------------------------------------------------------------
   //  The virtual input stream interface
   // -----------------------------------------------------------------------
   virtual XMLFilePos curPos() const {
      return (XMLFilePos)source.currentPos ();
   }

   virtual XMLSize_t readBytes (
      XMLByte* const toFill,
      const XMLSize_t maxToRead)
   {
      return (unsigned int)source.read
         ((OSOCTET* const)toFill, maxToRead);
   }

   virtual const XMLCh* getContentType() const { return 0; }
};

class OSCustomInputSource : public InputSource
{
   OSRTInputStreamIF& source;
 public:
   OSCustomInputSource (OSRTInputStreamIF& source_):
      source (source_)
   {
      XMLPlatformUtils::Initialize();
   }

   ~OSCustomInputSource ()
   {
      XMLPlatformUtils::Terminate();
   }

   BinInputStream* makeStream() const {
      return new OSCustomBinInputStream (source);
   }
};

OSXercesXMLReader::OSXercesXMLReader(OSXMLParserCtxtIF* pContext) :
   mpContext (pContext)
{
   rtSaxCppLockXmlLibrary ();

   // Create a SAX parser object
   mpParser = XMLReaderFactory::createXMLReader();

   mpParser->setContentHandler (this);
   mpParser->setErrorHandler (this);

   userSaxHandler = 0;
}

OSXercesXMLReader::~OSXercesXMLReader()
{
   // Delete the parser.  Must be done prior to calling Terminate.
   delete mpParser;

   // And call the termination method
   rtSaxCppUnlockXmlLibrary ();
}

int OSXercesXMLReader::parse ()
{
   OSRTInputStreamPtr in (mpContext->createInputStream ());
   return parse (*in);
}

class OSXercesString {
   OSUTF8CHAR* utf8;
   unsigned length;

   static XMLUTF8Transcoder* transcoder;
   static unsigned getSizeInBytes (const XMLCh* str, unsigned* srcLen);
 public:
   OSXercesString (const XMLCh* str, unsigned len = UINT_MAX);
   inline ~OSXercesString () { delete [] utf8; }

   inline unsigned getLength () { return length; }
   inline operator const OSUTF8CHAR* () { return utf8; }
   inline operator const char* () { return (const char*)utf8; }
} ;
XMLUTF8Transcoder* OSXercesString::transcoder;

OSXercesString::OSXercesString (const XMLCh* str, unsigned len_)
{
   unsigned srcLen = len_;
   XMLSize_t eaten;
   length = getSizeInBytes (str, &srcLen);
   utf8 = new OSUTF8CHAR [length + 1];
   if (length > 0) {
      if (transcoder == 0) {
         transcoder = new XMLUTF8Transcoder ((const XMLCh*)XMLStrL("UTF-8"), 0);
      }
      transcoder->transcodeTo
         (str, srcLen, (XMLByte*) utf8, length, eaten, XMLTranscoder::UnRep_RepChar);
   }
   utf8 [length] = 0;
}

unsigned OSXercesString::getSizeInBytes (const XMLCh* str, unsigned* srcLen)
{
   unsigned l = 0, totLen = *srcLen;

   *srcLen = 0;
   if (str != 0) {
      for (const XMLCh* p = str; (*srcLen) < totLen && *p != 0; p++) {
         l += rtxUTF8CharSize ((OS32BITCHAR)*p);
         (*srcLen)++;
      }
   }
   return l;
}


int OSXercesXMLReader::parse (OSRTInputStreamIF& source)
{
   try {
      // Create a memory buffer input source
      OSCustomInputSource xmlMemBuf (source);

      // Parse the document
      mpParser->parse (xmlMemBuf);
   }
   catch (StopParserException&) {
      return LOG_RTERRNEW1 (mpContext->getContext (), RTERR_XMLPARSE,
         rtxErrAddStrParm (mpContext->getContext (), "Fatal error."));
   }
   catch (SAXException& ex) {
      OSXercesString cstr (ex.getMessage ());
      return LOG_RTERRNEW1 (mpContext->getContext (), RTERR_XMLPARSE,
         rtxErrAddStrParm (mpContext->getContext (), (const char*)cstr));
   }
   catch (XMLException& ex) {
      OSXercesString cstr (ex.getMessage ());
      return LOG_RTERRNEW1 (mpContext->getContext (), RTERR_XMLPARSE,
         rtxErrAddStrParm (mpContext->getContext (), (const char*)cstr));
   }
   return 0;
}

int OSXercesXMLReader::parse (const char* const pBuffer, size_t bufSize)
{
   try {
      // Create a memory buffer input source
      MemBufInputSource xmlMemBuf
         ((XMLByte*)pBuffer, (unsigned int)bufSize, "");

      // Parse the document
      mpParser->parse (xmlMemBuf);
   }
   catch (StopParserException&) {
      return LOG_RTERRNEW1 (mpContext->getContext (), RTERR_XMLPARSE,
         rtxErrAddStrParm (mpContext->getContext (), "Fatal error."));
   }
   catch (SAXException& ex) {
      OSXercesString cstr (ex.getMessage ());
      return LOG_RTERRNEW1 (mpContext->getContext (), RTERR_XMLPARSE,
         rtxErrAddStrParm (mpContext->getContext (), (const char*)cstr));
   }
   catch (XMLException& ex) {
      OSXercesString cstr (ex.getMessage ());
      return LOG_RTERRNEW1 (mpContext->getContext (), RTERR_XMLPARSE,
         rtxErrAddStrParm (mpContext->getContext (), (const char*)cstr));
   }
   return 0;
}

int OSXercesXMLReader::parse (const char* const systemId)
{
   try {
      // Parse the document
      mpParser->parse (systemId);
   }
   catch (StopParserException&) {
      return LOG_RTERRNEW1 (mpContext->getContext (), RTERR_XMLPARSE,
         rtxErrAddStrParm (mpContext->getContext (), "Fatal error."));
   }
   catch (SAXException& ex) {
      OSXercesString cstr (ex.getMessage ());
      return LOG_RTERRNEW1 (mpContext->getContext (), RTERR_XMLPARSE,
         rtxErrAddStrParm (mpContext->getContext (), (const char*)cstr));
   }
   catch (XMLException& ex) {
      OSXercesString cstr (ex.getMessage ());
      return LOG_RTERRNEW1 (mpContext->getContext (), RTERR_XMLPARSE,
         rtxErrAddStrParm (mpContext->getContext (), (const char*)cstr));
   }
   return 0;
}

void OSXercesXMLReader::setSaxHandler(OSXMLDefaultHandlerIF* const handler)
{
   userSaxHandler = handler;
   userSaxHandler->resetErrorInfo ();
}

class OSXercesStringArray {
   OSXercesString** array;
   XMLSize_t size;
   const OSUTF8CHAR* * utf8array;
 public:
   inline OSXercesStringArray (XMLSize_t sz) {
      array = new OSXercesString* [sz];
      OSCRTLMEMSET (array, 0, sizeof (OSXercesString*) * sz);
      size = sz;
      utf8array = new const OSUTF8CHAR* [sz + 2];
      OSCRTLMEMSET (utf8array, 0, sizeof (OSUTF8CHAR*) * (sz + 2));
   }
   ~OSXercesStringArray ();
   inline void set (XMLSize_t idx, OSXercesString* elem) {
      if (idx >= size) return;
      array [idx] = elem;
      utf8array [idx] = (const OSUTF8CHAR*)(*elem);
   }
   inline const OSUTF8CHAR** getUtf8Array () { return utf8array; }
} ;

OSXercesStringArray::~OSXercesStringArray ()
{
   for (unsigned i = 0; i < size; i++) {
      delete array[i];
   }
   delete [] array;
   delete [] utf8array;
}

void OSXercesXMLReader::startElement
   (const XMLCh* const uri,
    const XMLCh* const localname,
    const XMLCh* const qname,
    const Attributes& attrs)
{
   OSCTXT* pctxt = mpContext->getContext ();
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo != 0) {
      ctxtInfo->mbCdataProcessed = FALSE;

      ctxtInfo->mSaxLevel++;
      if (ctxtInfo->mSkipLevel < 0) {
         // fatal error, stop parser
         throw StopParserException ();
      }
      else if (ctxtInfo->mSkipLevel > 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel)
         return;
   }

   if (userSaxHandler != 0) {
      OSXercesString uri_ (uri), localname_ (localname), qname_ (qname);
      OSXercesStringArray strArr ( attrs.getLength () * 2); // names + values

      // Convert attributes
      for (XMLSize_t i = 0, n = attrs.getLength (); i < n; i++) {
         const XMLCh* qname = attrs.getQName (i);
         const XMLCh* value = attrs.getValue (i);

         strArr.set (i * 2, new OSXercesString (qname));
         strArr.set (i * 2 + 1, new OSXercesString (value));
      }

      userSaxHandler->startElement (uri_, localname_, qname_,
         (const OSUTF8CHAR**)strArr.getUtf8Array ());
   }
}

void OSXercesXMLReader::characters
   (const XMLCh* const chars, const unsigned int length)
{
   OSCTXT* pctxt = mpContext->getContext ();
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo) {
      if (ctxtInfo->mSkipLevel != 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel)
         return;
   }

   if (userSaxHandler != 0) {
      OSXercesString chars_ (chars, length);
      userSaxHandler->characters (chars_, chars_.getLength ());
   }
}

void OSXercesXMLReader::endElement
   (const XMLCh* const uri,
    const XMLCh* const localname,
    const XMLCh* const qname)
{
   OSCTXT* pctxt = mpContext->getContext ();
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
   OSBOOL bSkip = FALSE;

   if (ctxtInfo) {
      if (ctxtInfo->mSkipLevel != 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel) {
         bSkip = TRUE;
      }
   }

   if (!bSkip && userSaxHandler) {
      OSXercesString uri_ (uri), localname_ (localname), qname_ (qname);
      userSaxHandler->endElement (uri_, localname_, qname_);
   }

   if (ctxtInfo != 0) {
      if (ctxtInfo->mSkipLevel > 0 && ctxtInfo->mSaxLevel == ctxtInfo->mSkipLevel)
         ctxtInfo->mSkipLevel = 0; /* reset skipLevel */
      ctxtInfo->mSaxLevel--;

      ctxtInfo->mbCdataProcessed = FALSE;
   }
}

void OSXercesXMLReader::startCDATA ()
{
   OSCTXT* pctxt = mpContext->getContext ();
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo != 0) {
      if (ctxtInfo->mSkipLevel != 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel)
         return;
      ctxtInfo->mbCdataProcessed = TRUE;
   }
}

void OSXercesXMLReader::endCDATA ()
{
}

OSXMLReaderClass* rtSaxCppCreateXmlReader (OSXMLParserCtxtIF* pContext,
                                      OSXMLDefaultHandlerIF* pSaxHandler)
{
   OSXercesXMLReader *reader = new OSXercesXMLReader (pContext);
   if (reader->mpParser == 0) {
      delete reader;
      return 0;
   }
   reader->setSaxHandler (pSaxHandler);
   return reader;
}

int rtSaxCppEnableThreadSafety ()
{
   return -1; // not supported
}

void rtSaxCppLockXmlLibrary ()
{
   XMLPlatformUtils::Initialize();
}

void rtSaxCppUnlockXmlLibrary ()
{
   XMLPlatformUtils::Terminate();
}


