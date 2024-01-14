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

#ifndef _NO_ASN1REAL

#include <math.h>
#include <stdlib.h>
#include "rtpersrc/asn1per.h"
#include "rtxsrc/rtxReal.h"
#include "rtxsrc/rtxUtil.h"
/* START NOOSS */
#ifdef RTEVAL
#define _CRTLIB
#include "rtxevalsrc/rtxEval.hh"
#else
#define OSRT_CHECK_EVAL_DATE_STAT0(pctxt,stat)
#define OSRT_CHECK_EVAL_DATE_STAT1(pctxt,stat)
#define OSRT_CHECK_EVAL_DATE0(pctxt)
#define OSRT_CHECK_EVAL_DATE1(pctxt)
#endif /* RTEVAL */
/* END NOOSS */

/***********************************************************************
 *
 *  Routine name: pe_Real
 *
 *  Description:  The following function encodes a real value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *  data        float   real value to be encoded
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     completion status of encode operation
 *
 **********************************************************************/

EXTPERMETHOD int pe_Real (OSCTXT* pctxt, OSREAL value)
{
   int            len, stat;
   double         mant, tmp;
   unsigned int   imant, last = 0, mult;
   int            e, sign;
   unsigned char  buf[sizeof(double)];
   int            i, ll, mantLen;
   OSUINT32       b, firstOctet;

   /* special real values for +/- oo, NAN, -0 */
   if (rtxIsMinusInfinity(value)) {
      if ((stat = pe_Length (pctxt, 1)) < 0) {
         return LOG_RTERR (pctxt, stat);
      }

      PU_NEWFIELD (pctxt, "Real");

      b = ASN1_K_MINUS_INFINITY;
      if ((stat = pe_bits (pctxt, b, 8)) != 0) {
         return LOG_RTERR (pctxt, stat);
      }
   }
   else if (rtxIsPlusInfinity(value)) {
      if ((stat = pe_Length (pctxt, 1)) < 0) {
         return LOG_RTERR (pctxt, stat);
      }

      PU_NEWFIELD (pctxt, "Real");

      b = ASN1_K_PLUS_INFINITY;
      if ((stat = pe_bits (pctxt, b, 8)) != 0) {
         return LOG_RTERR (pctxt, stat);
      }
   }
   else if (rtxIsNaN(value)) {
      if ((stat = pe_Length (pctxt, 1)) < 0) {
         return LOG_RTERR (pctxt, stat);
      }

      PU_NEWFIELD (pctxt, "Real");

      b = ASN1_K_NOT_A_NUMBER;
      if ((stat = pe_bits (pctxt, b, 8)) != 0) {
         return LOG_RTERR (pctxt, stat);
      }
   }
   else if (rtxIsMinusZero(value)) {
      if ((stat = pe_Length (pctxt, 1)) < 0) {
         return LOG_RTERR (pctxt, stat);
      }

      PU_NEWFIELD (pctxt, "Real");

      b = ASN1_K_MINUS_ZERO;
      if ((stat = pe_bits (pctxt, b, 8)) != 0) {
         return LOG_RTERR (pctxt, stat);
      }
   }
   else if (value != 0.0) {
      /* encode a binary real value */
      len = 1;

      /* this is what frexp gets from *value:
       * *value == mantissa * 2 ^ e
       * where 0.5 <= |manitissa| < 1.0  */

      mant = frexp (value, &e);

      /* set sign and make mantissa = | mantissa | */
      if (mant < 0.0) {
         sign = -1;
         mant *= -1;
      }
      else
         sign = 1;

      tmp = mant;

      /* calculate first multiplier to produce normalized REAL.
       * Normalized REAL is necessary for PER encoding as it uses DER/CER
       * encoding rules for REAL type.
       * REAL is normalized, if mantissa is either zero or odd. //!AB
       */
      for (i = 0; i < (int)sizeof(double); i++) {
         /* shift 16 bits worth to the left of the decimal */
         tmp *= (1L << 16);

         /* grab only (octet sized) the integer part */
         imant = (unsigned int) tmp;

         /* remove part to left of decimal now for next iteration */
         tmp -= imant;

         if (imant) last = imant;
         else if (tmp == 0.0) break;
      }

      /* convert mantissa into an unsigned integer */
      mult = (last & 0xFF) ? rtxOctetBitLen ((OSOCTET)(last & 0xFF)) :
            rtxOctetBitLen ((OSOCTET)(last >> 8));
      mantLen = 1;
      /* shift 'mult' bits worth to the left of the decimal */
      tmp = mant * (1L << mult);
      e -= mult; /* correct exponent */

      /* convert mantissa into an unsigned integer */

      for (i = 0; i < (int)sizeof(double); i++) {
         /* grab only (octet sized) the integer part */
         imant = (unsigned int) tmp;

         /* remove part to left of decimal now for next iteration */
         tmp -= imant;

         /* write into tmp buffer */
         buf[i] = (unsigned char) imant;

         if (tmp == 0.0)
            break;

         /* shift 8 bits worth to the left of the decimal (for the next iteration)*/
         tmp *= (1L << 8);
         mantLen ++;
         e -= 8; /* correct exponent */
      }
      len += mantLen;

      /*
       * write format octet  (first octet of content)
       *  field  1 S bb ff ee
       *  bit#   8 7 65 43 21
       *
       * 1 in bit#1 means binary rep
       * 1 in bit#2 means the mantissa is neg, 0 pos
       * bb is the base:    65  base
       *                    00    2
       *                    01    8
       *                    10    16
       *                    11    future ext.
       *
       * ff is the Value of F where  Mantissa = sign x N x 2^F
       *    FF can be one of 0 to 3 inclusive. (used to save re-alignment)
       *
       * ee is the length of the exponent:  21   length
       *                                    00     1
       *                                    01     2
       *                                    10     3
       *                                    11     long form
       *
       *
       * encoded binary real value looks like
       *
       *     fmt oct
       *   --------------------------------------------------------
       *   |1Sbbffee|  exponent (2's comp)  |   N (unsigned int)  |
       *   --------------------------------------------------------
       *    87654321
       */

      firstOctet = REAL_BINARY;
      if (sign == -1)
         firstOctet |= REAL_SIGN;

      /* bb is 00 since base is 2 so do nothing */
      /* ff is 00 since no other shifting is nec */

      ll = rtxIntByteCount (e); /* calc length of exponent (in bytes) */
      if (ll > 0) {
         len += ll;
         switch (ll) {
         case (1): firstOctet |= REAL_EXPLEN_1; break;
         case (2): firstOctet |= REAL_EXPLEN_2; break;
         case (3): firstOctet |= REAL_EXPLEN_3; break;
         default:
            firstOctet |= REAL_EXPLEN_LONG;
            len ++;
            break;
         }
      }

      if ((stat = pe_Length (pctxt, len)) < 0) {
         return LOG_RTERR (pctxt, stat);
      }

      PU_NEWFIELD (pctxt, "Real");

      /* write the format octet */

      if ((stat = pe_bits (pctxt, firstOctet, 8)) != 0) {
         return LOG_RTERR (pctxt, stat);
      }
      if ((firstOctet & REAL_EXPLEN_MASK) == REAL_EXPLEN_LONG) {
         /* write exponent length */

         if ((stat = pe_bits (pctxt, ll, 8)) != 0)
            return LOG_RTERR (pctxt, stat);
      }

      /* write the exponent */

      if (ll > 0) {
         if ((stat = pe_bits (pctxt, e, ll * 8)) != 0)
            return LOG_RTERR (pctxt, stat);
      }

      /* write the mantissa (N value) */

      if ((stat = pe_octets (pctxt, buf, mantLen * 8)) != 0)
         return LOG_RTERR (pctxt, stat);
   }
   else {
      /* no contents for 0.0 reals */
      if ((stat = pe_Length (pctxt, 0)) < 0) /* zero length */
         return LOG_RTERR (pctxt, stat);

      PU_NEWFIELD (pctxt, "Real");
   }

   PU_SETBITCOUNT (pctxt);

   OSRT_CHECK_EVAL_DATE1 (pctxt);

   return 0;
}

#endif
