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

#include "rtpersrc/asn1per.h"
#include "rtxsrc/rtxDiag.h"

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

#ifndef _NO_INT64_SUPPORT

EXTPERMETHOD int pd_ConsInt64 (OSCTXT* pctxt,
                  OSINT64* pvalue, OSINT64 lower, OSINT64 upper)
{
   OSUINT64 range_value = upper - lower;
   OSUINT64 adjusted_value;
   int stat;

   PU_NEWFIELD (pctxt, "ConsInteger");

   /* Adjust range value */
   if (range_value != OSUINT64MAX) { range_value += 1; }

   if (lower > upper)
      return LOG_RTERR (pctxt, RTERR_BADVALUE);
   else if (lower != upper) {
      stat = pd_ConsWholeNumber64 (pctxt, &adjusted_value, range_value);
      if (stat == 0) {
         if (pvalue) {
            *pvalue = adjusted_value + lower;

            /* Verify value is within given range */
            if (*pvalue < lower || *pvalue > upper) {
   #ifndef _COMPACT
               rtxErrAddElemNameParm (pctxt);
               rtxErrAddInt64Parm (pctxt, *pvalue);
               return LOG_RTERR (pctxt, RTERR_CONSVIO);
   #else
               return RTERR_CONSVIO;
   #endif
            }
         }
      }
#ifndef _COMPACT
      else return LOG_RTERR (pctxt, stat);
#else
      else return stat;
#endif
   }
   else {
      *pvalue = lower;
   }

   PU_SETBITCOUNT (pctxt);
   RTDIAGSTRM3 (pctxt, "pd_ConsInt64: decoded value is %d\n", *pvalue);

   return 0;
}

#endif

