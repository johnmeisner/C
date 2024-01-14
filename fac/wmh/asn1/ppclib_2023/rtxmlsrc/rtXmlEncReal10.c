/*
 * Copyright (c) 1997-2023 Objective Systems, Inc.
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

int encXerReal10 (OSCTXT* pctxt, const OSUTF8CHAR* pvalue)
{
   int  stat;
   const OSUTF8CHAR* p;
   OSUTF8CHAR* pExp;
   const OSUTF8CHAR* mantBeg = 0;
   const OSUTF8CHAR* mantEnd = 0;
   int digits = 0;
   int trailZeros = 0;
   int mantExpVal = 0;
   int expVal = 0;
   char mantMinus = 0;
   char expMinus = 0;
   char flFrac = 0;
   OSUTF8CHAR c;
   OSUTF8CHAR expBuf[8];

   if (!(pctxt->flags & OSXMLC14N)) {
      stat = rtXmlWriteChars(pctxt, pvalue, OSUTF8LEN(pvalue));
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else {
      /* skip ws and leading zeros and plus */
      for (p = pvalue; (c = *p) != 0; p++) {
         if (c == '-')
            mantMinus = 1;
         else if (c != ' ' && c != '0' && c != '+')
            break;
      }

      mantBeg = mantEnd = p;

      /* count mantissa digits */
      for (; (c = *p) != 0; p++) {
         if (c == '0')
            trailZeros++;
         else if (c > '0' && c <= '9') {
            if (digits == 0 && flFrac) {
               mantExpVal = -trailZeros;
               digits = 1;
               mantBeg = p;
            }
            else
               digits += trailZeros + 1;

            trailZeros = 0;
            mantEnd = p + 1;
         }
         else if (c == '.') {
            mantExpVal = digits + trailZeros;
            flFrac = 1;
         }
         else if (c == 'e' || c == 'E') {
            p++;
            break;
         }
         else if (c != ' ')
            break;
      }

      if (digits > 0) {
         if (!flFrac)
            mantExpVal = digits + trailZeros;

         /* skip ws and leading zeros and plus */
         for (; (c = *p) != 0; p++) {
            if (c == '-')
               expMinus = 1;
            else if (c != ' ' && c != '0' && c != '+')
               break;
         }

         /* get exponent */
         for (; (c = *p) != 0; p++) {
            if (c >= '0' && c <= '9')
               expVal = expVal * 10 + (c - '0');
            else if (c != ' ')
               break;
         }

         if (expMinus)
            expVal = -expVal;

         expVal += mantExpVal;

         expVal -= 1;

         /* put mantissa */
         if (mantMinus) {
            c ='-';
            stat = rtXmlPutChar (pctxt, c);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }

         stat = rtXmlPutChar (pctxt, *mantBeg);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         mantBeg++;

         if (digits == 1) {
            stat = rtXmlWriteChars (pctxt, OSUTF8(".0E"), 3);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }
         else {
            c ='.';
            stat = rtXmlPutChar (pctxt, c);
            if (stat != 0) return LOG_RTERR (pctxt, stat);

            while (mantBeg != mantEnd) {
               c = *mantBeg++;
               if (c >= '0' && c <= '9') {
                  stat = rtXmlPutChar (pctxt, c);
                  if (stat != 0) return LOG_RTERR (pctxt, stat);
               }
            }

            c ='E';
            stat = rtXmlPutChar (pctxt, c);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }

         /* put exponent */
         if (expMinus) {
            c ='-';
            stat = rtXmlPutChar (pctxt, c);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }

         pExp = expBuf + sizeof (expBuf);
         do {
            *(--pExp) = (expVal % 10) + '0';
            expVal /= 10;
         } while (expVal);

         stat = rtXmlWriteChars (pctxt, pExp,
            expBuf + sizeof (expBuf) - pExp);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      else { /* encode 0 */
         c ='0';
         stat = rtXmlPutChar (pctxt, c);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
   }

   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return (0);
}


int encXmlReal10
(OSCTXT *pctxt, const OSUTF8CHAR *pvalue)
{
   int  stat = 0;
   const char* p;
   const char* mantBeg = 0;
   const char* mantEnd = 0;
   int digits = 0;
   int trailZeros = 0;
   int mantExpVal = 0;
   int expVal = 0;
   char mantMinus = 0;
   char expMinus = 0;
   char flFrac = 0;
   char c;

   /* skip ws and leading zeros and plus */
   for (p = (const char*)pvalue; (c = *p) != 0; p++) {
      if (c == '-')
         mantMinus = 1;
      else if (c != ' ' && c != '0' && c != '+')
         break;
   }

   mantBeg = mantEnd = p;

   /* count mantissa digits */
   for (; (c = *p) != 0; p++) {
      if (c == '0')
         trailZeros++;
      else if (c > '0' && c <= '9') {
         if (digits == 0 && flFrac) {
            mantExpVal = -trailZeros;
            digits = 1;
            mantBeg = p;
         }
         else
            digits += trailZeros + 1;

         trailZeros = 0;
         mantEnd = p + 1;
      }
      else if (c == '.') {
         mantExpVal = digits + trailZeros;
         flFrac = 1;
      }
      else if (c == 'e' || c == 'E') {
         p++;
         break;
      }
      else if (c != ' ')
         break;
   }

   if (digits == 0) {
      stat = rtXmlPutChar (pctxt, '0');
   }
   else {
      if (!flFrac)
         mantExpVal = digits + trailZeros;

      /* skip ws and leading zeros and plus */
      for (; (c = *p) != 0; p++) {
         if (c == '-')
            expMinus = 1;
         else if (c != ' ' && c != '0' && c != '+')
            break;
      }

      /* get exponent */
      for (; (c = *p) != 0; p++) {
         if (c >= '0' && c <= '9')
            expVal = expVal * 10 + (c - '0');
         else if (c != ' ')
            break;
      }

      if (expMinus)
         expVal = -expVal;

      expVal += mantExpVal;

      if (mantMinus) stat = rtXmlPutChar (pctxt, '-');
      if (expVal <= 0) {
         stat = rtXmlPutChar (pctxt, '0');

         if (digits > 0)
            stat = rtXmlPutChar (pctxt, '.');
      }

      while (expVal < 0) {
         expVal++;
         stat = rtXmlPutChar (pctxt, '0');
      }

      while (mantBeg != mantEnd) {
         c = *mantBeg++;
         if (c >= '0' && c <= '9') {
            stat = rtXmlPutChar (pctxt, c);
            if (--expVal == 0 && mantBeg != mantEnd)
               stat = rtXmlPutChar (pctxt, '.');
         }
      }

      while (expVal > 0) {
         expVal--;
         stat = rtXmlPutChar (pctxt, '0');
      }
   }

   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return (0);
}

EXTXMLMETHOD int rtXmlEncReal10
(OSCTXT *pctxt, const OSUTF8CHAR *pvalue, const OSUTF8CHAR* elemName,
 OSXMLNamespace* pNS)
{
   int stat = 0;
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

   if (pctxt->state != OSXMLATTR)
      pctxt->state = OSXMLDATA;

   if ( rtxCtxtTestFlag(pctxt, OSASN1XER) ) {
      stat = encXerReal10(pctxt, pvalue);
   }
   else {
      stat = encXmlReal10(pctxt, pvalue);
   }

   if ( stat != 0 ) return LOG_RTERR(pctxt, stat);

   if (0 != elemName) {
      stat = rtXmlEncEndElement (pctxt, elemName, pNS);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}
