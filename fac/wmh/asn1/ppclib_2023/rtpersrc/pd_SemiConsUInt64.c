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

EXTPERMETHOD int pd_SemiConsUInt64 (OSCTXT* pctxt, OSUINT64* pvalue, OSUINT64 lower)
{
   OSUINT32 nbytes, temp = 0;
   int stat;

   RTDIAGSTRM2 (pctxt,"pd_SemiConsUInt64: start\n");

   stat = pd_Length (pctxt, &nbytes);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   RTDIAGSTRM3 (pctxt,"pd_SemiConsUInt64: nbytes is %d\n", nbytes);

   PU_NEWFIELD (pctxt, (lower == 0) ? "UnconsUnsigned" : "SemiConsUnsigned");

   if (nbytes > 0) {
      if (pctxt->buffer.aligned) {
         if ((stat = PD_BYTE_ALIGN (pctxt)) != 0)
            return LOG_RTERR (pctxt, stat);
      }

      if (nbytes > sizeof (OSUINT32)) {
         stat = pd_bits (pctxt, &temp, (nbytes - sizeof (OSUINT32)) * 8);
         nbytes = sizeof (OSUINT32);
      }

      if (stat == 0) {
         *pvalue = temp;
         *pvalue <<= sizeof (OSUINT32) * 8;
         stat = pd_bits (pctxt, &temp, nbytes * 8);
         *pvalue |= temp;
      }
      else return LOG_RTERR (pctxt, stat);
   }
   else
      *pvalue = 0;
   *pvalue += lower;

   PU_SETBITCOUNT (pctxt);
   RTDIAGSTRM3 (pctxt,"pd_SemiConsUInt64: decoded value is "OSUINT64FMT"\n", *pvalue);

   return 0;
}


EXTPERMETHOD int pd_SemiConsUInt64SignedBound (OSCTXT* pctxt, OSUINT64* pvalue,
                                                OSINT64 lower)
{
   OSUINT32 nbytes, temp = 0;
   int stat;

   RTDIAGSTRM2 (pctxt,"pd_SemiConsUInt64SignedBound: start\n");

   stat = pd_Length (pctxt, &nbytes);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   RTDIAGSTRM3 (pctxt,"pd_SemiConsUInt64SignedBound: nbytes is %d\n", nbytes);

   PU_NEWFIELD (pctxt, "SemiCons");

   if (nbytes > 0) {
      if (pctxt->buffer.aligned) {
         if ((stat = PD_BYTE_ALIGN (pctxt)) != 0)
            return LOG_RTERR (pctxt, stat);
      }

      if (nbytes > sizeof (OSUINT32)) {
         stat = pd_bits (pctxt, &temp, (nbytes - sizeof (OSUINT32)) * 8);
         nbytes = sizeof (OSUINT32);
      }

      if (stat == 0) {
         *pvalue = temp;
         *pvalue <<= sizeof (OSUINT32) * 8;
         stat = pd_bits (pctxt, &temp, nbytes * 8);
         *pvalue |= temp;
      }
      else return LOG_RTERR (pctxt, stat);
   }
   else
      *pvalue = 0;

   if ( (OSUINT64)(-lower) > *pvalue ) {
      /* actual value is negative */
      return LOG_RTERR(pctxt, RTERR_TOOBIG);
   }

   *pvalue += lower;

   if ( lower > 0 && *pvalue < (OSUINT64) lower ) {
      /* actual value > OSUINT64MAX.  We had overflow */
      return LOG_RTERR(pctxt, RTERR_TOOBIG);
   }

   PU_SETBITCOUNT (pctxt);
   RTDIAGSTRM3 (pctxt,"pd_SemiConsUInt64SignedBound: decoded value is "
                        OSUINT64FMT"\n", *pvalue);

   return 0;
}



EXTPERMETHOD int pd_UnconsUInt64 (OSCTXT* pctxt, OSUINT64* pvalue)
{
   OSUINT32 nbytes, highBits = 0;
   size_t MAX_BYTES = sizeof(OSUINT64);

   int stat;

   RTDIAGSTRM2 (pctxt,"pd_UnconsUInt64: start\n");

   stat = pd_Length (pctxt, &nbytes);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   RTDIAGSTRM3 (pctxt,"pd_UnconsUInt64: nbytes is %d\n", nbytes);

   PU_NEWFIELD (pctxt, "UnconsUInt64");

   if (nbytes > 0) {
      OSBOOL checkSign = TRUE;      /* true until the sign bit is checked */

      if (pctxt->buffer.aligned) {
         if ((stat = PD_BYTE_ALIGN (pctxt)) != 0)
            return LOG_RTERR (pctxt, stat);
      }

      if ( nbytes > MAX_BYTES + 1 ) {
         /* cannot possibly fit into OSUINT64 */
         return LOG_RTERR(pctxt, RTERR_TOOBIG);
      }
      else if (nbytes == MAX_BYTES + 1 ) {
         /* Will fit into OSUINT64 if and only if the leading byte is 0. */
         /* Otherwise, the value is out of range (either negative or too big).*/
         /* If leading byte is 0, go on to read the remaining bytes */

         OSUINT32 leadingByte;

         stat = pd_bits( pctxt, &leadingByte, 8 );
         if ( stat != 0 ) return LOG_RTERR(pctxt, stat);

         if ( leadingByte != 0 ) return LOG_RTERR(pctxt, RTERR_TOOBIG);
         else {
            checkSign = FALSE;    /* known to be positive by leading 0 byte */
            nbytes--;
         }
      }
      /* else: nbytes <= MAX_BYTES: proceed */

      if (nbytes > sizeof (OSUINT32)) {
         /* read the leading bytes */
         OSUINT32 bitsToRead = (nbytes - sizeof(OSUINT32)) * 8;
         OSUINT32 mostSigBit_mask = 1 << (bitsToRead - 1);

         stat = pd_bits (pctxt, &highBits, bitsToRead);

         if ( stat == 0 && checkSign) {
            /* check the sign bit.  It must not be set (we're decoding
               into unsigned type). */
            if ( ( highBits & mostSigBit_mask ) != 0 )
               return LOG_RTERR(pctxt, RTERR_TOOBIG);
            else checkSign = FALSE;    /* known to be positive */
         }

         nbytes = sizeof (OSUINT32);
      }

      if (stat == 0) {
         /* Note: previous code treated (OSUINT32*)pvalue + 1 as a pointer to
            the low 4 bytes of *pvalue.  This assumes endianess. */
         OSUINT32 lowBits;

         *pvalue = highBits;
         *pvalue <<= sizeof (OSUINT32) * 8;

         stat = pd_bits (pctxt, &lowBits, nbytes * 8);
         *pvalue |= lowBits;

         if ( stat != 0 ) return LOG_RTERR(pctxt, stat);

         if ( checkSign ) {
            OSUINT32 mostSigBit_mask =
               (OSUINT32)(1 << ((nbytes * 8) - 1));
            if ( ( lowBits & mostSigBit_mask ) != 0 )
               /* value is negative */
               return LOG_RTERR(pctxt, RTERR_TOOBIG);
         }
      }
      else return LOG_RTERR (pctxt, stat);
   }
   else {
      /* length of 0 is technically not correct, but a common mistake */
      *pvalue = 0;
   }

   PU_SETBITCOUNT (pctxt);
   RTDIAGSTRM3 (pctxt,"pd_UnconsUInt64: decoded value is "OSUINT64FMT"\n", *pvalue);

   return 0;
}



