/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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

#include "rtxmlsrc/rtSaxCppParserIF.h"
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxStream.h"
#include "rtxsrc/rtxStreamBuffered.h"
#include "rtxsrc/OSRTInputStreamIF.h"
#include "expatsrc/expat.h"

#if defined (XML_UNICODE) && defined(XML_UNICODE_WCHAR_T)
#error C SAX does not support XML_UNICODE_WCHAR_T macro at the moment
#endif

#define XML_BUF_SIZE 2048

///////////////////////////////////////////////////
class OSExpatXMLReader : public OSXMLReaderClass
{
   DECLARE_XMLBASEIMP
protected:
   OSXMLParserCtxtIF* mpContext;
   XML_Parser mParser;
   OSXMLDefaultHandlerIF* saxHandler;
   int mLevel, mState;
public:
   /** The default constructor */
   OSExpatXMLReader(OSXMLParserCtxtIF* pContext);

   /** The destructor */
   virtual ~OSExpatXMLReader();

   virtual int parse ();

   virtual int parse (OSRTInputStreamIF& source);

   virtual int parse (const char* const pBuffer,
                      size_t bufSize);

   virtual int parse (const char* const systemId);

   void setSaxHandler(OSXMLDefaultHandlerIF* const handler);
protected:
   // interface functions for callbacks
   static void startElementCallback(void *userData, const XML_Char* name, const XML_Char** atts);
   static void endElementCallback(void *userData, const XML_Char* name);
   static void charDataCallback(void *userData, const XML_Char* s, int len);

   static void startCdataSectionHandler (void *userData);
   static void endCdataSectionHandler (void *userData);
};

/////////////////////////////////////////////////////////
/* Returns error string (extended) */
static const OSUTF8CHAR*
EXML_ErrorString(XML_Parser parser, enum XML_Error code, XML_LChar* buf, int bufSize)
{
   const XML_LChar *err = XML_ErrorString (code);
   int i, len;
   char str[256];

   if (err == 0 || buf == 0 || bufSize == 0) {
      return (const OSUTF8CHAR*)err;
   }
   for (len = 0; err[len] != 0; len++)
      ;
   os_snprintf (str, 256, ", line = %i, column = %i",
      XML_GetCurrentLineNumber (parser),
      XML_GetCurrentColumnNumber (parser));

   if (bufSize + 1 < len)
      return (const OSUTF8CHAR*)err;

   OSCRTLSAFEMEMCPY (buf, (size_t)bufSize, err, sizeof (XML_LChar)*len);

   for (i = 0; (i + len + 1) < bufSize && str[i] != 0; i++) {
      buf[i + len] = (XML_LChar)str[i];
   }
   buf[i + len] = 0;
   return (const OSUTF8CHAR*)buf;
}

OSExpatXMLReader::OSExpatXMLReader(OSXMLParserCtxtIF* pContext) :
   mpContext (pContext), mLevel (0), mState (0)
{
   mParser = ::XML_ParserCreate (NULL);
   if (mParser == 0)
      return;
   saxHandler = 0;

   XML_SetCdataSectionHandler(mParser,
      (XML_StartCdataSectionHandler)startCdataSectionHandler,
      (XML_EndCdataSectionHandler)endCdataSectionHandler);
}

OSExpatXMLReader::~OSExpatXMLReader()
{
   if (mParser)
      ::XML_ParserFree (mParser);
}

int OSExpatXMLReader::parse ()
{
   if (mParser == 0)
      return LOG_RTERRNEW (mpContext->getContext(), RTERR_NOTINIT);

   OSRTInputStreamPtr in (mpContext->createInputStream());

   return parse (*in);
}

