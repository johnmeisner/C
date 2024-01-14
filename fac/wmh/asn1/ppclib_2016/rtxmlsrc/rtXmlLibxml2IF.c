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

#include <libxml/parser.h>
#include <libxml/SAX.h>
#include <libxml/parserInternals.h>
#include "rtxmlsrc/rtSaxCParser.h"
#include "rtxmlsrc/rtSaxDefs.h"
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxStream.h"
#include "rtxsrc/rtxErrCodes.h"

#if defined (XML_UNICODE) && defined(XML_UNICODE_WCHAR_T)
#error C SAX currently does not support XML_UNICODE_WCHAR_T macro
#endif

#define XML_BUF_SIZE 2048

static OSBOOL g_manual_cleanup = FALSE;

typedef struct XMLReaderImpl {
   OSXMLREADER reader;
   void* userData;
   xmlSAXHandler saxHandler;
   xmlParserCtxtPtr xmlCtxt;
   xmlSAXLocatorPtr locator;
   char errorString [128];
   CSAX_StartElementHandler  pStartElementProc;
   CSAX_EndElementHandler    pEndElementProc;
   CSAX_CharacterDataHandler pCharactersProc;
} XMLReaderImpl;

int rtSaxCReleaseReader (OSXMLREADER* pReader)
{
   XMLReaderImpl* readerImpl = (XMLReaderImpl*)(void*)pReader;

   if (pReader == 0 || pReader->pctxt == 0)
      return RTERR_NOTINIT;

   rtxMemFreePtr (pReader->pctxt, readerImpl);

   /* Cleanup parser. Will clean all internally used memory in Libxml2 */
   if (!g_manual_cleanup)
      xmlCleanupParser();
   return 0;
}

int rtSaxCEnableThreadSafety ()
{
   g_manual_cleanup = TRUE;
   xmlInitParser();
   return 0;
}

int rtSaxCParse (OSXMLREADER* pReader)
{
   int stat = 0, xstat = 0;

#ifndef _NO_STREAM
   struct OSRTSTREAM* pStream;
#endif /* _NO_STREAM */

   XMLReaderImpl* readerImpl = (XMLReaderImpl*)(void*)pReader;
   OSCTXT* pctxt;
   xmlParserCtxtPtr xmlCtxt = 0;

   if (pReader == 0 || pReader->pctxt == 0)
      return RTERR_NOTINIT;

   pctxt = pReader->pctxt;
   rtxErrReset (pctxt);

#ifndef _NO_STREAM
   pStream = pctxt->pStream;

   if (pStream == 0)
#endif /* _NO_STREAM */
   {
      xmlSAXHandlerPtr oldSax;

      xmlCtxt = xmlCreateMemoryParserCtxt ((char*)OSRTBUFPTR(pctxt),
                                           (int)pctxt->buffer.size);
      if (xmlCtxt == 0) return LOG_RTERRNEW (pctxt, RTERR_XMLPARSE);
      readerImpl->xmlCtxt = xmlCtxt;

      oldSax = xmlCtxt->sax;
      xmlCtxt->sax = &readerImpl->saxHandler;
      xmlCtxt->userData = readerImpl;

      xstat = xmlParseDocument (xmlCtxt);

      xmlCtxt->sax = oldSax;
      xmlFreeParserCtxt (xmlCtxt);
      xmlCtxt = 0;
   }
#ifndef _NO_STREAM
   else {
      /* AT 05/29/2007 -- offset removed to eliminate VC++ Level 4 warning. */
      OSUINT32 chunkLen, i, j, len /*, offset = 0 */;
      OSBOOL   terminate = FALSE;
      OSRTBuffer* pXmlBuffer;

      /* Init read buffer. We use the user data field in the context
         to preserve data between invocations. */

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
         stat = rtxStreamRead
            (pctxt, (OSOCTET*)(pXmlBuffer->data + pXmlBuffer->byteIndex),
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
            RTDIAGCHARS (pctxt, (char*)pXmlBuffer->data, len);
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
         RTDIAGCHARS (pctxt, (char*)   pXmlBuffer->data, chunkLen);
         if (0 == xmlCtxt) {
            xmlCtxt = xmlCreatePushParserCtxt
               (&readerImpl->saxHandler, readerImpl,
                (char*)pXmlBuffer->data, chunkLen, "");

            if (xmlCtxt == 0) return LOG_RTERRNEW (pctxt, RTERR_XMLPARSE);

            readerImpl->xmlCtxt = xmlCtxt;

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
                        (char*)&pXmlBuffer->data[chunkLen],
                        pXmlBuffer->byteIndex);
            if (terminate) {
               RTDIAG1 (pctxt, "Start of new document detected..\n");
               break;
            }
         }
         else pXmlBuffer->byteIndex = 0;
      }
   }
