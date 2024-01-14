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

/* This is necessary to disable a Visual C++ warning about an unreferenced
   formal parameter passed to rtSaxCEndCdataSectionHandler */
#ifdef _MSC_VER
#pragma warning(disable: 4100)
#endif /* _MSC_VER */

#include "rtxmlsrc/rtSaxCParser.h"
#include "rtxmlsrc/rtSaxDefs.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxStream.h"
#include "rtxsrc/rtxStreamBuffered.h"
#include "rtxsrc/rtxErrCodes.h"
#include "expatsrc/expat.h"

#if defined (XML_UNICODE) && defined(XML_UNICODE_WCHAR_T)
#error C SAX does not support XML_UNICODE_WCHAR_T macro at the moment
#else
#define LSTRX(pctxt,pLStr)   \
OSCRTLSTRCPY ((char*)rtxMemAlloc(pctxt, OSCRTLSTRLEN (pLStr) + 1), (pLStr))
#endif

#define XML_BUF_SIZE 2048

typedef struct XMLReaderImpl {
   OSXMLREADER reader;
   XML_Parser parser;
   void* userData;
   CSAX_StartElementHandler  pStartElementProc;
   CSAX_EndElementHandler    pEndElementProc;
   CSAX_CharacterDataHandler pCharactersProc;
} XMLReaderImpl;

static void XMLCALL rtSaxCStartCdataSectionHandler (void *userData);
static void XMLCALL rtSaxCEndCdataSectionHandler (void *userData);
static void XMLCALL rtSaxCStartElementHandler (void *userData,
                                               const XML_Char *name,
                                               const XML_Char **atts);
static void XMLCALL rtSaxCEndElementHandler (void *userData,
                                             const XML_Char *name);
static void XMLCALL rtSaxCCharacterDataHandler (void *userData,
                                                const XML_Char *s,
                                                int len);

/*
 * Release all allocated resources
 */
int rtSaxCReleaseReader (OSXMLREADER* pReader)
{
   XMLReaderImpl* readerImpl = (XMLReaderImpl*)(void*)pReader;
   XML_Parser parser;

   if (pReader == 0 || pReader->pctxt == 0 || readerImpl->parser == 0)
      return RTERR_NOTINIT;

   parser = readerImpl->parser;
   XML_MemFree (parser, readerImpl);
   XML_ParserFree (parser);
   return 0;
}

/* Returns error string (extended) */
const XML_LChar* EXML_ErrorString
(XML_Parser parser, enum XML_Error code, XML_LChar* buf, int bufSize)
{
   const XML_LChar *err = XML_ErrorString (code);
   char str[256], linebuf[32], colbuf[32];
   size_t i, len, bufSize2;

   if (err == 0 || buf == 0 || bufSize <= 0) {
      return err;
   }
   bufSize2 = (size_t) bufSize;
   for (len = 0; err[len] != 0; len++)
      ;
   rtxIntToCharStr
      (XML_GetCurrentLineNumber(parser), linebuf, sizeof(linebuf), 0);

   rtxIntToCharStr
      (XML_GetCurrentColumnNumber(parser), colbuf, sizeof(colbuf), 0);

   rtxStrcpy (str, sizeof(str), ", line = ");
   rtxStrcat (str, sizeof(str), linebuf);
   rtxStrcat (str, sizeof(str), ", column = ");
   rtxStrcat (str, sizeof(str), colbuf);

   if (bufSize2 + 1 < len)
      return err;

   OSCRTLSAFEMEMCPY (buf, bufSize2, err, sizeof(XML_LChar)*len);

   for (i = 0; (i + len + 1) < bufSize2 && str[i] != 0; i++) {
      buf[i + len] = (XML_LChar)str[i];
   }
   buf[i + len] = 0;
   return buf;
}

/*
 * CDATA section handler. Must set ctxtInfo->mbCdataProcessed to TRUE.
 */
void XMLCALL rtSaxCStartCdataSectionHandler (void *userData)
{
   XMLReaderImpl* reader = (XMLReaderImpl*)userData;
   OSCTXT* pctxt = reader->reader.pctxt;
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo) {
      /* if skip level is set and current level is greater or equal to it
         then skip whole level */
      if (ctxtInfo->mSkipLevel != 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel)
         return;
      ctxtInfo->mbCdataProcessed = TRUE;
   }
}

