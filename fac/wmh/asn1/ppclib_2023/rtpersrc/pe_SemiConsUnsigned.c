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

#include "rtpersrc/pe_common.hh"

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

EXTPERMETHOD int pe_UnconsUInt32Aligned (OSCTXT* pctxt, OSUINT32 value)
{
   /* Encode value as an unconstrained whole number. */

   OSUINT32 mask = 1UL << ((sizeof(value) * 8) - 1);

   if ((value & mask) == 0) {
      /* The most significant bit is 0, we can simply cast to OSINT32 */
      /* and use pe_UnconsInteger */
      return pe_UnconsInt32Aligned(pctxt, (OSINT32)value);
   }
   else {
      /* The most significant bit is 1.  We can encode this as a 2's
         complement binary integer by encoding the length, a zero byte,
         and then the value. */

      OSSIZE nbytes = sizeof(value) + 1; /*  + 1 for the leading zero byte */
      int stat;

      /* Encode length */
      PU_NEWFIELD (pctxt, "length");

      stat = pe_octetAligned (pctxt, (OSUINT8)nbytes);
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      PU_SETBITCOUNT (pctxt);

      /* Encode unsigned value */

      PU_NEWFIELD (pctxt, "UnconsUnsigned");

      /* encode the leading zero byte */
      stat = pe_octetAligned(pctxt, 0);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      /* encode the value */
      stat = pe_bits(pctxt, value, sizeof(value) * 8);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      PU_SETBITCOUNT (pctxt);

      return 0;
   }
}

EXTPERMETHOD int pe_UnconsUInt32Unaligned (OSCTXT* pctxt, OSUINT32 value)
{
   /* Encode value as an unconstrained whole number. */

   OSUINT32 mask = 1UL << ((sizeof(value) * 8) - 1);

   if ((value & mask) == 0) {
      /* The most significant bit is 0, we can simply cast to OSINT32 */
      /* and use pe_UnconsInteger */
      return pe_UnconsInt32Unaligned(pctxt, (OSINT32)value);
   }
   else {
      /* The most significant bit is 1.  We can encode this as a 2's
         complement binary integer by encoding the length, a zero byte,
         and then the value. */

      OSSIZE nbytes = sizeof(value) + 1; /*  + 1 for the leading zero byte */
      int stat;

      /* Encode length */
      PU_NEWFIELD (pctxt, "length");

      stat = pe_octet(pctxt, (OSUINT8)nbytes);
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      PU_SETBITCOUNT (pctxt);

      /* Encode unsigned value */

      PU_NEWFIELD (pctxt, "UnconsUnsigned");

      /* encode the leading zero byte */
      stat = pe_octet(pctxt, 0);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      /* encode the value */
      stat = pe_bits(pctxt, value, sizeof(value) * 8);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      PU_SETBITCOUNT (pctxt);

      return 0;
   }
}

EXTPERMETHOD int pe_UnconsUInt32 (OSCTXT* pctxt, OSUINT32 value)
{
   return (pctxt->buffer.aligned) ?
      pe_UnconsUInt32Aligned (pctxt, value) :
      pe_UnconsUInt32Unaligned (pctxt, value);
}
