/*
 * Copyright (c) 1997-2018 Objective Systems, Inc.
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

#include "xee_common.hh"

int xerEncReal10 (OSCTXT* pctxt, const OSUTF8CHAR* pvalue, const char* elemName)
{
   const char* pcharvalue = (const char*) pvalue;
   int  stat;
   const char* p;
   char* pExp;
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
   char expBuf[8];

   if (0 == elemName) elemName = "REAL";

   stat = xerEncStartElement (pctxt, elemName, 0);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   pctxt->state = XERDATA;

   if (!(pctxt->flags & ASN1CANXER)) {
      stat = xerPutCharStr (pctxt, pcharvalue, strlen (pcharvalue));
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else {
      /* skip ws and leading zeros and plus */
      for (p = pcharvalue; (c = *p) != 0; p++) {
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
            stat = xerPutCharStr (pctxt, &c, 1);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }

         stat = xerPutCharStr (pctxt, mantBeg, 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         mantBeg++;

         if (digits == 1) {
            stat = xerPutCharStr (pctxt, ".0E", 3);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }
         else {
            c ='.';
            stat = xerPutCharStr (pctxt, &c, 1);
            if (stat != 0) return LOG_RTERR (pctxt, stat);

            while (mantBeg != mantEnd) {
               c = *mantBeg++;
               if (c >= '0' && c <= '9') {
                  stat = xerPutCharStr (pctxt, &c, 1);
                  if (stat != 0) return LOG_RTERR (pctxt, stat);
               }
            }

            c ='E';
            stat = xerPutCharStr (pctxt, &c, 1);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }

         /* put exponent */
         if (expMinus) {
            c ='-';
            stat = xerPutCharStr (pctxt, &c, 1);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }

         pExp = expBuf + sizeof (expBuf);
         do {
            *(--pExp) = (expVal % 10) + '0';
            expVal /= 10;
         } while (expVal);

         stat = xerPutCharStr (pctxt, pExp,
            expBuf + sizeof (expBuf) - pExp);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      else { /* encode 0 */
         c ='0';
         stat = xerPutCharStr (pctxt, &c, 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
   }

   stat = xerEncEndElement (pctxt, elemName);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return (0);
}
