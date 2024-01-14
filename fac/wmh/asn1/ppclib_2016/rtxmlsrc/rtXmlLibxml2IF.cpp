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
#include <libxml/parser.h>
#include <libxml/SAX.h>
#include <libxml/parserInternals.h>
#include "rtxmlsrc/rtSaxCppParserIF.h"
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/OSRTInputStreamIF.h"

#if defined (XML_UNICODE) && defined(XML_UNICODE_WCHAR_T)
#error C SAX does not support XML_UNICODE_WCHAR_T macro at the moment
#endif

#define XML_BUF_SIZE 2048

///////////////////////////////////////////////////
class OSLibxmlXMLReader : public OSXMLReaderClass
{
   DECLARE_XMLBASEIMP
protected:
   OSXMLParserCtxtIF*  mpContext;
   xmlSAXHandler       saxHandler;
   xmlSAXLocatorPtr    locator;
   xmlParserCtxtPtr    currentCtxt;
   OSXMLDefaultHandlerIF* userSaxHandler;
   OSBOOL bFatalError;
   static OSBOOL bManualCleanup;
public:
   /** The default constructor */
   OSLibxmlXMLReader(OSXMLParserCtxtIF* pContext);

   /** The destructor */
   virtual ~OSLibxmlXMLReader();

   virtual int parse ();

   virtual int parse (OSRTInputStreamIF& source);

   virtual int parse (const char* const pBuffer,
                      size_t bufSize);

   virtual int parse (const char* const systemId);

   void setSaxHandler(OSXMLDefaultHandlerIF* const handler);

   inline static void setThreadSafety (OSBOOL value = TRUE)
      { bManualCleanup = value; }

protected:
   // interface functions for callbacks
   static void startElementCallback
      (void *userData, const xmlChar* name, const xmlChar** atts);
   static void endElementCallback(void *userData, const xmlChar* name);
   static void charDataCallback(void *userData, const xmlChar* s, int len);

   static void errorCallback (void *ctx, const char *msg, ...);
   static void setDocumentLocator (void *ctx, xmlSAXLocatorPtr loc);

   static void cdataBlockCallback
      (void *userData, const xmlChar *value, int len);
};

OSBOOL OSLibxmlXMLReader::bManualCleanup = FALSE;

/**
 * error:
 * @ctxt:  An XML parser context
 * @msg:  the message to display/transmit
 * @...:  extra parameters for the message display
 *
 * Display and format a error messages, gives file, line, position and
 * extra parameters.
 */
void OSLibxmlXMLReader::errorCallback (void *ctx, const char *msg, ...)
{
   OSLibxmlXMLReader* reader = (OSLibxmlXMLReader*) ctx;
   va_list args;
   char str [1256];

   va_start(args, msg);
   os_vsnprintf(str, 1256, msg, args);
   va_end(args);

   if (reader != 0 && reader->locator != 0 && reader->currentCtxt != 0) {
      int column = reader->locator->getColumnNumber (reader->currentCtxt);
      int line = reader->locator->getLineNumber (reader->currentCtxt);
      size_t l = OSCRTLSTRLEN (str);
      if (str [l - 1] == '\n') l--; /* remove trailing '\n' */
      os_snprintf ((((char*)str) + l), (1256 - l),
            ", line = %i, column = %i\n", line, column);
   }

   LOG_RTERRNEW (reader->mpContext->getContext (), RTERR_XMLPARSE);
   rtxErrAddStrParm (reader->mpContext->getContext (), (const char*)str);
   reader->bFatalError = TRUE;
}

void OSLibxmlXMLReader::setDocumentLocator (void *ctx, xmlSAXLocatorPtr loc)
{
   OSLibxmlXMLReader* reader = (OSLibxmlXMLReader*) ctx;
   if (reader != 0) {
      reader->locator = loc;
   }
}

OSLibxmlXMLReader::OSLibxmlXMLReader(OSXMLParserCtxtIF* pContext) :
   mpContext (pContext), bFatalError (FALSE)
{
   OSCRTLMEMSET (&saxHandler, 0, sizeof (saxHandler));

   saxHandler.startElement = (startElementSAXFunc)startElementCallback;
   saxHandler.endElement   = (endElementSAXFunc)endElementCallback;
   saxHandler.characters   = (charactersSAXFunc)charDataCallback;
   saxHandler.error        = (errorSAXFunc)errorCallback;
   saxHandler.setDocumentLocator = (setDocumentLocatorSAXFunc)setDocumentLocator;
   saxHandler.cdataBlock   = (cdataBlockSAXFunc)cdataBlockCallback;

   userSaxHandler = 0;

   /* xmlInitParser should be called by internal Libxml functions,
      but sometimes it does not happen. But anyway, we need to call
      xmlCleanupParser, but if xmlInitParser was not called,
      xmlCleanupParser will do nothing.
      Potential problem - multiple simultaneous calls to xmlInitParser
      may cause problem in multithreaded environment. */
   xmlInitParser();

   /* needs for substitute &amp to '&' in attr values */
   xmlSubstituteEntitiesDefault(1);
}