void XMLCALL rtSaxCEndCdataSectionHandler (void *userData)
{
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

/*
 * Start element handler adaptor.
 */
void XMLCALL rtSaxCStartElementHandler (void *userData,
                                        const XML_Char *name,
                                        const XML_Char **atts)
{
   XMLReaderImpl* reader = (XMLReaderImpl*)userData;
   OSCTXT* pctxt = reader->reader.pctxt;
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo) {
      /* reset cdata flag */
      ctxtInfo->mbCdataProcessed = FALSE;

      /* increment level */
      ctxtInfo->mSaxLevel++;
      if (ctxtInfo->mSkipLevel < 0) {
         /* fatal error, stop parser */
         XML_StopParser(reader->parser, TRUE);
         return;
      }
      /* if skip level is set and current level is greater or equal to it
         then skip whole level */
      else if (ctxtInfo->mSkipLevel > 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel)
         return;
   }
   /* call to target start element handler */
   if (reader->pStartElementProc != 0) {
      const OSUTF8CHAR* localname = parseQName ((const OSUTF8CHAR*)name);
      reader->pStartElementProc (reader->userData,
         (const OSUTF8CHAR*)localname,
         (const OSUTF8CHAR*)name,
         (const OSUTF8CHAR**)atts);
   }
}

/*
 * End element adaptor
 */
void XMLCALL rtSaxCEndElementHandler (void *userData,
                                      const XML_Char *name)
{
   XMLReaderImpl* reader = (XMLReaderImpl*)userData;
   OSCTXT* pctxt = reader->reader.pctxt;
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
   OSSAXHandlerBase* pSaxBase;
   OSBOOL bSkip = FALSE;

   if (ctxtInfo) {
      /* check, are we skipping this level or not */
      if (ctxtInfo->mSkipLevel != 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel) {
         bSkip = TRUE;
      }
   }
   /* call to target end element handler */
   if (!bSkip && reader->pEndElementProc != 0) {
      const OSUTF8CHAR* localname = parseQName ((const OSUTF8CHAR*)name);
      reader->pEndElementProc (reader->userData,
         (const OSUTF8CHAR*)localname,
         (const OSUTF8CHAR*)name);
   }
   if (ctxtInfo) {
      /* if we left the skipping level, reset mSkipLevel */
      if (ctxtInfo->mSkipLevel > 0 && ctxtInfo->mSaxLevel == ctxtInfo->mSkipLevel)
         ctxtInfo->mSkipLevel = 0; /* reset skipLevel */
      ctxtInfo->mSaxLevel--;

      ctxtInfo->mbCdataProcessed = FALSE;
   }

   /* Set mState to a final state, to indicate parsing is finished */
   pSaxBase = (OSSAXHandlerBase*)reader->userData;
   if (pSaxBase != 0 && pSaxBase->mLevel == 0) {
      XML_StopParser(reader->parser, TRUE);
      pSaxBase->mState = OS_SAX_FINAL_STATE;
   }
}

/*
 * character data handler adaptor
 */
void XMLCALL rtSaxCCharacterDataHandler (void *userData,
                                         const XML_Char *s,
                                         int len)
{
   XMLReaderImpl* reader = (XMLReaderImpl*) userData;
   OSCTXT* pctxt = reader->reader.pctxt;
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   /* check are we skipping the level */
   if (ctxtInfo) {
      if (ctxtInfo->mSkipLevel != 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel)
         return;
   }
   /* call to target characters handler */
   if (reader->pCharactersProc != 0) {
      reader->pCharactersProc (reader->userData, (const OSUTF8CHAR*)s, len);
   }
}

int rtSaxCEnableThreadSafety ()
{
   return -1; /* not supported */
}

/*
 * Starts parsing. The XML data being parsed should be set either set as
 * a buffer in OSCTXT (pctxt->buffer) or read from its stream (pctxt->pStream).
 */
