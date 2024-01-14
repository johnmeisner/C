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

#include "rtxmlsrc/rtSaxCAny.h"
#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/rtxContext.hh"

EXTXMLMETHOD int SAXAnyStartElement
   (void *userData,
    const OSUTF8CHAR* localname,
    const OSUTF8CHAR* qname,
    const OSUTF8CHAR* const* attrs)
{
   AnySaxHandler* pSaxHandler = (AnySaxHandler*) userData;
   OSSAXHandlerBase* pSaxBase = &pSaxHandler->mSaxBase;
   OSCTXT* pctxt = pSaxBase->mpCtxt;
   int stat = 0, i;

   SAXDIAGFUNCSTART (pSaxBase, "SAXAnyStartElement", localname);

   if (pSaxBase->mLevel == 0 && pSaxHandler->mSaxBase.mpElemName != 0) {
      stat = rtXmlDecXSIAttrs (pctxt, attrs, "any");
      if (stat != 0) LOG_SAXERR (pctxt, stat);

      pSaxHandler->mEncCtxt.state = OSXMLINIT;

      if (!rtxUTF8StrEqual ((const OSUTF8CHAR*)qname,
           pSaxHandler->mSaxBase.mpElemName))
      {
         return LOG_SAXERRNEW2_AND_SKIP (pctxt, RTERR_IDNOTFOU,
            rtxErrAddStrParm
               (pctxt, (const char*)pSaxHandler->mSaxBase.mpElemName),
            rtxErrAddStrParm (pctxt, (const char*)qname));
      }
   }

   if (pSaxBase->mLevel == pSaxBase->mStartLevel) {
      pSaxHandler->mEncCtxt.state = OSXMLSTART;
   }

   if (pSaxHandler->mEncCtxt.state != OSXMLINIT) {

      /* Encode start element */

      stat = rtXmlPutChar (&pSaxHandler->mEncCtxt, '<');
      if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);

      stat = rtXmlWriteChars (&pSaxHandler->mEncCtxt, qname,
                       rtxUTF8LenBytes ((const OSUTF8CHAR*)qname));
      if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);


      /* add attributes if present */

      if (attrs != 0) {
         for (i = 0; attrs[i] != 0; i += 2) {
            stat = rtXmlEncUTF8Attr
               (&pSaxHandler->mEncCtxt, attrs[i], attrs[i + 1]);
            if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);
         }
      }
      stat = rtXmlPutChar (&pSaxHandler->mEncCtxt, '>');
      if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);
   }
   /* Bump level */

   pSaxBase->mLevel++;

   SAXDIAGFUNCEND (pSaxBase, "SAXAnyStartElement", localname);

   return 0;
}

EXTXMLMETHOD int SAXAnyCharacters
   (void *userData, const OSUTF8CHAR* chars, int length)
{
   AnySaxHandler* pSaxHandler = (AnySaxHandler*) userData;
   OSSAXHandlerBase* pSaxBase = &pSaxHandler->mSaxBase;
   OSCTXT* pctxt = pSaxBase->mpCtxt;
   int stat;

   RTDIAG1 (pctxt, "SAXAnyCharacters: start\n");

   if (pSaxHandler->mEncCtxt.state == OSXMLSTART ||
       pSaxHandler->mEncCtxt.state == OSXMLDATA)
   {
      stat = rtXmlEncStringValue2 (&pSaxHandler->mEncCtxt, chars, length);
      if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);

      pSaxHandler->mEncCtxt.state = OSXMLDATA;
   }
   RTDIAG1 (pctxt, "SAXAnyCharacters: end\n");
   return 0;
}

/* Check encode buffer for empty element.  This method will modify the
 * buffer contents if element is found to be empty by replacing last
 * '>' character with '/>'.
 */
static OSBOOL SAX_any_isEmptyElement
   (AnySaxHandler* pSaxHandler, const OSUTF8CHAR* qname)
{
   char* bufp;
   int stat;

   pSaxHandler->mEncCtxt.buffer.byteIndex--;
   bufp = (char*) OSRTBUFPTR ((&pSaxHandler->mEncCtxt));
   if (*bufp == '>') { /* no character data */
      *bufp-- = '\0';  /* null out last '>' character */
      while (*bufp != '<') {
         bufp--;
      }
      bufp++;
      if (rtXmlStrCmpAsc (qname, bufp)) {
         /* empty element */

         /* append null element indicator */
         stat = rtXmlWriteChars (&pSaxHandler->mEncCtxt, OSUTF8("/>"), 2);
         if (stat != 0) {
            LOG_SAXERR_AND_SKIP (&pSaxHandler->mEncCtxt, stat);
            return FALSE;
         }
         else return TRUE;
      }
      else
         /* put back original '>' */
         stat = rtXmlWriteChars (&pSaxHandler->mEncCtxt, OSUTF8(">"), 1);
   }
   else
      pSaxHandler->mEncCtxt.buffer.byteIndex++;    /* restore byte index */

   return FALSE;
}

