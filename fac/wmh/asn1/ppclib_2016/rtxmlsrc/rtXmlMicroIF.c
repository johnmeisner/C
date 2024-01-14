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

/* This is necessary to disable a Visual C++ warning about an unreferenced
   formal parameter passed to rtSaxCReleaseReader */
#ifdef _MSC_VER
#pragma warning(disable: 4100)
#endif /* _MSC_VER */

#include "rtxmlsrc/rtSaxCParser.h"
#include "rtxmlsrc/rtSaxDefs.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxStream.h"
#include "rtxsrc/rtxStreamBuffered.h"
#include "rtxsrc/rtxErrCodes.h"

#define MAX_MEMBLOCK_SIZE 2048

typedef struct {
   OSOCTET buffer[MAX_MEMBLOCK_SIZE];
   OSSIZE lastIdx;
} OSMemBlock;

static void* memAlloc (OSMemBlock* pMemBlk, OSSIZE size)
{
   void* ptr;

   if (pMemBlk->lastIdx + size > MAX_MEMBLOCK_SIZE)
      return 0;
   ptr = &pMemBlk->buffer [pMemBlk->lastIdx];
   pMemBlk->lastIdx += size;
   return ptr;
}

#define memReset(pMemBlk) (pMemBlk)->lastIdx = 0

typedef struct XMLReaderImpl {
   OSXMLREADER reader;
   void* userData;
   OSMemBlock memBlock;
   CSAX_StartElementHandler  pStartElementProc;
   CSAX_EndElementHandler    pEndElementProc;
   CSAX_CharacterDataHandler pCharactersProc;
   OSBOOL bStop;
} XMLReaderImpl;

static void rtSaxCStartElementHandler (XMLReaderImpl* reader,
                                       const OSUTF8CHAR *name,
                                       const OSUTF8CHAR *localname,
                                       const OSUTF8CHAR **atts);
static void rtSaxCEndElementHandler (XMLReaderImpl* reader,
                                     const OSUTF8CHAR *qname,
                                     const OSUTF8CHAR *localname);
static void rtSaxCCharacterDataHandler (XMLReaderImpl* reader,
                                        const OSUTF8CHAR *s,
                                        OSSIZE len);

static void rtSaxCStopParser (XMLReaderImpl* reader);

EXTXMLMETHOD int rtSaxCReleaseReader (OSXMLREADER* pReader)
{
   return 0;
}

void rtSaxCStartElementHandler (XMLReaderImpl* reader,
                                const OSUTF8CHAR *qname,
                                const OSUTF8CHAR *localname,
                                const OSUTF8CHAR **atts)
{
   OSCTXT* pctxt = reader->reader.pctxt;
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo) {
      ctxtInfo->mbCdataProcessed = FALSE;

      ctxtInfo->mSaxLevel++;
      if (ctxtInfo->mSkipLevel < 0) {
         /* fatal error, stop parser */
         rtSaxCStopParser(reader);
         return;
      }
      else if (ctxtInfo->mSkipLevel > 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel)
         return;
   }
   if (reader->pStartElementProc != 0) {
      reader->pStartElementProc (reader->userData,
         (const OSUTF8CHAR*)localname,
         (const OSUTF8CHAR*)qname,
         (const OSUTF8CHAR**)atts);
   }
}

void rtSaxCEndElementHandler (XMLReaderImpl* reader,
                              const OSUTF8CHAR *qname,
                              const OSUTF8CHAR *localname)
{
   OSCTXT* pctxt = reader->reader.pctxt;
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
   OSSAXHandlerBase* pSaxBase;
   OSBOOL bSkip = FALSE;

   if (ctxtInfo) {
      if (ctxtInfo->mSkipLevel != 0 && ctxtInfo->mSaxLevel >= ctxtInfo->mSkipLevel) {
         bSkip = TRUE;
      }
   }
   if (!bSkip && reader->pEndElementProc != 0) {
      reader->pEndElementProc (reader->userData,
         (const OSUTF8CHAR*)localname,
         (const OSUTF8CHAR*)qname);
   }
   if (ctxtInfo) {
      if (ctxtInfo->mSkipLevel > 0 && ctxtInfo->mSaxLevel == ctxtInfo->mSkipLevel)
         ctxtInfo->mSkipLevel = 0; /* reset skipLevel */
      ctxtInfo->mSaxLevel--;

      ctxtInfo->mbCdataProcessed = FALSE;
   }

   /* Set mState to a final state, to indicate parsing is finished */
   pSaxBase = (OSSAXHandlerBase*)reader->userData;
   if (pSaxBase != 0 && pSaxBase->mLevel == 0) {
      rtSaxCStopParser(reader);
      pSaxBase->mState = OS_SAX_FINAL_STATE;
   }
}

