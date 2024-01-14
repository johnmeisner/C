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

#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxToken.h"

#include "rtxmlsrc/rtSaxCSimpleType.h"

/**************************************************************/
/*                                                            */
/*  simpleType                                                */
/*                                                            */
/**************************************************************/
EXTXMLMETHOD int SAXSimpleTypeStartElement
   (void *userData, const OSUTF8CHAR* localname,
    const OSUTF8CHAR* qname, const OSUTF8CHAR* const* attrs)
{
   int stat = 0;
   SimpleTypeSaxHandler* pSaxHandler = (SimpleTypeSaxHandler*) userData;
   OSSAXHandlerBase* pSaxBase = &pSaxHandler->mSaxBase;
   OSCTXT* pctxt = pSaxBase->mpCtxt;

   SAXDIAGFUNCSTART (pSaxBase, "SAXSimpleTypeStartElement", qname);

   if (pSaxBase->mLevel == 0) {
      /* parse attributes */
      if (0 != attrs) {
         int i = 0;
         for (; 0 != attrs[i]; i += 2) {
            OSXMLSETUTF8DECPTR (pctxt, attrs[i+1]);

            /* check for namespace attribute (xmlns) */
            if (IS_XMLNSATTR (attrs[i])) {
               stat = rtXmlDecNSAttr
                  (pctxt, attrs[i], attrs[i+1], 0, 0, 0);
               if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);
            }
            else if (IS_XSIATTR (attrs[i])) {
               stat = rtXmlDecXSIAttr (pctxt, attrs[i], attrs[i+1]);
               if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);
            }
         }
      }

      pctxt->state = OSXMLSTART;

      if (!rtxUTF8StrEqual (localname, pSaxHandler->mSaxBase.mpElemName)) {
         return LOG_SAXERRNEW2_AND_SKIP (pctxt, RTERR_IDNOTFOU,
            rtxErrAddStrParm
               (pctxt, (const char*)pSaxHandler->mSaxBase.mpElemName),
            rtxErrAddStrParm (pctxt, (const char*)qname));
      }
   }
   pSaxBase->mLevel++;

   SAXDIAGFUNCEND (pSaxBase, "SAXSimpleTypeStartElement", qname);
   return 0;
}

EXTXMLMETHOD int SAXSimpleTypeCharacters
   (void *userData, const OSUTF8CHAR* chars, int length)
{
   SimpleTypeSaxHandler* pSaxHandler = (SimpleTypeSaxHandler*) userData;
   OSSAXHandlerBase* pSaxBase = &pSaxHandler->mSaxBase;
   OSCTXT* pctxt = pSaxBase->mpCtxt;

   RTDIAG1 (pctxt, "SAXSimpleTypeCharacters: start\n");

   if ((pctxt->state == OSXMLSTART || pctxt->state == OSXMLDATA)
      && (pSaxBase->mLevel >= 1))
   {
      int stat;
      pctxt->state = OSXMLDATA;
      stat = rtxMemBufAppend (&pSaxHandler->mCurrElemValue,
                              (const OSOCTET*)chars,
                              length * sizeof (OSUTF8CHAR));
      if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);
   }
   RTDIAG1 (pctxt, "SAXSimpleTypeCharacters: end\n");
   return 0;
}

EXTXMLMETHOD int SAXSimpleTypeEndElement
   (void *userData, const OSUTF8CHAR* localname, const OSUTF8CHAR* qname)
{
   SimpleTypeSaxHandler* pSaxHandler = (SimpleTypeSaxHandler*) userData;
   OSSAXHandlerBase* pSaxBase = &pSaxHandler->mSaxBase;
   OSCTXT* pctxt = pSaxBase->mpCtxt;

   SAXDIAGFUNCSTART (pSaxBase, "SAXSimpleTypeEndElement", localname);

   if (pSaxBase->mLevel == 1 &&
      (pctxt->state == OSXMLSTART || pctxt->state == OSXMLDATA))
   {
      rtXmlFinalizeMemBuf (&pSaxHandler->mCurrElemValue);
   }

   SAXDIAGFUNCEND (pSaxBase, "SAXSimpleTypeEndElement", qname);
   return 0;
}

EXTXMLMETHOD int SAXSimpleTypeInit
   (OSCTXT* pctxt, SimpleTypeSaxHandler* pSaxHandler,
    const OSUTF8CHAR* elemName)
{
   OSCRTLMEMSET (pSaxHandler, 0, sizeof (*pSaxHandler));
   rtxMemBufInit (pctxt, &pSaxHandler->mCurrElemValue, OSRTMEMBUFSEG);

   pSaxHandler->mSaxBase.mpElemName = elemName;
   pSaxHandler->mSaxBase.mpCtxt = pctxt;
   pSaxHandler->mSaxBase.mpStartElement =
      (CSAX_StartElementHandler)SAXSimpleTypeStartElement;
   pSaxHandler->mSaxBase.mpEndElement   =
      (CSAX_EndElementHandler)SAXSimpleTypeEndElement;
   pSaxHandler->mSaxBase.mpCharacters   =
      (CSAX_CharacterDataHandler)SAXSimpleTypeCharacters;
   pSaxHandler->mSaxBase.mpFree = (CSAX_FreeHandler)SAXSimpleTypeFree;
   return 0;
}

EXTXMLMETHOD void SAXSimpleTypeFree
   (OSCTXT* pctxt, SimpleTypeSaxHandler* pSaxHandler)
{
   OS_UNUSED_ARG(pctxt);
   rtxMemBufFree (&pSaxHandler->mCurrElemValue);
   pctxt = 0; /* to keep VC++ -W4 happy */
}


