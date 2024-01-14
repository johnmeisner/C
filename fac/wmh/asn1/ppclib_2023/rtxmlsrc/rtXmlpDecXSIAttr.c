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
#include "rtxmlsrc/osrtxml.h"
#include "rtxmlsrc/rtXmlPull.h"

/* Decode XSI namespace attributes */

EXTXMLMETHOD int rtXmlpDecXSIAttr (OSCTXT* pctxt, const OSXMLNameFragments* attrName)
{
   int stat = 1; /* zero - decoded OK, negative - NOK, > 0 - skipped */
   const OSUTF8CHAR* attrValue;

   OSRTASSERT (0 != pctxt->pXMLInfo);

   if (rtxUTF8StrnEqual (attrName->mPrefix.value, OSUTF8("xsi"), 3)) {
      if (rtxUTF8StrnEqual (attrName->mLocalName.value,
                            OSUTF8("schemaLocation"), 13)) {
         stat = rtXmlpDecDynUTF8Str (pctxt, &attrValue);
         if (0 != stat) return LOG_RTERR (pctxt, stat);
         stat = rtXmlSetSchemaLocation (pctxt, attrValue);
      }
      else if (rtxUTF8StrnEqual (attrName->mLocalName.value,
                                 OSUTF8("noNamespaceSchemaLocation"), 25)) {
         stat = rtXmlpDecDynUTF8Str (pctxt, &attrValue);
         if (0 != stat) return LOG_RTERR (pctxt, stat);
         stat = rtXmlSetNoNSSchemaLocation (pctxt, attrValue);
      }
      else if (rtxUTF8StrnEqual (attrName->mLocalName.value,
                                 OSUTF8("type"), 4)) {
         /* this is now handled in derived type processing */
         stat = 0;
      }
      else if (rtxUTF8StrnEqual (attrName->mLocalName.value,
                                 OSUTF8("nil"), 3)) {
         /* ignore xsi:nil */
         stat = 0;
      }
   }
#if 0
   else if (rtxUTF8StrnEqual (attrName->mQName.value, OSUTF8("xmlns"), 5)) {
      OSUTF8CHAR* nsPrefix = 0;

      /* decode namespace URI value*/
      stat = rtXmlpDecDynUTF8Str (pctxt, &attrValue);
      if (0 != stat) return LOG_RTERR (pctxt, stat);

      if (attrName->mQName.length > 6) {
         /* xmlns:prefix="uri" */
         nsPrefix = (OSUTF8CHAR*)
            rtxMemAlloc (pctxt, attrName->mQName.length - 5);

         OSCRTLMEMCPY ((void*)nsPrefix,
                 attrName->mQName.value+6,
                 attrName->mQName.length-6);

         nsPrefix[attrName->mQName.length - 5] = '\0';
      }
      rtXmlNSSetNamespace (pctxt, nsPrefix, attrValue, TRUE);
   }
#endif
   else if (rtxUTF8StrnEqual (attrName->mQName.value,
                              OSUTF8("SOAP-ENV:"), 9)) {
      stat = 0;
   }

   return stat;
}

EXTXMLMETHOD int rtXmlpGetXmlnsAttrs (OSCTXT* pctxt, OSRTDList* pNSAttrs) {
   OSXMLNameFragments attr;
   OSXMLNamespace* pNS;
   int nmAttributes, i;
   OSINT16 nsidx;
   int stat;

   nmAttributes = rtXmlpGetAttributeCount (pctxt);

   for (i = 0; i < nmAttributes; i++) {
      stat = rtXmlpSelectAttribute (pctxt, &attr, &nsidx, i);
      if (0 != stat) return LOG_RTERR (pctxt, stat);

      if (nsidx != OSXMLNSI_XMLNS) continue;

      pNS = rtxMemAllocTypeZ (pctxt, OSXMLNamespace);
      if (pNS == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);

      if (attr.mPrefix.length == 0)
         pNS->prefix = rtxUTF8Strdup (pctxt, (const OSUTF8CHAR*)"");
      else
         pNS->prefix = rtxUTF8Strndup
            (pctxt, attr.mLocalName.value, attr.mLocalName.length);

      if (0 == pNS->prefix) {
         rtxMemFreePtr (pctxt, pNS);
         return LOG_RTERR (pctxt, RTERR_NOMEM);
      }

      rtXmlpSetWhiteSpaceMode (pctxt, OSXMLWSM_COLLAPSE);
      stat = rtXmlpDecDynUTF8Str (pctxt, &(pNS->uri));
      if (0 != stat) {
         rtxMemFreePtr (pctxt, pNS->prefix);
         rtxMemFreePtr (pctxt, pNS);
         return LOG_RTERR (pctxt, stat);
      }

      rtxDListAppend (pctxt, pNSAttrs, pNS);
   }

   return 0;
}

EXTXMLMETHOD int rtXmlpDecXSIAttrs (OSCTXT* pctxt)
{
   return rtXmlRdGetXSITypeAttr (XMLPREADER(pctxt), 0, 0, 0);
}


