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

#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/rtxCommonDefs.h"

EXTXMLMETHOD int rtXmlEncBinStrValue (OSCTXT* pctxt,
                         OSSIZE nbits,
                         const OSOCTET* data)
{
   OSSIZE i;
   int stat;
   if (pctxt->state != OSXMLATTR)
      pctxt->state = OSXMLDATA;

   if (nbits > 0 && 0 != data) {
      stat = rtXmlCheckBuffer (pctxt, nbits);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      for (i = 0; i < nbits; i++) {
         if (rtxTestBit (data, nbits, i)) {
            stat = rtXmlPutChar (pctxt, '1');
         }
         else {
            stat = rtXmlPutChar (pctxt, '0');
         }
      }
   }

   return 0;
}

EXTXMLMETHOD int rtXmlEncBitString (OSCTXT* pctxt, OSSIZE nbits,
                       const OSOCTET* value,
                       const OSUTF8CHAR* elemName,
                       OSXMLNamespace* pNS)
{
   int stat = 0;

   if (nbits == 0) {
      if (0 != elemName) {
         OSRTDList namespaces;         /* isolate any ns prefixes created by */
                                       /* rtXmlEncEmptyElement */
         rtxDListInit(&namespaces);

         stat = rtXmlEncEmptyElement (pctxt, elemName, pNS, &namespaces, TRUE);
      }
   }
   else {
      OSRTDList namespaces;
      rtxDListInit(&namespaces);

      stat = rtXmlEncStartElement (pctxt, elemName, pNS, &namespaces, FALSE);

      /* if rtXmlEncStartElement added any namespaces, encode them */
      if ( stat == 0 )
         stat = rtXmlEncNSAttrs(pctxt, &namespaces);

      /* encode close of start tag */
      if ( stat == 0 )
         stat = rtXmlEncTermStartElement(pctxt);

      if (0 == stat)
         stat = rtXmlEncBinStrValue (pctxt, nbits, value);

      if (0 != elemName && 0 == stat) {
         stat = rtXmlEncEndElement (pctxt, elemName, pNS);
      }
   }

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}

EXTXMLMETHOD int rtXmlEncBitStringExt (OSCTXT* pctxt, OSSIZE nbits,
                      const OSOCTET* value,
                      OSSIZE dataSize,
                      const OSOCTET* extValue,
                      const OSUTF8CHAR* elemName,
                      OSXMLNamespace* pNS)
{
   int stat = 0;

   if (nbits == 0) {
      if (0 != elemName) {
         OSRTDList namespaces;         /* isolate any ns prefixes created by */
                                       /* rtXmlEncEmptyElement */
         rtxDListInit(&namespaces);

         stat = rtXmlEncEmptyElement (pctxt, elemName, pNS, &namespaces, TRUE);
      }
   }
   else {
      OSSIZE dataSizeBits;
      OSRTDList namespaces;

      if (dataSize > OSSIZE_MAX/8) {
         return LOG_RTERR (pctxt, RTERR_TOOBIG);
      }
      dataSizeBits = dataSize * 8;

      rtxDListInit(&namespaces);

      stat = rtXmlEncStartElement (pctxt, elemName, pNS, &namespaces, FALSE);

      /* if rtXmlEncStartElement added any namespaces, encode them */
      if ( stat == 0 )
         stat = rtXmlEncNSAttrs(pctxt, &namespaces);

      /* encode close of start tag */
      if ( stat == 0 )
         stat = rtXmlEncTermStartElement(pctxt);

      /* Encode root data value */
      if (0 == stat) {
         if (nbits <= dataSizeBits) {
            stat = rtXmlEncBinStrValue (pctxt, nbits, value);
         }
         else {
            stat = rtXmlEncBinStrValue (pctxt, dataSizeBits, value);

            /* Encode extdata value */
            if (extValue && (0 == stat))
               stat = rtXmlEncBinStrValue 
                  (pctxt, nbits - dataSizeBits, extValue);
         }
      }

      if (0 != elemName && 0 == stat) {
         stat = rtXmlEncEndElement (pctxt, elemName, pNS);
      }
   }

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}