void rtSaxCCharacterDataHandler (XMLReaderImpl* reader,
                                 const OSUTF8CHAR *s,
                                 OSSIZE len)
{
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

typedef enum {
   SAXERR_UNEXPECTED_CHAR = -1,
   SAXERR_INVALID_CHAR = -2,
   SAXERR_BADHEADER = -3
} ErrorCodes;

static const OSOCTET transitions [][13] = {
   /* columns - states, raws - input chars */
   /* <   ?   !   >   /  ch  nm  sp   :   -   =   " other */
   {  2,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0 }, /* st=1, initial */
   {  0,  3,  25, 0,  0,  7,  0,  0,  0,  0,  0,  0,  0 }, /* st=2, initial after '<' */
   {  0,  4,  3,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3 }, /* st=3, header, first '?' recvd */
   {  0,  0,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=4, header, last  '?' recvd */
   {  6,  0,  0,  0,  0,  0,  0,  5,  0,  0,  0,  0,  0 }, /* st=5, wtng for strtElm '<' */
   {  0,  0,  25, 0,  0,  7,  0,  0,  0,  0,  0,  0,  0 }, /* st=6, '<' recvd, stElem/cmt */
   {  0,  0,  0,  10, 11, 7,  7,  12, 8,  0,  0,  0,  0 }, /* st=7, first char recvd for stElem */
   {  0,  0,  0,  0,  0,  9,  0,  0,  0,  0,  0,  0,  0 }, /* st=8, ':' ns pfx recvd */
   {  0,  0,  0,  10, 11, 9,  9,  12, 0,  0,  0,  0,  0 }, /* st=9, elem nam after pfx */
   {  20, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10 }, /* st=10, '>' of stElem recvd - chars/endEl/oth stElm */
   {  0,  0,  0,  19, 0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=11, / of stElem recvd */
   {  0,  0,  0,  10, 11, 13, 0,  12, 0,  0,  0,  0,  0 }, /* st=12, first sp in stElm recvd - attrs */
   {  0,  0,  0,  0,  0,  13, 13, 0,  14, 0,  16, 0,  0 }, /* st=13, first sym of attr recvd */
   {  0,  0,  0,  0,  0,  15, 0,  0,  0,  0,  0,  0,  0 }, /* st=14, ':' attr ns pfx recvd */
   {  0,  0,  0,  0,  0,  15, 15, 0,  0,  0,  16, 0,  0 }, /* st=15, rest of attr */
   {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  17, 0 }, /* st=16, '=' recvd */
   {  17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 17 }, /* st=17, beg " recvd */
   {  0,  0,  0,  10, 11, 0,  0,  32, 0,  0,  0,  0,  0 }, /* st=18, final " recvd */
   {  20, 0,  0,  0,  0,  0,  0,  19, 0,  0,  0,  0,  0 }, /* st=19, /> recvd for stElem */
   {  0,  0, 33,  0,  21, 7,  0,  0,  0,  0,  0,  0,  0 }, /* st=20, endElem or anth stElem < recvd*/
   {  0,  0,  0,  0,  0,  22, 0,  0,  0,  0,  0,  0,  0 }, /* st=21, / char endElem */
   {  0,  0,  0,  31, 0,  22, 22, 0,  23, 0,  0,  0,  0 }, /* st=22, first char endElem name */
   {  0,  0,  0,  0,  0,  24, 0,  0,  0,  0,  0,  0,  0 }, /* st=23, : of endElem recvd */
   {  0,  0,  0,  31, 0,  24, 24, 0,  0,  0,  0,  0,  0 }, /* st=24, rest of endElem name */
   {  0,  0,  0,  0,  0,  0,  0,  0,  0,  26, 0,  0,  0 }, /* st=25, <! recvd */
   {  0,  0,  0,  0,  0,  0,  0,  0,  0,  27, 0,  0,  0 }, /* st=26, <!- recvd */
   {  27, 27, 27, 27, 27, 27, 27, 27, 27, 28, 27, 27, 27 }, /* st=27, <!-- recvd */
   {  27, 27, 27, 27, 27, 27, 27, 27, 27, 29, 27, 27, 27 }, /* st=28, - recvd */
   {  27, 27, 27, 30, 27, 27, 27, 27, 27, 29, 27, 27, 27 }, /* st=29, -- recvd */
   {  6,  0,  0,  0,  0,  0,  0,  30, 0,  0,  0,  0,  0 }, /* st=30, end of comments */
   {  20, 0,  0,  0,  0,  0,  0,  31, 0,  0,  0,  0,  0 }, /* st=31, > recvd for end of endElm */
   {  0,  0,  0,  10, 11, 13, 0,  32, 0,  0,  0,  0,  0 }, /* st=32, not first sp in stElm recvd - attrs */
   {  0,  0,  0,  0,  0,  0,  0,  0,  0, 34,  0,  0,  0 }, /* st=33, <! recvd */
   {  0,  0,  0,  0,  0,  0,  0,  0,  0, 35,  0,  0,  0 }, /* st=34, <!- recvd */
   { 35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 35, 35, 35 }, /* st=35, <!-- recvd */
   { 35, 35, 35, 35, 35, 35, 35, 35, 35, 37, 35, 35, 35 }, /* st=36, - recvd */
   { 35, 35, 35, 38, 35, 35, 35, 35, 35, 37, 35, 35, 35 }, /* st=37, -- recvd */
   { 20,  0,  0,  0,  0,  0,  0, 38,  0,  0,  0,  0,  0 }, /* st=38, end of comments */
};

static int getSymbolIndex (OSOCTET sym)
{
   int symbolIndex = -1;

   switch (sym) {
      case '<': symbolIndex = 0; break;
      case '?': symbolIndex = 1; break;
      case '!': symbolIndex = 2; break;
      case '>': symbolIndex = 3; break;
      case '/': symbolIndex = 4; break;
      case ':': symbolIndex = 8; break;
      case '-': symbolIndex = 9; break;
      case '=': symbolIndex = 10; break;
      case '\"': symbolIndex = 11; break;
      default:
         if (OS_ISALPHA (sym)) symbolIndex = 5;
         else if (OS_ISDIGIT (sym)) symbolIndex = 6;
         else if (OS_ISSPACE (sym)) symbolIndex = 7;
         else symbolIndex = 12;
   }
   return symbolIndex;
}

EXTXMLMETHOD int rtSaxCEnableThreadSafety ()
{
   return -1; /* not supported */
}

EXTXMLMETHOD int rtSaxCParse (OSXMLREADER* pReader)
{
   XMLReaderImpl* readerImpl = (XMLReaderImpl*)(void*)pReader;
   OSCTXT* pctxt = pReader->pctxt;
   const OSOCTET *p, *pend, *qnamePtr = 0, *localNamePtr = 0, *endNamePtr = 0,
     *charsPtr = 0, *attrNamePtr = 0, *attrValPtr = 0;
   int state = 1, prevState = 0, len;
   int nAttrs = 0, nCurAttrIdx = 0;
   const OSUTF8CHAR** attrs = 0;

   if (pReader == 0 || pReader->pctxt == 0)
      return RTERR_NOTINIT;

   rtxErrReset (pctxt);

   if (OSRTBUFFER (pctxt) == 0) {
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
   }

   len = (int)(OSRTBUFSIZE (pctxt) - (OSRTBUFPTR (pctxt) - OSRTBUFFER (pctxt)));
   p = OSRTBUFPTR (pctxt);
   pend = p + len - 1;

   for (; !readerImpl->bStop && state > 0 && p <= pend; p++) {
      int symbolIndex = getSymbolIndex (*p);

      if (symbolIndex == -1) {
         state = SAXERR_INVALID_CHAR;
         continue;
      }

      prevState = state;
      state = transitions [state - 1][symbolIndex];

      if (state == 0) {
         /* invalid state! */
         continue;
      }

      if (prevState != state) {
         switch (state) {
            case 22: /* end of element, beginning */
            case 7:  /* beginning of startElement */
               qnamePtr = localNamePtr = p;
               nAttrs = 0;
               endNamePtr = charsPtr = 0;
               attrs = 0;
               memReset (&readerImpl->memBlock);
               break;
            case 9: /* namespace prefix of startElement recvd */
            case 24: /* namespace prefix of endElement recvd */
               localNamePtr = p;
               break;
            case 10: /* end of startElement recvd */
               charsPtr = p + 1;
            case 19: /* empty element recvd */
               /* add terminating 0.0 to attributes array */
               if (nAttrs > 0) {
                  attrs [nCurAttrIdx * 2] = 0;
                  attrs [nCurAttrIdx * 2 + 1] = 0;
               }
            case 31: /* end of endElement */
               {
               OSUTF8CHAR* localName, *qname;

               if (endNamePtr == 0)
                  endNamePtr = p;

               qname = (OSUTF8CHAR*)memAlloc (&readerImpl->memBlock, endNamePtr - qnamePtr + 1);
               if (qname == 0) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

               OSCRTLSAFEMEMCPY (qname, (endNamePtr-qnamePtr+1), qnamePtr,
                     endNamePtr - qnamePtr);
               qname [endNamePtr - qnamePtr] = 0;

               if (qnamePtr != localNamePtr) {
                  localName = (OSUTF8CHAR*)memAlloc (&readerImpl->memBlock, endNamePtr - localNamePtr + 1);
                  if (localName == 0) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
                  OSCRTLSAFEMEMCPY (localName, endNamePtr-localNamePtr+1,
                        localNamePtr, endNamePtr - localNamePtr);
                  localName [endNamePtr - localNamePtr] = 0;
               }
               else
                  localName = qname;

               if (state == 10 || state == 19)
                  rtSaxCStartElementHandler (readerImpl, qname, localName, attrs);

               if (state == 19 || state == 31)
                  rtSaxCEndElementHandler (readerImpl, qname, localName);

               }
               break;
            case 11:
               if ((endNamePtr == 0) && (nAttrs == 0)) endNamePtr = p;
               break;
            case 12: /* end of startElement name recvd - attrs follows */
               {
               int st;
               const OSOCTET *pp;

               endNamePtr = p;
               nAttrs = nCurAttrIdx = 0;

               /* calc number of attributes */
               for (pp = p, st = state; pp <= pend && st != 10; pp++) {
                  int symbolIndex = getSymbolIndex (*pp);

                  if (symbolIndex == -1) {
                     continue;
                  }
                  st = transitions [st - 1][symbolIndex];
                  if (st == 16) { /* = recvd */
                     nAttrs++;
                  } else if ((st == 19) || (st == 10)) { /* recvd end of element (opening) tag */
                     break;
                  }
               }
               /* allocate array of pointers to hold pointers to name/vals */
               attrs = (const OSUTF8CHAR**)memAlloc (&readerImpl->memBlock,
                     (nAttrs + 1) * 2 * sizeof (OSUTF8CHAR*));
               if (attrs == 0) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
               attrs[0] = attrs[1] = 0;
               } break;
            case 13: /* attribute name, beginning */
               attrNamePtr = p;
               break;
            case 16: /* attribute name, ending (= recvd) */
               {
               OSUTF8CHAR* attrName;

               attrName = (OSUTF8CHAR*)memAlloc (&readerImpl->memBlock, p - attrNamePtr + 1);
               if (attrName == 0) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

               OSCRTLSAFEMEMCPY (attrName, p-attrNamePtr+1, attrNamePtr,
                     p - attrNamePtr);
               attrName [p - attrNamePtr] = 0;
               attrs [nCurAttrIdx * 2] = attrName;
               } break;
            case 17: /* attribute value, beginning */
               attrValPtr = p + 1; /* ignore " */
               break;
            case 18: /* attribute value, ending */
               {
               OSUTF8CHAR* attrValue;

               attrValue = (OSUTF8CHAR*)memAlloc (&readerImpl->memBlock, p - attrValPtr + 1);
               if (attrValue == 0) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

               OSCRTLSAFEMEMCPY (attrValue, p-attrValPtr+1, attrValPtr,
                     p - attrValPtr);
               attrValue [p - attrValPtr] = 0;
               attrs [nCurAttrIdx * 2 + 1] = attrValue;

               nCurAttrIdx++;
               } break;

            case 20: /* end of chars */
               if (charsPtr != p) {
                  rtSaxCCharacterDataHandler
                     (readerImpl, charsPtr, p - charsPtr);
               }
               break;
         }
      }
   }

   if (state == 0)
      return LOG_RTERRNEW (pctxt, RTERR_XMLPARSE);

   return 0;
}

EXTXMLMETHOD void rtSaxCStopParser (XMLReaderImpl* reader)
{
   reader->bStop = TRUE;
}

static XMLReaderImpl gReader;


EXTXMLMETHOD OSXMLREADER* rtSaxCCreateXmlReader (OSCTXT* pctxt, void* pSaxHandlerData,
                                    CSAX_StartElementHandler  pStartElementProc,
                                    CSAX_EndElementHandler    pEndElementProc,
                                    CSAX_CharacterDataHandler pCharactersProc)
{
   XMLReaderImpl* reader;

   reader = &gReader;
   if (reader == 0) return 0;
   OSCRTLMEMSET (reader, 0, sizeof (XMLReaderImpl));

   reader->pStartElementProc = pStartElementProc;
   reader->pEndElementProc   = pEndElementProc;
   reader->pCharactersProc   = pCharactersProc;
   reader->userData = pSaxHandlerData;
   reader->bStop = FALSE;

   reader->reader.pctxt = pctxt;
   return &reader->reader;
}



