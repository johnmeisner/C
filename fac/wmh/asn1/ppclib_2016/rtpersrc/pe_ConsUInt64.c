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

#ifndef _NO_INT64_SUPPORT

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

EXTPERMETHOD int pe_ConsUInt64 (OSCTXT* pctxt,
                   OSUINT64 value, OSUINT64 lower, OSUINT64 upper)
{
   OSUINT64 range_value;
   OSUINT64 adjusted_value;
   int stat;

   /* Check for special case: if lower is 0 and upper is OSUINT64MAX,  */
   /* set range to OSUINT64MAX; otherwise to upper - lower + 1         */

   range_value = (lower == 0 && upper == OSUINT64MAX) ?
      OSUINT64MAX : upper - lower + 1;

   adjusted_value = value - lower;

   PU_NEWFIELD (pctxt, "ConsUnsigned");

   if (lower != upper) {
      stat = pe_ConsWholeNumber64 (pctxt, adjusted_value, range_value);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   PU_SETBITCOUNT (pctxt);

   return 0;
}

EXTPERMETHOD int pe_ConsUInt64SignedBound (OSCTXT* pctxt, OSUINT64 value,
                                    OSINT64 lower, OSINT64 upper)
{
   /* if lower <= 0, value is okay w.r.t. lower.
      if upper < 0, value cannot be valid.  So, the only ways for the bound
      to be violated is:
      1) 0 <= value < lower OR
      2) upper < 0 OR
      3) 0 <= upper < value
   */
   if ( ( lower > 0 && value < (OSUINT64) lower ) ||
         ( upper < 0 ) ||
         ( upper >= 0 && value > (OSUINT64) upper ) )
      return LOG_RTERR(pctxt, RTERR_CONSVIO);
   else {
      /* The range will be OSUINT64MAX + 1 if lower == OSINT64MIN and
         upper = OSINT64MAX.  In that case, it is safe to pass OSINT64MIN
         as the range to pe_ConsWholeNumber64 (see note in code there).
      */
      OSUINT64 range_value = (lower == OSINT64MIN && upper == OSINT64MAX ) ?
         OSUINT64MAX : (OSUINT64)(upper - lower + 1);

      /* value - lower will not exceed OSUINT64MAX since value is in the
         range of lower..upper */
      OSUINT64 adjusted_value = value - lower;

      PU_NEWFIELD (pctxt, "ConsUnsigned");

      if (lower != upper) {
         int stat = pe_ConsWholeNumber64 (pctxt, adjusted_value, range_value);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }

      PU_SETBITCOUNT (pctxt);

      return 0;
   }
}

#endif

