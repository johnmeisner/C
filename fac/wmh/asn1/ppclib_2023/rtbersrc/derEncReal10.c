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

#include "rtbersrc/asn1ber.h"

int xe_derReal10 (OSCTXT *pctxt, const char* object_p, ASN1TagType tagging)
{
   int ll0 = 0;
   int stat;
   const char* p;
   const char* mantBeg = 0;
   const char* mantEnd = 0;
   int digits = 0;
   int trailZeros = 0;
   int mantExpVal = 0;
   int expVal = 0;
   int tm;
   char mantMinus = 0;
   char expMinus = 0;
   char flFrac = 0;
   char c;

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

      expVal -= digits;

      /* put exponent */
      if (expVal != 0)  {
         if (expVal < 0) {
            expMinus = 1;
            expVal = -expVal;
         }
         else
            expMinus = 0;

         while (expVal) {
            c = expVal % 10;
            expVal /= 10;
            XE_SAFEPUT1(pctxt, '0' + c);
            ll0++;
         }

         if (expMinus) {
            XE_SAFEPUT1(pctxt, '-');
            ll0++;
         }
      }
      else {
         XE_CHKBUF(pctxt, 2)
         XE_PUT1(pctxt, '0')
         XE_PUT1(pctxt, '+')
         ll0 += 2;
      }

      tm = digits + 3 + mantMinus;
      XE_CHKBUF(pctxt, (unsigned)(tm))
      ll0 += tm;

      XE_PUT1(pctxt, 'E');
      XE_PUT1(pctxt, '.');

      do {
         c = *(--mantEnd);
         if (c < '0' || c > '9')
            continue;

         XE_PUT1(pctxt, c);
      } while (mantEnd != mantBeg);

      if (mantMinus)
         XE_PUT1(pctxt, '-');

      XE_PUT1(pctxt, 3)
   }

   if (tagging == ASN1EXPL) {
      stat = xe_tag_len (pctxt, TM_UNIV|TM_PRIM|9, ll0);
      if (stat < 0) return LOG_RTERR (pctxt, stat);
      ll0 = stat;
   }

   return ll0;
}