EXTXMLMETHOD int SAXAnyEndElement
   (void *userData, const OSUTF8CHAR* localname, const OSUTF8CHAR* qname)
{
   AnySaxHandler* pSaxHandler = (AnySaxHandler*) userData;
   OSXMLSTRING* pMsgData = pSaxHandler->mpMsgData;
   OSSAXHandlerBase* pSaxBase = &pSaxHandler->mSaxBase;
   OSCTXT* pctxt = pSaxBase->mpCtxt;
   int stat = 0;
   const OSOCTET* data;

   SAXDIAGFUNCSTART (pSaxBase, "SAXAnyEndElement", localname);

   --pSaxBase->mLevel;

   if (pSaxHandler->mEncCtxt.state == OSXMLSTART ||
       pSaxHandler->mEncCtxt.state == OSXMLDATA)
   {
      /* Encode end element */

      if (pSaxBase->mLevel >= 0 &&
          !SAX_any_isEmptyElement (pSaxHandler, qname))
      {
         stat = rtXmlWriteChars (&pSaxHandler->mEncCtxt, OSUTF8("</"), 2);
         if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);

         stat = rtXmlWriteChars (&pSaxHandler->mEncCtxt,
                          qname,
                          rtxUTF8LenBytes ((const OSUTF8CHAR*)qname));
         if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);

         stat = rtXmlPutChar (&pSaxHandler->mEncCtxt, '>');
         if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);
      }

      if (ISCOMPLETE (pSaxHandler)) {
         /* finalize value by trailing zero */

         stat = rtXmlPutChar (&pSaxHandler->mEncCtxt, '\0');
         if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);

         /* Update mem link and open type data variable */

         data = pSaxHandler->mEncCtxt.buffer.data;
         pMsgData->value = (const OSUTF8CHAR*)data;

         pSaxHandler->mEncCtxt.state = OSXMLEND;
         SAXAnyFree (pctxt, pSaxHandler);
      }
   }
   SAXDIAGFUNCEND (pSaxBase, "SAXAnyEndElement", localname);

   return 0;
}

EXTXMLMETHOD void SAXAnyInit
  (OSCTXT* pctxt, AnySaxHandler* pSaxHandler,
   OSXMLSTRING* pvalue, int level)
{
   int stat;

   if ((stat = rtxPreInitContext (&pSaxHandler->mEncCtxt)) != 0) {
      LOG_SAXERR (pctxt, stat);
      return;
   }
   rtxCtxtSetMemHeap (&pSaxHandler->mEncCtxt, pctxt);
   rtXmlSetEncBufPtr (&pSaxHandler->mEncCtxt, 0, 1024);
   pSaxHandler->mEncCtxt.state = OSXMLSTART;

   /* reset pvalue */
   pvalue->value = 0;
   pvalue->cdata = FALSE;

   pSaxHandler->mpMsgData = pvalue;
   pSaxHandler->bInitialized = TRUE;
   pSaxHandler->mSaxBase.mpStartElement = SAXAnyStartElement;
   pSaxHandler->mSaxBase.mpEndElement   = SAXAnyEndElement;
   pSaxHandler->mSaxBase.mpCharacters   = SAXAnyCharacters;
   pSaxHandler->mSaxBase.mpFree = (CSAX_FreeHandler)SAXAnyFree;
   pSaxHandler->mSaxBase.mpCtxt = pctxt;
   pSaxHandler->mSaxBase.mLevel = 0;
   pSaxHandler->mSaxBase.mStartLevel = (OSINT16) level;
   pSaxHandler->mSaxBase.mpElemName = 0;
   pSaxHandler->mSaxBase.mpSaxHandlerBuff = 0;
   pSaxHandler->mSaxBase.mReqElemCnt = 0;
   pSaxHandler->mSaxBase.mCurrElemIdx = 0;
   pSaxHandler->mSaxBase.mState = 0;
}

EXTXMLMETHOD int SAXAnyElementInit
  (OSCTXT* pctxt, AnySaxHandler* pSaxHandler,
   OSXMLSTRING* pvalue, const OSUTF8CHAR* elemName)
{
   SAXAnyInit (pctxt, pSaxHandler, pvalue, 0);
   pSaxHandler->mSaxBase.mpElemName = elemName;
   pSaxHandler->mSaxBase.mStartLevel = 1;
   return 0;
}

EXTXMLMETHOD void SAXAnyFree
  (OSCTXT* pctxt, AnySaxHandler* pSaxHandler)
{
   OS_UNUSED_ARG(pctxt);
   if (pSaxHandler->bInitialized) {
      pSaxHandler->mEncCtxt.buffer.data = 0; /* to prevent from freeing */
      pSaxHandler->mEncCtxt.buffer.size = 0;
      rtxFreeContext (&pSaxHandler->mEncCtxt);
      pSaxHandler->bInitialized = FALSE;
      pctxt = 0; /* to keep VC++ -W4 happy */
   }
}