OSLibxmlXMLReader::~OSLibxmlXMLReader()
{
   /* Cleanup parser. Will clean all internally used memory in Libxml2 */
   xmlCleanupParser();
}

int OSLibxmlXMLReader::parse ()
{
   OSRTInputStreamPtr in (mpContext->createInputStream ());
   return parse (*in);
}

int OSLibxmlXMLReader::parse (OSRTInputStreamIF& source)
{
   xmlParserCtxtPtr xmlCtxt = 0;
   OSCTXT* pctxt = source.getCtxtPtr();
   OSUINT32 chunkLen, i, j, len;
   OSINT32  stat = 0, xstat = 0;
   OSBOOL   terminate = FALSE;

   /* Init read buffer. We use the user data field in the context
      to preserve data between invocations. */

   OSRTBuffer* pXmlBuffer;
   if (0 == pctxt->pUserData) {
      pXmlBuffer = rtxMemAllocTypeZ (pctxt, OSRTBuffer);
      if (pXmlBuffer == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);
      pXmlBuffer->data = (OSOCTET*) rtxMemAlloc (pctxt, XML_BUF_SIZE);
      if (pXmlBuffer->data == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);
      pXmlBuffer->size = XML_BUF_SIZE;
      pXmlBuffer->dynamic = TRUE;
      pctxt->pUserData = (void*) pXmlBuffer;
   }
   else
      pXmlBuffer = (OSRTBuffer*) pctxt->pUserData;

   for (;;) {
      /* Read a chunk of the XML document into memory */
      stat = source.read
         ((OSOCTET*)(pXmlBuffer->data + pXmlBuffer->byteIndex),
          pXmlBuffer->size - pXmlBuffer->byteIndex);

      if (stat < 0) break;

      len = (OSUINT32)(stat + pXmlBuffer->byteIndex);
      if (len == 0) {
         if (xmlCtxt != 0) {
            /* Terminate parser */
            xstat = xmlParseChunk
               (xmlCtxt, (char*)pXmlBuffer->data, 0, TRUE);
         }
         break;
      }
      else {
         RTDIAGCHARS (pctxt, (const char*)pXmlBuffer->data, len);
      }

      /* Scan buffer to see if it contains another start-of-document
         header.  If yes, adjust buffer parameters. */
      terminate = FALSE;

      /* check for Unicode BOM */
      j = 0;
      if (len >= 3 &&
         !memcmp(pXmlBuffer->data, "\xef\xbb\xbf", 3)) {
            j = 3;
      }
      else if (len >= 4 &&
         (!memcmp(pXmlBuffer->data, "\x00\x00\xfe\xff", 4) ||
          !memcmp(pXmlBuffer->data, "\xff\xfe\x00\x00", 4))) {
             j = 4;
      }
      else if (len >= 2 &&
         (!memcmp(pXmlBuffer->data, "\xfe\xff", 2) ||
          !memcmp(pXmlBuffer->data, "\xff\xfe", 2))) {
            j = 2;
      }

      /* Skip BOM and '<?' if document is at start of buffer */
      /* Do not want to skip BOM if not at start of document because
         it can actually represent a Unicode character in other places */
      i = (len >= j + 2 && pXmlBuffer->data[j + 0] == '<' &&
           pXmlBuffer->data[j + 1] == '?') ? j + 2 : 0;

      for ( ; i < len; i++) {
         if (pXmlBuffer->data[i] == '<') {
            if ((i + 1) < len) {
               if (pXmlBuffer->data[i+1] == '?') {
                  /* Start of a new document */
                  terminate = TRUE;
                  break;
               }
            }
            else {
               /* This is tricky.  < occurs as last character in current
                  chunk.  We need to get next chunk to determine what to
                  do with it.  Adjust buffer arguments to not consider
                  this character in current parse and move it up to the
                  front of the buffer later. */
               break;
            }
         }
         /* Test for BOM followed by start of new document.  As above, if
         start of character sequence found, break and get next chunk. */
         else if (!memcmp(pXmlBuffer->data + i, "\xef\xbb\xbf<?",
                  OSRTMIN(len - i, 5))) {
            if (OSRTMIN(len - i, 5) == 5)
               terminate = TRUE;
			   break;
         }
         else if ((!memcmp(pXmlBuffer->data, "\x00\x00\xfe\xff<?", 6) ||
                   !memcmp(pXmlBuffer->data, "\xff\xfe\x00\x00<?", 6))) {
            if (OSRTMIN(len - i, 6) == 6)
               terminate = TRUE;
            break;
         }
         else if ((!memcmp(pXmlBuffer->data, "\xfe\xff<?", 4) ||
                   !memcmp(pXmlBuffer->data, "\xff\xfe<?", 4))) {
            if (OSRTMIN(len - i, 4) == 4)
               terminate = TRUE;
            break;
         }
      }
      chunkLen = i;

      /* Parse chunk */
      RTDIAG2 (pctxt, "ParseChunk: len = %d\n", chunkLen);
      RTDIAGCHARS (pctxt, (const char*)pXmlBuffer->data, chunkLen);
      if (0 == xmlCtxt) {
         /* Note: this will parse the first chunk or data */
         xmlCtxt = xmlCreatePushParserCtxt
            (&saxHandler, this, (char*)pXmlBuffer->data, chunkLen, "");

         if (xmlCtxt == 0) return LOG_RTERRNEW (pctxt, RTERR_XMLPARSE);
         currentCtxt = xmlCtxt;
         if (terminate) {
            xstat = xmlParseChunk
               (xmlCtxt, (char*)pXmlBuffer->data, 0, terminate);
         }
      }
      else {
         xstat = xmlParseChunk
            (xmlCtxt, (char*)pXmlBuffer->data, chunkLen, terminate);
      }

      /* If start of another document found, move data for next doc
         to beginning of buffer */
      if (chunkLen < len) {
         pXmlBuffer->byteIndex = len - chunkLen;
         rtxStrncpy ((char*)pXmlBuffer->data,
                     pXmlBuffer->size,
                     (const char*)&pXmlBuffer->data[chunkLen],
                     pXmlBuffer->byteIndex);
         if (terminate) break;
      }
      else pXmlBuffer->byteIndex = 0;
   }

   if (xmlCtxt != 0) {
      xmlFreeParserCtxt (xmlCtxt);
   }
   currentCtxt = 0;

   if (stat < 0) { /* run-time error */
      return LOG_RTERR (pctxt, stat);
   }
   else if (xstat != 0) {
      LOG_RTERRNEW (pctxt, RTERR_XMLPARSE);
      // rtxErrAddStrParm (pctxt, LSTRX (pctxt, readerImpl->errorString));
      return RTERR_XMLPARSE;
   }

   return 0;
}

