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

#include "rtxsrc/rtxDecimal.h"
#include "rtxsrc/rtxMemory.h"

EXTRTMETHOD const char* rtxNR3toDecimal (OSCTXT *pctxt, const char* object_p)
{
   const char* p;
   char* pd;
   const char* mantBeg = 0;
   const char* mantEnd = 0;
   int intDigits = 0;
   int fracDigits = 0;
   int expVal = 0;
   int totalDigits = 0;
   int nmZeros = 0;
   char mantMinus = 0;
   char expMinus = 0;
   char c;

   /* skip ws and leading zeros and plus */
   for (p = object_p; (c = *p) != 0; p++) {
      if (c == '-')
         mantMinus = 1;
      else if (c != ' ' && c != '0' && c != '+')
         break;
   }

   mantBeg = mantEnd = p;

   /* count mantissa integer digits */
   for (; (c = *p) != 0; p++) {
      if (c >= '0' && c <= '9') {
         intDigits ++;
         mantEnd = p + 1;
      }
      else if (c == '.') {
         p++;
         break;
      }
      else if (c != ' ')
         break;
   }

   /* count mantissa fractional digits */
   for (; (c = *p) != 0; p++) {
      if (c >= '0' && c <= '9') {
         fracDigits ++;
         mantEnd = p + 1;
      }
      else if (c == 'e' || c == 'E') {
         break;
      }
      else if (c != ' ')
         break;
   }

   if (*p == 'e' || *p == 'E')
      p++;
   else /* already decimal format */
      return object_p;

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

   totalDigits = intDigits + fracDigits;

   if (expVal < 0) {
      intDigits += expVal;
      if (intDigits < 0) {
         nmZeros = -intDigits;
         intDigits = 0;
      }
   }
   else if (expVal > 0) {
      fracDigits -= expVal;
      if (fracDigits < 0) {
         nmZeros = -fracDigits;
         fracDigits = 0;
      }
   }

   totalDigits += nmZeros;
   if (fracDigits > 0) totalDigits++; /* point */
   if (intDigits == 0) totalDigits++; /* leading 0 */
   if (mantMinus) totalDigits++;

   p = (const char*) rtxMemAlloc (pctxt, totalDigits + 1);
   if (p == 0) return 0;

   pd = (char*) p;

   if (mantMinus) *pd++ = '-';
   if (intDigits == 0) {
      *pd++ = '0';
      *pd++ = '.';
      intDigits--;
      }

   if (expVal < 0) {
      while (nmZeros--) *pd++ = '0';
   }

   while (mantBeg != mantEnd) {
      c = *mantBeg++;
      if (c >= '0' && c<= '9') {
         if (intDigits-- == 0)
            *pd++ = '.';

         *pd++ = c;
      }
   }

   if (expVal > 0) {
      while (nmZeros--) *pd++ = '0';
   }

   *pd = 0;
   rtxMemFreePtr (pctxt, (void*)object_p);
   return p;
}