#endif /* _NO_STREAM */

   if (xmlCtxt != 0) {
      xmlFreeParserCtxt (xmlCtxt);
   }

   if (stat < 0) { /* run-time error */
      return LOG_RTERR (pctxt, stat);
   }
   else if (xstat != 0) {
      LOG_RTERRNEW (pctxt, RTERR_XMLPARSE);
      rtxErrAddStrParm (pctxt, readerImpl->errorString);
      return RTERR_XMLPARSE;
   }

   return 0;
}

static void setDocumentLocatorCallback (void *ctx, xmlSAXLocatorPtr loc)
{
   XMLReaderImpl* reader = (XMLReaderImpl*) ctx;
   if (reader != 0) {
      reader->locator = loc;
   }
}

/**
 * error:
 * @ctxt:  An XML parser context
 * @msg:  the message to display/transmit
 * @...:  extra parameters for the message display
 *
 * Display and format a error messages, gives file, line, position and
 * extra parameters.
 */
static void errorCallback (void *ctx, const char *msg, ...)
{
   XMLReaderImpl* reader = (XMLReaderImpl*) ctx;
   va_list args;
   char str [1256];

   va_start(args, msg);
   os_vsnprintf(str, 1256, msg, args);
   va_end(args);

   if (reader != 0 && reader->locator != 0 && reader->xmlCtxt != 0) {
      char numbuf[32];
      int column = reader->locator->getColumnNumber (reader->xmlCtxt);
      int line = reader->locator->getLineNumber (reader->xmlCtxt);
      int l = (int)OSCRTLSTRLEN (str);
      if (str [l - 1] == '\n') str[l-1] = '\0'; /* remove trailing '\n' */
      rtxStrcat (str, sizeof(str), ", line = ");
      rtxIntToCharStr (line, numbuf, sizeof(numbuf), 0);
      rtxStrcat (str, sizeof(str), numbuf);
      rtxStrcat (str, sizeof(str), ", column = ");
      rtxIntToCharStr (column, numbuf, sizeof(numbuf), 0);
      rtxStrcat (str, sizeof(str), numbuf);
      rtxStrcat (str, sizeof(str), "\n");
   }
   rtxStrncpy (reader->errorString, 128, str, 127);
}

static void cdataBlockCallback (void *userData,
				const xmlChar *value,
				int len)
{
   XMLReaderImpl* reader = (XMLReaderImpl*)userData;
   OSCTXT* pctxt = reader->reader.pctxt;
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo) {
      if (ctxtInfo->mSkipLevel != 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel)
         return;
      ctxtInfo->mbCdataProcessed = TRUE;
   }
   if (reader->pCharactersProc != 0) {
      reader->pCharactersProc (reader->userData, (const OSUTF8CHAR*)value, len);
   }
}

static const OSUTF8CHAR* parseQName (const OSUTF8CHAR* qname)
{
   register int i = 0;
   const OSUTF8CHAR* lname = qname;

   while ((qname[i] != 0) && (qname[i] != ':'))
      i++;

   if (qname[i] != 0) lname += i + 1;

   return (lname);
}