int OSLibxmlXMLReader::parse (const char* const pBuffer, size_t bufSize)
{
   int ret = 0;

   currentCtxt = xmlCreateMemoryParserCtxt(pBuffer, (int)bufSize);
   if (currentCtxt == NULL) ret = -1;
   else {
      xmlSAXHandlerPtr oldsax;
      oldsax = currentCtxt->sax;
      currentCtxt->sax = &saxHandler;
      currentCtxt->userData = this;

      xmlParseDocument(currentCtxt);

      if (currentCtxt->wellFormed)
      ret = 0;
      else {
        if (currentCtxt->errNo != 0)
           ret = currentCtxt->errNo;
      else
         ret = -1;
      }
      currentCtxt->sax = oldsax;
      xmlFreeParserCtxt(currentCtxt);
   }
   if (ret) {
      return LOG_RTERRNEW (mpContext->getContext (), RTERR_NOTINIT);
      //mpContext->throwSaxException ((const OSUTF8CHAR*)
      //   "Can't create parser memory context");
   }
   return (bFatalError) ? RTERR_XMLPARSE : 0;
}

int OSLibxmlXMLReader::parse (const char* const systemId)
{
   int ret = 0;

   currentCtxt = xmlCreateFileParserCtxt (systemId);
   if (currentCtxt == NULL) ret = -1;
   else {
      xmlSAXHandlerPtr oldsax;
      oldsax = currentCtxt->sax;
      currentCtxt->sax = &saxHandler;
      currentCtxt->userData = this;

      xmlParseDocument(currentCtxt);

      if (currentCtxt->wellFormed)
      ret = 0;
      else {
        if (currentCtxt->errNo != 0)
           ret = currentCtxt->errNo;
      else
         ret = -1;
      }
      currentCtxt->sax = oldsax;
      xmlFreeParserCtxt(currentCtxt);
   }
   if (ret) {
      return LOG_RTERRNEW (mpContext->getContext (), RTERR_NOTINIT);
      //mpContext->throwSaxException ((const OSUTF8CHAR*)
      //   "Can't create parser file context");
   }
   return (bFatalError) ? RTERR_XMLPARSE : 0;
}

