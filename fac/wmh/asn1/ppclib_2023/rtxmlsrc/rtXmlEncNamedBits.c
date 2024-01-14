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

#include "rtxsrc/rtxBitString.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxmlsrc/osrtxml.hh"

EXTXMLMETHOD int rtXmlEncNamedBitsValue
(OSCTXT* pctxt, const OSBitMapItem* pBitMap, OSSIZE nbits,
 const OSOCTET* pvalue)
{
   if (nbits > 0 && 0 != pvalue) {
      OSSIZE i = 0;
      OSBOOL first = TRUE;

      if (pctxt->state != OSXMLATTR)
         pctxt->state = OSXMLDATA;

      for ( ; pBitMap[i].name != 0; i++) {
         if (pBitMap[i].bitno < nbits &&
             rtxTestBit (pvalue, nbits, pBitMap[i].bitno)) {
            if (!first) {
               rtXmlPutChar (pctxt, ' ');
            }
            else first = FALSE;

            rtXmlWriteChars (pctxt, pBitMap[i].name, pBitMap[i].namelen);
         }
      }
   }
   return 0;
}

EXTXMLMETHOD int rtXmlEncNamedBits
(OSCTXT* pctxt, const OSBitMapItem* pBitMap, OSSIZE nbits,
 const OSOCTET* pvalue, const OSUTF8CHAR* elemName, OSXMLNamespace* pNS)
{
   /* Encode start element tag */

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

   /* Encode content */

   if (0 == stat)
      stat = rtXmlEncNamedBitsValue (pctxt, pBitMap, nbits, pvalue);

   /* Encode end element tag */

   if (0 != elemName && 0 == stat) {
      stat = rtXmlEncEndElement (pctxt, elemName, pNS);
   }

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}

