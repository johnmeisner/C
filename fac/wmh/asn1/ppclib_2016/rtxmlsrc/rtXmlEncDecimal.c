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

#include "rtxsrc/rtxRegExp.hh"
#include "rtxmlsrc/osrtxml.hh"

EXTXMLMETHOD int rtXmlEncDecimal (OSCTXT* pctxt, OSREAL value,
                     const OSUTF8CHAR* elemName,
                     OSXMLNamespace* pNS,
                     const OSDecimalFmt* pFmtSpec)
{
   int stat;
   OSRTDList namespaces;
   rtxDListInit(&namespaces);

   stat = rtXmlEncStartElement (pctxt, elemName, pNS, &namespaces, FALSE);

   /* if rtXmlEncStartElement added any namespaces, encode them */
   if ( stat == 0 )
      stat = rtXmlEncNSAttrs(pctxt, &namespaces);

   /* encode close of start tag */
   if ( stat == 0 )
      stat = rtXmlEncTermStartElement(pctxt);

   if (stat != 0) return LOG_RTERR (pctxt, stat);

   if (pFmtSpec != 0 && pFmtSpec->nPatterns > 0) {
      char lbuf[512];
      int i;
      rtxRegexpPtr pRegExp;

      stat = rtXmlEncDecimalValue
         (pctxt, value, pFmtSpec, lbuf, sizeof (lbuf));

      if (stat != 0) return LOG_RTERR (pctxt, stat);

      /* test encoded value against the patterns */
      for (i = 0; i < pFmtSpec->nPatterns; i++) {
         pRegExp = rtxRegexpCompile (pctxt,
            (const OSUTF8CHAR*)pFmtSpec->patterns[i]);

         if (0 != pRegExp) {

            stat = rtxRegexpExec (pctxt, pRegExp, (OSUTF8CHAR*)lbuf);
            if (stat == 0)
               stat = RTERR_PATMATCH;
            else
               stat = 0;

            rtxRegFreeRegexp (pctxt, pRegExp);
         }
         else
            return LOG_RTERRNEW (pctxt, RTERR_REGEXP);
         if (stat == 0) break;
      }
      if (stat != 0) return LOG_RTERRNEW (pctxt, stat);

      stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)lbuf, OSCRTLSTRLEN(lbuf));
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else {
      stat = rtXmlEncDecimalValue (pctxt, value, pFmtSpec, 0, 0);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   pctxt->state = OSXMLDATA;

   if (0 != elemName) {
      stat = rtXmlEncEndElement (pctxt, elemName, pNS);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return (0);
}