void OSLibxmlXMLReader::setSaxHandler(OSXMLDefaultHandlerIF* const handler)
{
   userSaxHandler = handler;
   userSaxHandler->resetErrorInfo ();
}


void
OSLibxmlXMLReader::startElementCallback(void *userData, const xmlChar* name,
   const xmlChar** atts)
{
   OSLibxmlXMLReader* reader = (OSLibxmlXMLReader*) userData;
   OSCTXT* pctxt = reader->mpContext->getContext ();
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo != 0) {
      ctxtInfo->mbCdataProcessed = FALSE;

      ctxtInfo->mSaxLevel++;
      if (ctxtInfo->mSkipLevel < 0) {
         // fatal error, stop parser
         xmlStopParser (reader->currentCtxt);
         return;
      }
      else if (ctxtInfo->mSkipLevel > 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel)
         return;
   }

   if (reader->userSaxHandler) {
      const OSUTF8CHAR* localname = reader->mpContext->parseQName
         ((const OSUTF8CHAR*)name);
      reader->userSaxHandler->startElement((const OSUTF8CHAR*)name,
         localname, (const OSUTF8CHAR*) name,
         (const OSUTF8CHAR**)atts);
   }
}


void
OSLibxmlXMLReader::endElementCallback(void *userData, const xmlChar* name)
{
   OSLibxmlXMLReader* reader = (OSLibxmlXMLReader*) userData;
   OSCTXT* pctxt = reader->mpContext->getContext ();
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
   OSBOOL bSkip = FALSE;

   if (ctxtInfo) {
      if (ctxtInfo->mSkipLevel != 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel) {
         bSkip = TRUE;
      }
   }

   if (!bSkip && reader->userSaxHandler) {
      const OSUTF8CHAR* localname = reader->mpContext->parseQName
         ((const OSUTF8CHAR*)name);
      reader->userSaxHandler->endElement((const OSUTF8CHAR*)name,
         localname, (const OSUTF8CHAR*)name);
   }

   if (ctxtInfo != 0) {
      if (ctxtInfo->mSkipLevel > 0 && ctxtInfo->mSaxLevel == ctxtInfo->mSkipLevel)
         ctxtInfo->mSkipLevel = 0; /* reset skipLevel */
      ctxtInfo->mSaxLevel--;

      ctxtInfo->mbCdataProcessed = FALSE;
   }
}

void
OSLibxmlXMLReader::charDataCallback(void *userData, const xmlChar* s, int len)
{
   OSLibxmlXMLReader* reader = (OSLibxmlXMLReader*) userData;
   OSCTXT* pctxt = reader->mpContext->getContext ();
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo) {
      if (ctxtInfo->mSkipLevel != 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel)
         return;
   }
   if (reader->userSaxHandler) {
      reader->userSaxHandler->characters ((const OSUTF8CHAR*)s, len);
   }
}

void OSLibxmlXMLReader::cdataBlockCallback
   (void *userData, const xmlChar *value, int len)
{
   OSLibxmlXMLReader* reader = (OSLibxmlXMLReader*) userData;
   OSCTXT* pctxt = reader->mpContext->getContext ();
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo != 0) {
      if (ctxtInfo->mSkipLevel != 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel)
         return;
      ctxtInfo->mbCdataProcessed = TRUE;
   }
   if (reader->userSaxHandler) {
      reader->userSaxHandler->characters ((const OSUTF8CHAR*)value, len);
   }
}

OSXMLReaderClass* rtSaxCppCreateXmlReader
(OSXMLParserCtxtIF* pContext, OSXMLDefaultHandlerIF* pSaxHandler)
{

   OSLibxmlXMLReader *reader = new OSLibxmlXMLReader (pContext);
   reader->setSaxHandler (pSaxHandler);
   return reader;
}

int rtSaxCppEnableThreadSafety ()
{
   xmlInitParser();
   OSLibxmlXMLReader::setThreadSafety();
   return 0;
}

void rtSaxCppLockXmlLibrary ()
{
}

void rtSaxCppUnlockXmlLibrary ()
{
}