static void startElementCallback(void *userData, const xmlChar* name,
   const xmlChar** atts)
{
   XMLReaderImpl* reader = (XMLReaderImpl*) userData;
   OSCTXT* pctxt = reader->reader.pctxt;
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo) {
      ctxtInfo->mbCdataProcessed = FALSE;

      ctxtInfo->mSaxLevel++;
      if (ctxtInfo->mSkipLevel < 0) {
         /* fatal error, stop parser */
         xmlStopParser (reader->xmlCtxt);
         return;
      }
      else if (ctxtInfo->mSkipLevel > 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel)
         return;
   }
   if (reader->pStartElementProc != 0) {
      const OSUTF8CHAR* localname = parseQName ((const OSUTF8CHAR*)name);
      reader->pStartElementProc (reader->userData,
         localname, (const OSUTF8CHAR*)name, (const OSUTF8CHAR**)atts);
   }
}


static void endElementCallback(void *userData, const xmlChar* name)
{
   XMLReaderImpl* reader = (XMLReaderImpl*) userData;
   OSCTXT* pctxt = reader->reader.pctxt;
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
   OSBOOL bSkip = FALSE;

   if (ctxtInfo) {
      if (ctxtInfo->mSkipLevel != 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel) {
         bSkip = TRUE;
      }
   }
   if (!bSkip && reader->pEndElementProc != 0) {
      const OSUTF8CHAR* localname = parseQName ((const OSUTF8CHAR*)name);
      reader->pEndElementProc (reader->userData,
         localname, (const OSUTF8CHAR*)name);
   }
   if (ctxtInfo) {
      if (ctxtInfo->mSkipLevel > 0 && ctxtInfo->mSaxLevel == ctxtInfo->mSkipLevel)
         ctxtInfo->mSkipLevel = 0; /* reset skipLevel */
      ctxtInfo->mSaxLevel--;

      ctxtInfo->mbCdataProcessed = FALSE;
   }
}

static void charDataCallback(void *userData, const xmlChar* s, int len)
{
   XMLReaderImpl* reader = (XMLReaderImpl*) userData;
   OSCTXT* pctxt = reader->reader.pctxt;
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo) {
      if (ctxtInfo->mSkipLevel != 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel)
         return;
   }
   if (reader->pCharactersProc != 0) {
      reader->pCharactersProc (reader->userData, (const OSUTF8CHAR*)s, len);
   }
}

OSXMLREADER* rtSaxCCreateXmlReader (OSCTXT* pctxt, void* pSaxHandlerData,
                                    CSAX_StartElementHandler  pStartElementProc,
                                    CSAX_EndElementHandler    pEndElementProc,
                                    CSAX_CharacterDataHandler pCharactersProc)
{
   XMLReaderImpl* reader;

   reader = (XMLReaderImpl*) rtxMemAlloc (pctxt, sizeof (XMLReaderImpl));
   if (reader == 0) return 0;
   OSCRTLMEMSET (reader, 0, sizeof (XMLReaderImpl));

   reader->pStartElementProc = pStartElementProc;
   reader->pEndElementProc   = pEndElementProc;
   reader->pCharactersProc   = pCharactersProc;
   reader->saxHandler.startElement = startElementCallback;
   reader->saxHandler.endElement   = endElementCallback;
   reader->saxHandler.characters   = charDataCallback;
   reader->saxHandler.error        = errorCallback;
   reader->saxHandler.cdataBlock   = cdataBlockCallback;
   reader->saxHandler.setDocumentLocator = setDocumentLocatorCallback;
   reader->userData = pSaxHandlerData;
   reader->reader.pctxt = pctxt;

   /* xmlInitParser should be called by internal Libxml functions,
      but sometimes it does not happen. But anyway, we need to call
      xmlCleanupParser, but if xmlInitParser was not called,
      xmlCleanupParser will do nothing.
      Potential problem - multiple simultaneous calls to xmlInitParser
      may cause problem in multithreaded environment. */
   xmlInitParser();

   /* needs for substitute &amp to '&' in attr values */
   xmlSubstituteEntitiesDefault(1);

   return &reader->reader;
}