int OSExpatXMLReader::parse (OSRTInputStreamIF& source)
{
   long len;
   enum XML_Status stat = XML_STATUS_OK;

   if (mParser == 0)
      return LOG_RTERRNEW (mpContext->getContext(), RTERR_NOTINIT);

   mLevel = mState = 0;

   do {
      void* pbuf;
      XML_Bool isFinal;

      pbuf = ::XML_GetBuffer (mParser, XML_BUF_SIZE);

      len = source.read ((OSOCTET*)pbuf, XML_BUF_SIZE);
      if (len < 0)
         break;
      isFinal = !len;

      if ((stat = ::XML_ParseBuffer (mParser, len, isFinal)) == 0)
         break;

      if (mState == OS_SAX_FINAL_STATE) {
         /* if parsing is finished, but buffer is not empty we need
          * to find the beginning of the next XML message and set
          * this piece of data as pre-read buffer for BufferedStream.*/
         XML_ParsingStatus status;

         XML_GetParsingStatus(mParser, &status);
         if (status.parsing == XML_SUSPENDED) {
            int offset, lenn;
            OSCTXT* pctxt = source.getCtxtPtr ();

            /* Get buffer pointer, offset and length of remaining data.
               Note, that patching of Expat is necessary to fix two problems:
               1) even if parser is stopped by XML_StopParser, it will return
                  error "junk after end-of-document" if buffer is not empty;
               2) XML_GetInputContext worked only if macro XML_CONTEXT_BYTES
                  was defined. But it could work even without it. */
            const char * _pbuf =
               XML_GetInputContext(mParser, &offset, &lenn);

            if (offset > 0 && lenn - offset > 0) {
               int stat = 0;
               const OSUTF8CHAR* prereadBuf = (const OSUTF8CHAR*)_pbuf + offset;
               size_t prereadBufLen = (size_t)(lenn - offset), i;

               /* check, is the buffer just whitespaces or not. If yes,
                  discard it */
               for (i = 0; i < prereadBufLen; i++) {
                  if (!OS_ISSPACE (prereadBuf[i])) {

                     if (OSRTSTREAM_ID (pctxt) != OSRTSTRMID_DIRECTBUF) {
                        stat = rtxStreamBufferedCreate (pctxt,
                           OSRTSTRMCM_RESTORE_UNDERLAYING_AFTER_RESET);
                        if (stat < 0) len = stat;
                     }
                     if (stat == 0) {
                        stat = rtxStreamBufferedSetPreReadBuf (pctxt,
                          (const OSOCTET*)prereadBuf + i, prereadBufLen - i);
                        if (stat < 0) len = stat;
                     }
                     break;
                  }
               }
            }
            stat = XML_STATUS_OK;
         }
         break;
      }
   } while (len > 0);

   if (len < 0) { /* ASN.1 RT error */
      return LOG_RTERR (mpContext->getContext (), len);
   }
   else if (stat != XML_STATUS_OK) {
      XML_LChar str[256];

      ::EXML_ErrorString(mParser,
         ::XML_GetErrorCode (mParser), str, sizeof(str)/sizeof(str[0]));

      int stat = LOG_RTERRNEW (mpContext->getContext (), RTERR_XMLPARSE);
      rtxErrAddStrParm (mpContext->getContext (), (const char*)str);
      return stat;
   }

   return 0;
}

int OSExpatXMLReader::parse (const char* const pBuffer, size_t bufSize)
{
   mLevel = mState = 0;

   if (mParser == 0) return LOG_RTERRNEW (mpContext->getContext (), RTERR_NOTINIT);

   if (::XML_Parse (mParser, pBuffer, (int)bufSize, 1) == 0) { // is error occurred?
      XML_LChar str[256];
      ::EXML_ErrorString(mParser,
         ::XML_GetErrorCode (mParser), str, sizeof(str)/sizeof(str[0]));

      int stat = LOG_RTERRNEW (mpContext->getContext (), RTERR_XMLPARSE);
      rtxErrAddStrParm (mpContext->getContext (), (const char*)str);
      return stat;
   }
   return 0;
}

int OSExpatXMLReader::parse (const char* const systemId)
{
   mLevel = mState = 0;

   if (mParser == 0)
      return LOG_RTERRNEW (mpContext->getContext(), RTERR_NOTINIT);

   OSRTInputStreamPtr in (mpContext->createFileInputStream (systemId));

   return parse (*in);
}

