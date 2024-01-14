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

#include "rtbersrc/asn1berStream.h"

int cerEncStrmReal10 (OSCTXT *pctxt, const char* object_p, ASN1TagType tagging)
{
   int ll0 = 0;
   int stat;
   const char* p;
   char* pExp = 0;
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

   if (!object_p)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   /* skip ws and leading zeros and plus */
   for (p = object_p; (c = *p) != 0; p++) {
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

   if ((ll0 = digits) > 0) {
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

      expVal -= digits;

      if (expVal < 0) {
         expMinus = 1;
         expVal = -expVal;
      }
      else
         expMinus = 0;

      /* count exponent length */
      if (expVal != 0)  {
         int tm = expVal;
         pExp = expBuf + sizeof (expBuf);

         while (tm) {
            *(--pExp) = (tm % 10) + '0';
            tm /= 10;
            ll0++;
         }
      }
      else {
         ll0 += 2;
      }

      ll0 += 3 + expMinus + mantMinus;

      if (tagging == ASN1EXPL) {
         stat = berEncStrmTagAndLen (pctxt, TM_UNIV|TM_PRIM|9, ll0);
         if (stat < 0) return LOG_RTERR (pctxt, stat);
      }

      /* real10 NR3 marker */
      stat = berEncStrmWriteOctet (pctxt, 3);
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      /* put mantissa */
      if (mantMinus) {
         stat = berEncStrmWriteOctet (pctxt, '-');
         if (stat < 0) return LOG_RTERR (pctxt, stat);
      }

      while (mantBeg != mantEnd) {
         c = *mantBeg++;
         if (c >= '0' && c<= '9') {
            stat = berEncStrmWriteOctet (pctxt, c);
            if (stat < 0) return LOG_RTERR (pctxt, stat);
         }
      }

      stat = berEncStrmWriteOctet (pctxt, '.');
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      stat = berEncStrmWriteOctet (pctxt, 'E');
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      /* put exponent */
      if (expVal != 0)  {
         if (expMinus) {
            stat = berEncStrmWriteOctet (pctxt, '-');
            if (stat < 0) return LOG_RTERR (pctxt, stat);
         }

         stat = berEncStrmWriteOctets (pctxt, (OSOCTET*)pExp,
            expBuf + sizeof (expBuf) - pExp);
         if (stat < 0) return LOG_RTERR (pctxt, stat);
      }
      else {
         stat = berEncStrmWriteOctet (pctxt, '+');
         if (stat < 0) return LOG_RTERR (pctxt, stat);
         stat = berEncStrmWriteOctet (pctxt, '0');
         if (stat < 0) return LOG_RTERR (pctxt, stat);
      }
   }
   else { /* encode 0 */
      if (tagging == ASN1EXPL) {
         stat = berEncStrmTagAndLen (pctxt, TM_UNIV|TM_PRIM|9, 0);
         if (stat < 0) return LOG_RTERR (pctxt, stat);
      }
   }

   return 0;
}
