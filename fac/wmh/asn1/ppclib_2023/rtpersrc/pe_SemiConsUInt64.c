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

#ifndef _NO_INT64_SUPPORT

#include "rtpersrc/pe_common.hh"
#include "rtxsrc/rtxDiag.h"

/***********************************************************************
 *
 *  Routine name: pe_<integer>
 *
 *  Description:  The following functions encode integers.  Different
 *                routines handle signed/unsigned and constrained/
 *                unconstrained variations.  There is also a function
 *                to handle big (> 32 bit) integers.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  value       int     Integer value to be encoded
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  length      uint    Number of octets required
 *
 **********************************************************************/

EXTPERMETHOD int pe_SemiConsUInt64
(OSCTXT* pctxt, OSUINT64 value, OSUINT64 lower)
{
   /*
   To determine how many bytes we must encode for the value, shift all but the
   highest 8 bits, then 16, 24, etc. until we find the highest byte that is
   nonzero.

   invariant: all bits above the lowest shift + 8 bits are zero bits (at first,
   this involves imaginary bits above the number itself).
   */
   int nbytes, stat;
   int shift = (sizeof(value) - 1) * 8;

   OSUINT64 tempValue;

   value -= lower;

   /* If adjusted value is zero, encode a zero length value and return */

   if (0 == value) {
      return pe_Length (pctxt, 0);
   }

   /* Calculate unsigned number value length */

   for ( ; shift > 0; shift -= 8) {
      /* Given the invariant, value >> shift will give the 8 bits above the
         lowest shift bits; all higher bits are zero bits.
      */
      tempValue = value >> shift;

      RTDIAGSTRM4 (pctxt, "pe_SemiConsUInt64: shift is %d, "
                   "tempValue is 0x%x\n", shift, tempValue);

      if (tempValue == 0) {
         /* We now know the bits above the lowest shift bits are all zero.
            The invariant will be maintaned when shift is decreased by 8.
         */
         continue;
      }
      else {
         /* The 8 bits above the lowest shift bits are not all zero.
            By the invariant, the lowest shift + 8 bits contain all of the
            non-zero bits of the number.
         */
         break;
      }
   }

   nbytes = shift / 8 + 1;
   RTDIAGSTRM3 (pctxt,"pe_SemiConsUInt64: nbytes is %d\n", nbytes);

   /* Encode length */

   if ((stat = pe_Length (pctxt, nbytes)) < 0) {
      return LOG_RTERR (pctxt, stat);
   }

   PU_NEWFIELD (pctxt, "SemiConsUInt64");

   if (pctxt->buffer.aligned) {
      if ((stat = pe_byte_align (pctxt)) != 0)
         return LOG_RTERR (pctxt, stat);
   }

   /* Encode unsigned value */

   stat = pe_NonNegBinInt64 (pctxt, value);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   PU_SETBITCOUNT (pctxt);

   return 0;
}


EXTPERMETHOD int pe_SemiConsUInt64SignedBound
(OSCTXT* pctxt, OSUINT64 value, OSINT64 lower)
{
   if ( lower > 0 && value < (OSUINT64) lower )
      return LOG_RTERR(pctxt, RTERR_CONSVIO);
   else {
      /* lower <= value, possibly with lower < 0 so that value - lower > lower
      if lower >= 0, value - lower will not overflow (since value >= lower)
      if lower < 0, overflow happened if value - lower <= value
      */
      OSUINT64 adjValue = value - lower;

      if ( lower < 0 && adjValue <= value ) {
         /* The adjusted value is outside the bounds of OSUINT64. */
         return LOG_RTERR(pctxt, RTERR_TOOBIG);
      }
      /* else no overflow */

      return pe_SemiConsUInt64(pctxt, adjValue, 0);
   }
}


EXTPERMETHOD int pe_UnconsUInt64Aligned (OSCTXT* pctxt, OSUINT64 value)
{
   /* Encode value as an unconstrained whole number. */

   OSUINT64 mask = OSUI64CONST(1) << ((sizeof(value) * 8) - 1);

   if ((value & mask) == 0) {
      /* The most significant bit is 0, we can simply cast to OSINT64 */
      /* and use pe_UnconsInt64 */
      return pe_UnconsInt64Aligned(pctxt, (OSINT64) value);
   }
   else {
      /* The most significant bit is 1.  We can encode this as a 2's
         complement binary integer by encoding the length, a zero byte,
         and then the value. */

      int stat;

      /* Encode length */

      PU_NEWFIELD (pctxt, "length");

      stat = pe_octetAligned (pctxt, (OSUINT8)(sizeof(value) + 1));
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      PU_SETBITCOUNT (pctxt);

      /* Encode the leading zero byte */
      PU_NEWFIELD (pctxt, "UnconsUInt64");

      stat = pe_octetAligned (pctxt, 0);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      /* Encode the value */
      stat = pe_bits64(pctxt, value, sizeof(value) * 8);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      PU_SETBITCOUNT (pctxt);

      return 0;
   }
}

EXTPERMETHOD int pe_UnconsUInt64Unaligned (OSCTXT* pctxt, OSUINT64 value)
{
   /* Encode value as an unconstrained whole number. */

   OSUINT64 mask = OSUI64CONST(1) << ((sizeof(value) * 8) - 1);

   if ((value & mask) == 0) {
      /* The most significant bit is 0, we can simply cast to OSINT64 */
      /* and use pe_UnconsInt64 */
      return pe_UnconsInt64Unaligned(pctxt, (OSINT64)value);
   }
   else {
      /* The most significant bit is 1.  We can encode this as a 2's
         complement binary integer by encoding the length, a zero byte,
         and then the value. */

      int stat;

      /* Encode length */

      PU_NEWFIELD (pctxt, "length");

      stat = pe_octet(pctxt, (OSUINT8)(sizeof(value) + 1));
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      PU_SETBITCOUNT (pctxt);

      /* Encode the leading zero byte */
      PU_NEWFIELD (pctxt, "UnconsUInt64");

      stat = pe_octet(pctxt, 0);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      /* Encode the value */
      stat = pe_bits64(pctxt, value, sizeof(value) * 8);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      PU_SETBITCOUNT (pctxt);

      return 0;
   }
}

EXTPERMETHOD int pe_UnconsUInt64 (OSCTXT* pctxt, OSUINT64 value)
{
   return (pctxt->buffer.aligned) ?
      pe_UnconsUInt64Aligned (pctxt, value) :
      pe_UnconsUInt64Unaligned (pctxt, value);
}

#endif