void OSExpatXMLReader::setSaxHandler(OSXMLDefaultHandlerIF* const handler)
{
   if (mParser == 0) return;

   saxHandler = handler;
   ::XML_SetUserData(mParser, this);
   ::XML_SetElementHandler(mParser,
      (XML_StartElementHandler)startElementCallback,
      (XML_EndElementHandler)endElementCallback);
   ::XML_SetCharacterDataHandler(mParser,
      (XML_CharacterDataHandler)charDataCallback);
   saxHandler->resetErrorInfo ();
}

void
OSExpatXMLReader::startElementCallback(void *userData, const XML_Char* name,
   const XML_Char** atts)
{
   OSExpatXMLReader* reader = (OSExpatXMLReader*) userData;
   OSCTXT* pctxt = reader->mpContext->getContext ();
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   reader->mLevel++;
   if (ctxtInfo != 0) {
      ctxtInfo->mbCdataProcessed = FALSE;

      ctxtInfo->mSaxLevel++;
      if (ctxtInfo->mSkipLevel < 0) {
         // fatal error, stop parser
         XML_StopParser(reader->mParser, TRUE);
         return;
      }
      else if (ctxtInfo->mSkipLevel > 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel)
         return;
   }

   if (reader->saxHandler) {
      const OSUTF8CHAR* localname =
         reader->mpContext->parseQName ((const OSUTF8CHAR*)name);
      reader->saxHandler->startElement((const OSUTF8CHAR*)name, localname,
         (const OSUTF8CHAR*)name, (const OSUTF8CHAR**)atts);
   }
}


void
OSExpatXMLReader::endElementCallback(void *userData, const XML_Char* name)
{
   OSExpatXMLReader* reader = (OSExpatXMLReader*) userData;
   OSCTXT* pctxt = reader->mpContext->getContext ();
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
   OSBOOL bSkip = FALSE;

   if (ctxtInfo) {
      if (ctxtInfo->mSkipLevel != 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel) {
         bSkip = TRUE;
      }
   }

   if (!bSkip && reader->saxHandler) {
      const OSUTF8CHAR* localname =
         reader->mpContext->parseQName ((const OSUTF8CHAR*)name);
      reader->saxHandler->endElement((const OSUTF8CHAR*)name, localname,
         (const OSUTF8CHAR*)name);
   }

   if (ctxtInfo != 0) {
      if (ctxtInfo->mSkipLevel > 0 && ctxtInfo->mSaxLevel == ctxtInfo->mSkipLevel)
         ctxtInfo->mSkipLevel = 0; /* reset skipLevel */
      ctxtInfo->mSaxLevel--;

      ctxtInfo->mbCdataProcessed = FALSE;
   }

   // Set mState to a final state, to indicate parsing is finished

   if (--reader->mLevel == 0) {
      XML_StopParser(reader->mParser, TRUE);
      reader->mState = OS_SAX_FINAL_STATE;
   }
}


void
OSExpatXMLReader::charDataCallback(void *userData, const XML_Char* s, int len)
{
   OSExpatXMLReader* reader = (OSExpatXMLReader*) userData;
   OSCTXT* pctxt = reader->mpContext->getContext ();
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo) {
      if (ctxtInfo->mSkipLevel != 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel)
         return;
   }
   if (reader->saxHandler) {
      reader->saxHandler->characters ((const OSUTF8CHAR*)s, len);
   }
}

void OSExpatXMLReader::startCdataSectionHandler (void *userData)
{
   OSExpatXMLReader* reader = (OSExpatXMLReader*) userData;
   OSCTXT* pctxt = reader->mpContext->getContext ();
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo) {
      if (ctxtInfo->mSkipLevel != 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel)
         return;
      ctxtInfo->mbCdataProcessed = TRUE;
   }
}

void OSExpatXMLReader::endCdataSectionHandler (void * /*userData*/)
{
}


OSXMLReaderClass* rtSaxCppCreateXmlReader
(OSXMLParserCtxtIF* pContext, OSXMLDefaultHandlerIF* pSaxHandler)
{

   OSExpatXMLReader *reader = new OSExpatXMLReader (pContext);
   reader->setSaxHandler (pSaxHandler);
   return reader;
}

int rtSaxCppEnableThreadSafety ()
{
   return -1; // not supported
}

void rtSaxCppLockXmlLibrary ()
{
}

void rtSaxCppUnlockXmlLibrary ()
{
}



