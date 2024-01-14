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

#include "rtpersrc/asn1per.h"
#include "rtxsrc/rtxContext.hh"

/***********************************************************************
 *
 *  Routine name: pd_<integer>
 *
 *  Description:  The following functions decode integers.  Different
 *                routines handle signed/unsigned and constrained/
 *                unconstrained variations.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pvalue      int*    Pointer to value to receive decoded result
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     Completion status of operation
 *
 **********************************************************************/

EXTPERMETHOD int pd_ConsUInt64
(OSCTXT* pctxt, OSUINT64* pvalue, OSUINT64 lower, OSUINT64 upper)
{
	/* This code originated in pd_ConsUnsigned.c */
   OSUINT64 range_value;
   OSUINT64 adjusted_value;
   int stat;

   /* Check for special case: if lower is 0 and upper is OSUINT64MAX,  */
   /* set range to OSUINT64MAX; otherwise to upper - lower + 1         */

   range_value = (lower == 0 && upper == OSUINT64MAX) ?
      OSUINT64MAX : upper - lower + 1;

   PU_NEWFIELD (pctxt, "ConsUnsigned");

   if (lower != upper) {
      stat = pd_ConsWholeNumber64 (pctxt, &adjusted_value, range_value);
      if (stat == 0) {
         *pvalue = adjusted_value + lower;

         /* Verify value is within given range */
         if (*pvalue < lower || *pvalue > upper) {
            rtxErrAddElemNameParm (pctxt);
            rtxErrAddUInt64Parm (pctxt, *pvalue);
            return LOG_RTERR (pctxt, RTERR_CONSVIO);
         }
      }
      else return LOG_RTERR (pctxt, stat);
   }
   else *pvalue = lower;

   PU_SETBITCOUNT (pctxt);

   LCHECKPER (pctxt);

   return 0;
}

EXTPERMETHOD int pd_ConsUInt64SignedBound
(OSCTXT* pctxt, OSUINT64* pvalue, OSINT64 lower, OSINT64 upper)
{
   OSUINT64 adjusted_value;
   int stat;

   /* The range will be OSUINT64MAX + 1 if lower == OSINT64MIN and
      upper = OSINT64MAX.  In that case, it is safe to pass OSINT64MIN
      as the range to pd_ConsWholeNumber64 (see note in code there).
   */
   OSUINT64 range_value = (lower == OSINT64MIN && upper == OSINT64MAX ) ?
      OSUINT64MAX : (OSUINT64)(upper - lower + 1);

   PU_NEWFIELD (pctxt, "ConsUnsigned");

   if ( lower == upper ) {
      if ( lower < 0 ) {
         /* The actual value is negative and cannot fit in pvalue */
         return LOG_RTERR(pctxt, RTERR_TOOBIG);
      }
      else *pvalue = lower;
   }
   else {
      stat = pd_ConsWholeNumber64 (pctxt, &adjusted_value, range_value);
      if (stat == 0) {
         if ( lower < 0 && (OSUINT64)(-lower) > adjusted_value ) {
            /* The actual value is negative and cannot fit in pvalue */
            return LOG_RTERR(pctxt, RTERR_TOOBIG);
         }
         /* else actual value will be >= 0 and representable, assuming
            that adjusted_value is bounded by the number of bits required
            to represent ub - lb, even if adjusted_value > range_value.
         */

         *pvalue = adjusted_value + lower;

         /* The actual value is in range when 0 <= adj <= ub - lb. */
         if ( adjusted_value > (OSUINT64) (upper - lower) ) {
            rtxErrAddElemNameParm (pctxt);
            rtxErrAddUInt64Parm (pctxt, *pvalue);
            return LOG_RTERR (pctxt, RTERR_CONSVIO);
         }
      }
      else return LOG_RTERR (pctxt, stat);
   }

   PU_SETBITCOUNT (pctxt);

   LCHECKPER (pctxt);

   return 0;
}

