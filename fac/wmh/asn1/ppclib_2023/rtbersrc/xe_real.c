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

#include <math.h>
#include <stdlib.h>
#include "rtbersrc/asn1ber.h"
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
 *  Routine name: xe_real
 *
 *  Description:  This routine encodes an ASN.1 real value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *  object	real	Real value to be encoded.
 *  tagging     enum    Explicit or implicit tagging specification.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  aal         int     Overall message length.  Returned as function
 *                      result.  Will be negative if encoding fails.
 *
 **********************************************************************/

int xe_real (OSCTXT* pctxt, OSREAL *object_p, ASN1TagType tagging)
{
   double         mant, tmp;
   unsigned int   imant, last = 0, mult;
   int            e, sign;
   unsigned char  buf[sizeof(double)];
   int            aal, i, ll = 0, mantLen;
   OSOCTET        b, firstOctet;

   /* special real values for +/- oo, NAN, -0 */
   if (rtxIsMinusInfinity(*object_p)) {
      b = ASN1_K_MINUS_INFINITY;
      aal = xe_memcpy (pctxt, &b, 1);
   }
   else if (rtxIsPlusInfinity(*object_p)) {
      b = ASN1_K_PLUS_INFINITY;
      aal = xe_memcpy (pctxt, &b, 1);
   }
   else if (rtxIsNaN(*object_p)) {
      b = ASN1_K_NOT_A_NUMBER;
      aal = xe_memcpy (pctxt, &b, 1);
   }
   else if (rtxIsMinusZero(*object_p)) {
      b = ASN1_K_MINUS_ZERO;
      aal = xe_memcpy (pctxt, &b, 1);
   }
   else if (*object_p == 0.0) {
      /* no contents for 0.0 reals */
      aal = 0;
   }
   else {
      /* encode a binary real value */
      /* this is what frexp gets from *value:
       * *value == mantissa * 2 ^ e
       * where 0.5 <= |manitissa| < 1.0  */

      mant = frexp (*object_p, &e);

      /* set sign and make mantissa = | mantissa | */
      if (mant < 0.0) {
         sign = -1;
         mant *= -1;
      }
      else
         sign = 1;

      tmp = mant;

      /* calculate first multiplier to produce normalized REAL.
       * Normalized REAL is necessary for DER/CER.
       * REAL is normalized, if mantissa is either zero or odd. !AB */
      for (i = 0; i < (int)sizeof(double); i++) {
         /* shift 16 bits worth to the left of the decimal */
         tmp *= (OSINTCONST(1) << 16);

         /* grab only (octet sized) the integer part */
         imant = (unsigned int) tmp;

         /* remove part to left of decimal now for next iteration */
         tmp -= imant;

         if (imant) last = imant;
         else if (tmp == 0.0 ) break;    /* no more bits remain */
      }

      /* convert mantissa into an unsigned integer */
      mult = (last & 0xFF) ? rtxOctetBitLen ((OSOCTET)(last & 0xFF)) :
         rtxOctetBitLen ((OSOCTET)(last >> 8));
      mantLen = 1;
      /* shift 'mult' bits worth to the left of the decimal */
      tmp = mant * (OSINTCONST(1) << mult);
      e -= mult; /* correct exponent */

      for (i = 0; i < (int)sizeof(double); i++) {
         /* grab only (octet sized) the integer part */
         imant = (unsigned int) tmp;

         /* remove part to left of decimal now for next iteration */
         tmp -= imant;

         /* write into tmp buffer */
         buf[i] = (unsigned char) imant;

         if (tmp == 0.0)
            break;

         /* shift 8 bits worth to the left of the decimal
            (for the next iteration) */
         tmp *= (OSINTCONST(1) << 8);
         mantLen ++;
         e -= 8; /* correct exponent */
      }

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

      /* write the mantissa (N value) */
      aal = xe_memcpy (pctxt, buf, mantLen);

      /* write the exponent */
      if (aal > 0) {
         aal = ((ll = xe_integer (pctxt, &e, ASN1IMPL)) > 0) ? aal + ll : ll;
      }

      /* Set exponent length in first octet; write long form byte if 	*/
      /* length > 3..							*/

      if (aal > 0) {
         switch (ll) {
            case (1): firstOctet |= REAL_EXPLEN_1; break;
            case (2): firstOctet |= REAL_EXPLEN_2; break;
            case (3): firstOctet |= REAL_EXPLEN_3; break;
            default:
               firstOctet |= REAL_EXPLEN_LONG;
               b = (OSOCTET) ll;  /* exponent length */
               aal = ((ll = xe_memcpy (pctxt, &b, 1)) > 0) ? aal + ll : ll;
               break;
         }
      }

      /* write the format octet */

      if (aal > 0) {
         aal = ((ll = xe_memcpy (pctxt, &firstOctet, 1)) > 0) ? aal + ll : ll;
      }
   }

   if (tagging == ASN1EXPL)
      aal = xe_tag_len (pctxt, TM_UNIV|TM_PRIM|ASN_ID_REAL, aal);

   OSRT_CHECK_EVAL_DATE1 (pctxt);

   return (aal);
}

