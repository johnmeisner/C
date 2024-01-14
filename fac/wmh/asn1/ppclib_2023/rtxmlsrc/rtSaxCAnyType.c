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

#include "rtxmlsrc/rtSaxCAnyType.h"
#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/rtxContext.hh"

EXTXMLMETHOD int SAXAnyTypeStartElement
   (void *userData,
    const OSUTF8CHAR* localname,
    const OSUTF8CHAR* qname,
    const OSUTF8CHAR* const* attrs)
{
   AnyTypeSaxHandler* pSaxHandler = (AnyTypeSaxHandler*) userData;
   OSXSDAnyType* pMsgData = pSaxHandler->mpMsgData;
   OSSAXHandlerBase* pSaxBase = &pSaxHandler->mSaxBase;
   OSCTXT* pctxt = pSaxBase->mpCtxt;
   int stat = 0, i = 0;

   SAXDIAGFUNCSTART (pSaxBase, "SAXAnyTypeStartElement", localname);

   if (pSaxBase->mLevel == 0) {
      if (0 != attrs) {
         for (; 0 != attrs[i]; i += 2) {
            if (IS_XSIATTR (attrs[i])) {
               stat = rtXmlDecXSIAttr (pctxt, attrs[i], attrs[i+1]);
            }
            else {
               OSAnyAttr* pAnyAttr = rtxMemAllocType (pctxt, OSAnyAttr);
               pAnyAttr->name = rtxUTF8Strdup (pctxt, attrs[i]);
               pAnyAttr->value = rtxUTF8Strdup (pctxt, attrs[i+1]);
               rtxDListAppend (pctxt, &(pMsgData->attrs), pAnyAttr);
            }
         }
      }
      if (stat != 0) LOG_SAXERR (pctxt, stat);
      pSaxHandler->mEncCtxt.state = OSXMLINIT;

      if (0 != pSaxHandler->mSaxBase.mpElemName &&
          !rtxUTF8StrEqual ((const OSUTF8CHAR*)qname,
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

   SAXDIAGFUNCEND (pSaxBase, "SAXAnyTypeStartElement", localname);

   return 0;
}

EXTXMLMETHOD int SAXAnyTypeCharacters
   (void *userData, const OSUTF8CHAR* chars, OSSIZE length)
{
   AnyTypeSaxHandler* pSaxHandler = (AnyTypeSaxHandler*) userData;
   OSSAXHandlerBase* pSaxBase = &pSaxHandler->mSaxBase;
   OSCTXT* pctxt = pSaxBase->mpCtxt;
   int stat;

   RTDIAG1 (pctxt, "SAXAnyTypeCharacters: start\n");

   stat = rtXmlEncStringValue2 (&pSaxHandler->mEncCtxt, chars, length);
   if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);

   pSaxHandler->mEncCtxt.state = OSXMLDATA;

   RTDIAG1 (pctxt, "SAXAnyTypeCharacters: end\n");
   return 0;
}


/* Check encode buffer for empty element.  This method will modify the
 * buffer contents if element is found to be empty by replacing last
 * '>' character with '/>'.
 */
static OSBOOL SAX_anytype_isEmptyElement
   (AnyTypeSaxHandler* pSaxHandler, const OSUTF8CHAR* qname)
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
         stat = rtXmlPutChar (&pSaxHandler->mEncCtxt, '>');
   }
   else
      pSaxHandler->mEncCtxt.buffer.byteIndex++;    /* restore byte index */

   return FALSE;
}


EXTXMLMETHOD int SAXAnyTypeEndElement
   (void *userData, const OSUTF8CHAR* localname, const OSUTF8CHAR* qname)
{
   AnyTypeSaxHandler* pSaxHandler = (AnyTypeSaxHandler*) userData;
   OSXSDAnyType* pMsgData = pSaxHandler->mpMsgData;
   OSSAXHandlerBase* pSaxBase = &pSaxHandler->mSaxBase;
   OSCTXT* pctxt = pSaxBase->mpCtxt;
   int stat = 0;
   const OSOCTET* data;

   SAXDIAGFUNCSTART (pSaxBase, "SAXAnyTypeEndElement", localname);

   --pSaxBase->mLevel;

   /* Encode end element */

   if (pSaxBase->mLevel > 0 &&
       !SAX_anytype_isEmptyElement (pSaxHandler, qname))
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

   if (pSaxHandler->mSaxBase.mLevel <
      pSaxHandler->mSaxBase.mStartLevel) {
      /* finalize value by trailing zero */

      stat = rtXmlPutChar (&pSaxHandler->mEncCtxt, '\0');
      if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);

      /* Update mem link and open type data variable */

      data = pSaxHandler->mEncCtxt.buffer.data;
      pMsgData->value.value = (const OSUTF8CHAR*)data;

      pSaxHandler->mEncCtxt.state = OSXMLEND;
      SAXAnyTypeFree (pctxt, pSaxHandler);
   }

   SAXDIAGFUNCEND (pSaxBase, "SAXAnyTypeEndElement", localname);

   return 0;
}

EXTXMLMETHOD void SAXAnyTypeInit
  (OSCTXT* pctxt, AnyTypeSaxHandler* pSaxHandler,
   OSXSDAnyType* pvalue, int level)
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
   pvalue->value.value = 0;
   pvalue->value.cdata = FALSE;
   rtxDListInit (&pvalue->attrs);

   pSaxHandler->mpMsgData = pvalue;
   pSaxHandler->bInitialized = TRUE;
   pSaxHandler->mSaxBase.mpStartElement = SAXAnyTypeStartElement;
   pSaxHandler->mSaxBase.mpEndElement   = SAXAnyTypeEndElement;
   pSaxHandler->mSaxBase.mpCharacters   = SAXAnyTypeCharacters;
   pSaxHandler->mSaxBase.mpFree = (CSAX_FreeHandler)SAXAnyTypeFree;
   pSaxHandler->mSaxBase.mpCtxt = pctxt;
   pSaxHandler->mSaxBase.mLevel = 0;
   pSaxHandler->mSaxBase.mStartLevel = (OSINT16)(level + 1);
   pSaxHandler->mSaxBase.mpElemName = 0;
   pSaxHandler->mSaxBase.mpSaxHandlerBuff = 0;
   pSaxHandler->mSaxBase.mReqElemCnt = 0;
   pSaxHandler->mSaxBase.mCurrElemIdx = 0;
   pSaxHandler->mSaxBase.mState = 0;
}

EXTXMLMETHOD int SAXAnyTypeElementInit
  (OSCTXT* pctxt, AnyTypeSaxHandler* pSaxHandler,
   OSXSDAnyType* pvalue, const OSUTF8CHAR* elemName)
{
   SAXAnyTypeInit (pctxt, pSaxHandler, pvalue, 0);
   pSaxHandler->mSaxBase.mpElemName = elemName;
   pSaxHandler->mSaxBase.mStartLevel = 1;
   return 0;
}

EXTXMLMETHOD void SAXAnyTypeFree
  (OSCTXT* pctxt, AnyTypeSaxHandler* pSaxHandler)
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


