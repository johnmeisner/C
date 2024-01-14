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

#include <math.h>
#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxErrCodes.h"

#define LG2 0.30102999566398119521373889472449
#define PRECISION 15 /* the default number of fraction
                        digits for normalized double */
#define FRACTIONDIGITS 10

EXTXMLMETHOD int rtXmlEncDecimalValue (OSCTXT* pctxt, OSREAL value,
                          const OSDecimalFmt* pFmtSpec,
                          char* pDestBuf, size_t destBufSize)
{
   char lbuf[512];
   double mantissa;
   int exponent, sign = 1;
   int ch, stat;
   char *p = lbuf, *firstZeroInFractionPtr = 0;
   int totalDigits = PRECISION + 1;
   int integerMaxDigits = INT_MAX, integerMinDigits = 0;
   int fractionDigits = FRACTIONDIGITS;
   int fractionDigitsNow = 0, fractionMinDigits = 0;
   OSXMLCtxtInfo* xmlCtxt;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   xmlCtxt = ((OSXMLCtxtInfo*)pctxt->pXMLInfo);

   /* handle special values - for decimals just return error */

   if (rtxIsPlusInfinity (value)) {
      return LOG_RTERRNEW (pctxt, RTERR_INVREAL);
   }
   else if (rtxIsMinusInfinity (value)) {
      return LOG_RTERRNEW (pctxt, RTERR_INVREAL);
   }
   else if (rtxIsNaN (value)) {
      return LOG_RTERRNEW (pctxt, RTERR_INVREAL);
   }

   if (value < 0) {
      sign = -1;
      value = -value;
   }

   if (pFmtSpec != 0) {
      if (pFmtSpec->totalDigits > 0)
         totalDigits = pFmtSpec->totalDigits;
      if (pFmtSpec->fractionDigits >= 0)
         fractionDigits = pFmtSpec->fractionDigits;
      if (pFmtSpec->integerMaxDigits >= 0)
         integerMaxDigits = pFmtSpec->integerMaxDigits;
      if (pFmtSpec->integerMinDigits >= 0)
         integerMinDigits = pFmtSpec->integerMinDigits;
      if (pFmtSpec->fractionMinDigits >= 0)
         fractionMinDigits = pFmtSpec->fractionMinDigits;
   }

   /* check facets set in xmlCtxt. These facets are set if fractionDigit
      and/or totalDigits facet are used in schema. */
   if (xmlCtxt->facets.fractionDigits >= 0) {
      fractionDigits = xmlCtxt->facets.fractionDigits;
      xmlCtxt->facets.fractionDigits = -1;
   }
   if (xmlCtxt->facets.totalDigits >= 0) {
      totalDigits = xmlCtxt->facets.totalDigits;
      xmlCtxt->facets.totalDigits = -1;
   }

   mantissa = frexp (value, &exponent);

   /* value = mantissa * 2^exponent */
   /* We need convert it to mantissa2 * 10^exp2 */

   /* calculate new exponent first.
      Assume, 2^exponent = 10^exp2. In this case exp2 = n*lg 2 */

   exponent = (int)(LG2 * (double)exponent);

   /* calculate new mantissa. mantissa2 = value/(10^exp2) */

   if (exponent == 0)
      mantissa = value; /* 10^0 = 1 */
   else
      mantissa = value/pow (10.0, exponent);

   /* now, normalize the mantissa, if necessary*/

   if (mantissa != 0.0) {

      while (mantissa >= 10.0) {
         mantissa = mantissa / 10.0;
         exponent ++;
      }
      while (mantissa < 1.0) {
         mantissa = 10.0 * mantissa;
         exponent --;
      }
   }

   if (totalDigits < 0)
      totalDigits = PRECISION + 1;

   /* round up the mantissa accordingly to precission */

   if (mantissa != 0.0) {
      double correndum;
      int corrpow;

      if (exponent > totalDigits - 1)
         corrpow = -totalDigits + 1;
      else if (fractionDigits == INT_MAX) {
         corrpow = -PRECISION;
      }
      else if (exponent > 0)
         corrpow = -((fractionDigits + exponent < totalDigits) ?
            fractionDigits + exponent : totalDigits - 1);
      else
         corrpow = -((fractionDigits < totalDigits) ?
                     fractionDigits : totalDigits);

      correndum = 0.5 * pow (10.0, corrpow);
      mantissa += correndum;

      /* here we need to perform one-step normalization of the mantissa.
         It might be necessary in some cases, when mantissa becomes
         greater than 1.0 after addition of correndum. For example, it
         happens, if value == 0.01 */

      if (mantissa >= 10.0) {
         mantissa = mantissa / 10.0;
         exponent ++;
      }
      if (mantissa < 1.0) {
         mantissa = 10.0 * mantissa;
         exponent --;
      }
   }

   /* Handle sign */

   if (sign < 0 || (pFmtSpec != 0 && pFmtSpec->signPresent)) {
      *p++ = (char)((sign < 0) ? '-' : '+');
   }

   /* Handle leading zeroes */

   if (exponent < 0) {

      if (integerMaxDigits > 0) {
         int i = (integerMinDigits > 0) ? integerMinDigits : 1;
         for (; i > 0; i--) {
            *p++ = '0';
         }
      }
      *p++ = '.';

      while (++exponent) {
         *p++ = '0';
         fractionDigitsNow++;
      }
      exponent--;
   }
   else {
      int i = integerMinDigits - (exponent + 1);
      for (; i > 0; i--) {
         *p++ = '0';
      }
   }

   /* The real work */
   do {

      ch = (int)mantissa;
      mantissa = mantissa - ch;

      *p++ = (char)(ch + '0');

      mantissa = 10.0 * mantissa;

      if (exponent < 0) {
         fractionDigitsNow++;
         if (ch == 0) {
            if (firstZeroInFractionPtr == 0)
               firstZeroInFractionPtr = p - 1;
         }
         else firstZeroInFractionPtr = 0;
      }
      if (0 == exponent--) {
         *p++ = '.';
      }

   } while (--totalDigits > 0 && fractionDigitsNow < fractionDigits);

   /* handle trailing zeroes */
   while (exponent >= 0) {

      *p++ = '0';

      if (0 == exponent--) {
         *p++ = '.';
      }
   }

   /* remove extra zeros in fraction part */

   if (firstZeroInFractionPtr != 0 && fractionDigitsNow > fractionMinDigits) {
      int z = fractionDigitsNow - (int)(p - firstZeroInFractionPtr);
      if (fractionMinDigits > z) {
         firstZeroInFractionPtr += (fractionMinDigits - z);
         fractionDigitsNow -= (int)(p - firstZeroInFractionPtr);
      }
      exponent += (int)(p - firstZeroInFractionPtr);
      p = firstZeroInFractionPtr;
   }

   /* handle trailing zeros in fraction part, if necessary */

   while (fractionDigitsNow < fractionMinDigits) {
      *p++ = '0';
      fractionDigitsNow++;
      exponent--;
   }

   /* Handle isolated '.' at end */

   if (-1 == exponent) {
      if (pFmtSpec == 0 || !pFmtSpec->pointPresent)
         p--; /* eliminate the decimal point */
   }

   *p = '\0';

   if (destBufSize == 0) {
      stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)lbuf, OSCRTLSTRLEN(lbuf));
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else {
      if (p + 1 - lbuf > (long)destBufSize)
         return LOG_RTERRNEW (pctxt, RTERR_BUFOVFLW);

      rtxStrcpy (pDestBuf, destBufSize, lbuf);
   }

   return (0);
}


