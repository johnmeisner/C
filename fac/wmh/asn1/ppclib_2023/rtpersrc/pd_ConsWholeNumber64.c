#include "rtpersrc/asn1per.h"
#include "rtxsrc/rtxUtil.h"

/***********************************************************************
 *
 *  Routine name: pd_ConsWholeNumber64
 *
 *  Description:  This routine encompasses the rules to decode a
 *                constrained whole number as specified in section 
 *                10.5 of the X.691 standard.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pdbuf      struct	Pointer to PER decode buffer structure
 *  value       uint*   Pointer to value to receive decoded result
 *  lower       uint    Lower constraint value
 *  upper       uint    Upper constraint value
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  status      int     Completion status of encode operation
 *
 **********************************************************************/

#ifndef _NO_INT64_SUPPORT

EXTPERMETHOD int pd_ConsWholeNumber64 
(OSCTXT* pctxt, OSUINT64* padjusted_value, OSUINT64 range_value)
{ 
   /*
   Note: if the actual range is OSUINT64MAX + 1, it is safe to pass 
   OSUINT64MAX, since the number of bits to represent both ranges is the same.
   */
   
   OSUINT32 nocts, range_bitcnt, temp = 0;
   int stat = 0;

   /* If unaligned, decode non-negative binary integer in the minimum   */
   /* number of bits necessary to represent the range (10.5.6)          */

   if (!pctxt->buffer.aligned) {
      range_bitcnt = rtxUInt64BitLen (range_value - 1);
   }

   /* If aligned, encoding depended on range value (10.5.7) */

   else {  /* aligned */

      /* If range is <= 255, bit-field case (10.5.7a) */

      if (range_value <= 255) {
         range_bitcnt = rtxUInt64BitLen (range_value - 1);
      }

      /* If range is exactly 256, one-octet case (10.5.7b) */

      else if (range_value == 256) {
         stat = PD_BYTE_ALIGN (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         range_bitcnt = 8;
      }

      /* If range > 256 and <= 64k (65535), two-octet case (10.5.7c) */

      else if (range_value <= OSINTCONST(65536)) {
         stat = PD_BYTE_ALIGN (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         range_bitcnt = 16;
      }

      /* If range > 64k, indefinite-length case (10.5.7d) */

      else {
         /* 2^16 < range <= 2^64 - 1
            We'll have length (in octets) bt. 3 and 8 octets or encoded
            (adjusted) length between 2 and 7 (2 and 3 bits to encode).
            a range of 2^32 + 1 is where 3 bits becomes needed for length.
         */
         OSUINT64 limit = (OSUINT64)OSUINT32_MAX + 1;
         OSUINT32 nbits = (range_value > limit) ? 3 : 2;

         stat = pd_bits (pctxt, &nocts, nbits);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         PU_INSLENFLD (pctxt);

         stat = PD_BYTE_ALIGN (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         range_bitcnt = (nocts + 1) * 8;
      }
   }

   if (range_bitcnt > sizeof (OSUINT32) * 8) {
      stat = pd_bits (pctxt, &temp, range_bitcnt - sizeof (OSUINT32) * 8);
      range_bitcnt = sizeof (OSUINT32) * 8;
   }

   if (stat == 0) {
      OSUINT32 temp2;
      stat = pd_bits (pctxt, &temp2, range_bitcnt);
      if (padjusted_value) {
         *padjusted_value = temp;
         *padjusted_value <<= sizeof (OSUINT32) * 8;
         *padjusted_value |= temp2;
      }
   }

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}

#endif