int rtSaxCParse (OSXMLREADER* pReader)
{
   long len;
   enum XML_Status stat = XML_STATUS_OK;

#ifndef _NO_STREAM
   struct OSRTSTREAM* pStream;
#endif /* _NO_STREAM */

   XMLReaderImpl* readerImpl = (XMLReaderImpl*)(void*)pReader;
   OSCTXT* pctxt;
   OSSAXHandlerBase* pSaxBase;

   if (pReader == 0 || pReader->pctxt == 0 || readerImpl->parser == 0)
      return RTERR_NOTINIT;

   pctxt = pReader->pctxt;
   rtxErrReset (pctxt);
   pSaxBase = (OSSAXHandlerBase*)readerImpl->userData;

#ifndef _NO_STREAM
   pStream = pctxt->pStream;

   if (pStream == 0)
#endif /* _NO_STREAM */
   {

      /* stream is not set - parse just a buffer */

      stat = XML_Parse (readerImpl->parser, (char*)OSRTBUFPTR(pctxt),
                        (int)pctxt->buffer.size, TRUE);
      return (stat == XML_STATUS_ERROR) ? RTERR_XMLPARSE : 0;
   }
#ifndef _NO_STREAM
   else { /* read from stream and parse */
      do {
         void* pbuf;
         XML_Bool isFinal;

         /* get the buffer to read in */
         pbuf = XML_GetBuffer (readerImpl->parser, XML_BUF_SIZE);

         /* read data to the buffer */
         len = rtxStreamRead (pctxt, (OSOCTET*)pbuf, XML_BUF_SIZE);
         if (len < 0)
            break;
         isFinal = (XML_Bool)(!len);

         /* parse the data in the buffer */
         if ((stat = XML_ParseBuffer (readerImpl->parser, len, isFinal)) == 0)
            break;

         /* the following code is necessary only if it is necessary to
          * decode several XML documents consequently from one stream. */
         if (pSaxBase->mState == OS_SAX_FINAL_STATE) {
            /* if parsing is finished, but buffer is not empty we need
             * to find the beginning of the next XML message and set
             * this piece of data as pre-read buffer for BufferedStream.*/
            XML_ParsingStatus status;

            XML_GetParsingStatus(readerImpl->parser, &status);
            if (status.parsing == XML_SUSPENDED) {
               int offset, lenn;

               /* Get buffer pointer, offset and length of remaining data.
                  Note, that patching of Expat is necessary to fix two problems:
                  1) even if parser is stopped by XML_StopParser, it will return
                     error "junk after end-of-document" if buffer is not empty;
                  2) XML_GetInputContext worked only if macro XML_CONTEXT_BYTES
                     was defined. But it could work even without it. */
               const char * _pbuf =
                  XML_GetInputContext(readerImpl->parser, &offset, &lenn);

               if (offset > 0 && lenn - offset > 0) {
                  int stat2 = 0;
                  const OSUTF8CHAR* prereadBuf = (const OSUTF8CHAR*)_pbuf + offset;
                  size_t prereadBufLen = (size_t)(lenn - offset), i;

                  /* check, is the buffer just whitespaces or not. If yes,
                     discard it */
                  for (i = 0; i < prereadBufLen; i++) {
                     if (!OS_ISSPACE (prereadBuf[i])) {

                        if (OSRTSTREAM_ID (pctxt) != OSRTSTRMID_DIRECTBUF) {
                           stat2 = rtxStreamBufferedCreate (pctxt,
                              OSRTSTRMCM_RESTORE_UNDERLAYING_AFTER_RESET);
                           if (stat2 < 0) len = stat;
                        }
                        if (stat2 == 0) {
                           stat2 = rtxStreamBufferedSetPreReadBuf (pctxt,
                             (const OSOCTET*)prereadBuf + i, prereadBufLen - i);
                           if (stat2 < 0) len = stat2;
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
   }
#endif /* _NO_STREAM */

   if (len < 0) {
      return LOG_RTERR (pctxt, len);
   }
   else if (stat != XML_STATUS_OK) {
      XML_LChar str[256];
      len = RTERR_XMLPARSE;

      EXML_ErrorString (readerImpl->parser,
                        XML_GetErrorCode (readerImpl->parser), str,
                        sizeof(str)/sizeof(str[0]));
      LOG_RTERRNEW (pctxt, len);
      rtxErrAddStrParm (pctxt, LSTRX (pctxt, str));
      return len;
   }

   return 0;
}

/*
 * This function creates and initializes a new instance of parser.
 * It is necessary to allocate the internal structure XMLReaderImpl that
 * will hold the top-level start/end/characters handlers, the pointer to
 * top-level SAX-handler.
 * It will be necessary to convert native SAX handlers calls to
 * XBinder's ones.
 */
OSXMLREADER* rtSaxCCreateXmlReader (OSCTXT* pctxt, void* pSaxHandlerData,
                                    CSAX_StartElementHandler  pStartElementProc,
                                    CSAX_EndElementHandler    pEndElementProc,
                                    CSAX_CharacterDataHandler pCharactersProc)
{
   XML_Parser parser = XML_ParserCreate (NULL); /* Create Expat's parser instance */
   XMLReaderImpl* reader;

   reader = (XMLReaderImpl*)XML_MemMalloc (parser, sizeof (XMLReaderImpl));
   if (reader == 0) return 0;
   OSCRTLMEMSET (reader, 0, sizeof (XMLReaderImpl));

   reader->pStartElementProc = pStartElementProc;
   reader->pEndElementProc   = pEndElementProc;
   reader->pCharactersProc   = pCharactersProc;
   reader->userData = pSaxHandlerData;

   XML_SetUserData (parser, reader);

   XML_SetElementHandler (parser,
                          rtSaxCStartElementHandler,
                          rtSaxCEndElementHandler);
   XML_SetCharacterDataHandler (parser,
                                rtSaxCCharacterDataHandler);
   XML_SetCdataSectionHandler(parser,
                              rtSaxCStartCdataSectionHandler,
                              rtSaxCEndCdataSectionHandler);

   reader->parser = parser;
   reader->reader.pctxt = pctxt;
   return &reader->reader;
}


