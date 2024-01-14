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

#include "rtxmlsrc/osrtxml.hh"

/*
 * This array is a lookup table that translates 6-bit positive integer
 * index values into their "Base64 Alphabet" equivalents as specified
 * in Table 1 of RFC 2045.
 */
static const char encodeTable[] = {
   'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
   'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
   'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
   'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};



EXTXMLMETHOD int rtXmlEncBase64StrValue (OSCTXT* pctxt, OSSIZE nocts,
                            const OSOCTET* value)
{
   OSSIZE i;
   int stat;
   char lbuf[128], *pDstData = (char*)lbuf;
   OSSIZE numFullGroups = nocts/3;
   OSSIZE numBytesInPartialGroup = nocts - 3*numFullGroups;
   const OSOCTET* pSrcData = value;

   /* Translate all full groups from byte array elements to Base64 */
   for (i = 0; i < numFullGroups; i++) {
      int byte0 = *pSrcData++ & 0xff;
      int byte1 = *pSrcData++ & 0xff;
      int byte2 = *pSrcData++ & 0xff;
      *pDstData++ = encodeTable[byte0 >> 2];
      *pDstData++ = encodeTable[((byte0 << 4) & 0x3f) | (byte1 >> 4)];
      *pDstData++ = encodeTable[((byte1 << 2) & 0x3f) | (byte2 >> 6)];
      *pDstData++ = encodeTable[byte2 & 0x3f];

      if (pDstData >= (((char*)lbuf) + sizeof(lbuf))) {
         /* flush buffer */
         stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)lbuf, sizeof(lbuf));
         if (stat != 0) return LOG_RTERR (pctxt, stat);
         pDstData = (char*)lbuf;
      }
   }

   /* Translate partial group if present */
   if (numBytesInPartialGroup != 0) {
      int byte0 = *pSrcData++ & 0xff;
      *pDstData++ = encodeTable[byte0 >> 2];
      if (numBytesInPartialGroup == 1) {
         *pDstData++ = encodeTable[(byte0 << 4) & 0x3f];
         *pDstData++ = '=';
         *pDstData++ = '=';
      } else {
         /* assert numBytesInPartialGroup == 2; */
         int byte1 = *pSrcData++ & 0xff;
         *pDstData++ = encodeTable[((byte0 << 4) & 0x3f) | (byte1 >> 4)];
         *pDstData++ = encodeTable[(byte1 << 2) & 0x3f];
         *pDstData++ = '=';
      }
   }
   /* flush buffer */
   stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)lbuf,
                         pDstData - ((char*)lbuf));

   if (stat != 0) return LOG_RTERR (pctxt, stat);

   if (pctxt->state != OSXMLATTR)
      pctxt->state = OSXMLDATA;

   return (0);
}

EXTXMLMETHOD int rtXmlEncBase64Binary (OSCTXT* pctxt, OSSIZE nocts,
                          const OSOCTET* value,
                          const OSUTF8CHAR* elemName,
                          OSXMLNamespace* pNS)
{
   int stat = 0;

   if (nocts == 0) {
      if (!OS_ISEMPTY(elemName)) {
         OSRTDList namespaces;         /* isolate any ns prefixes created by */
                                       /* rtXmlEncEmptyElement */
         rtxDListInit(&namespaces);

         stat = rtXmlEncEmptyElement (pctxt, elemName, pNS, &namespaces, TRUE);

         pctxt->state = OSXMLEND;
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
         stat = rtXmlEncBase64StrValue (pctxt, nocts, value);

      if (0 != elemName && 0 == stat) {
         stat = rtXmlEncEndElement (pctxt, elemName, pNS);
      }
   }

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}

