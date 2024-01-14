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

EXTPERMETHOD int pd_SemiConsUnsigned (OSCTXT* pctxt, OSUINT32* pvalue,
                                       OSUINT64 lower)
{
   OSUINT64 value;
   int stat;

   stat = pd_SemiConsUInt64(pctxt, &value, lower);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   if ( value > OSUINT32_MAX ) return LOG_RTERR(pctxt, RTERR_TOOBIG);
   else {
      *pvalue = (OSUINT32) value;
      return 0;
   }
}



EXTPERMETHOD int pd_UnconsUnsigned (OSCTXT* pctxt, OSUINT32* pvalue)
{
   size_t MAX_BYTES = sizeof(OSUINT32);
   OSUINT32 nbytes;
   int stat;

   RTDIAGSTRM2 (pctxt,"pd_UnconsUnsigned: start\n");

   stat = pd_Length (pctxt, &nbytes);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   RTDIAGSTRM3 (pctxt,"pd_UnconsUnsigned: nbytes is %d\n", nbytes);

   PU_NEWFIELD (pctxt, "UnconsUnsigned");

   if (nbytes > 0) {
      if (pctxt->buffer.aligned) {
         if ((stat = PD_BYTE_ALIGN (pctxt)) != 0)
            return LOG_RTERR (pctxt, stat);
      }
      if ( nbytes > MAX_BYTES + 1 ) {
         /* cannot fit into pvalue */
         return LOG_RTERR(pctxt, RTERR_TOOBIG);
      }
      else if ( nbytes == MAX_BYTES + 1 ) {
         /* okay only if leading byte is 0; otherwise we have a positive or */
         /* negative value that cannot fit in pvalue */
         stat = pd_bits(pctxt, pvalue, 8);
         if ( stat != 0 ) return LOG_RTERR(pctxt, stat);
         if ( *pvalue != 0 ) return LOG_RTERR(pctxt, RTERR_TOOBIG);
         else {
            /* value is okay; read the remaining bytes into pvalue */
            stat = pd_bits (pctxt, pvalue, (nbytes - 1) * 8);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }
      }
      else {
         /* nbytes <= MAX_BYTES; read the bytes and then make sure the leading */
         /* bit that was read was not 1 (would indicate a negative value) */
         OSUINT32 mask = 1 << ( nbytes * 8 - 1 );
         stat = pd_bits (pctxt, pvalue, nbytes * 8);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
         if ( (*pvalue & mask) != 0 ) {
            /* negative value */
            return LOG_RTERR(pctxt, RTERR_TOOBIG);
         }
      }
   }
   else {
      /* technically should not occur, but its a common mistake */
      *pvalue = 0;
   }

   PU_SETBITCOUNT (pctxt);
   RTDIAGSTRM3 (pctxt,"pd_UnconsUnsigned: decoded value is %u\n", *pvalue);

   return 0;
}
