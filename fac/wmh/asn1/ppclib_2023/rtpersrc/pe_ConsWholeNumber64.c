#include "rtpersrc/pe_common.hh"
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxUtil.h"

/***********************************************************************
 *
 *  Routine name: pe_ConsWholeNumber64
 *
 *  Description:  This routine encompasses the rules to encode a
 *                constrained whole number as specified in section 
 *                10.5 of the X.691 standard.
 *
 *  Inputs:
 *
 *  Name             Type    Description
 *  ----             ----    -----------
 *  pctxt           struct* Pointer to ASN.1 PER context structure
 *  adjusted_value   uint    Value to be encoded
 *  range_value      uint    Lower - Upper + 1
 *
 *  Outputs:
 *
 *  Name             Type    Description
 *  ----             ----    -----------
 *  status           int     Completion status of encode operation
 *
  **********************************************************************/

#ifndef _NO_INT64_SUPPORT

EXTPERMETHOD int pe_ConsWholeNumber64 (OSCTXT* pctxt, 
                          OSUINT64 adjusted_value, OSUINT64 range_value)
{ 
   /*
   Note: it is safe to pass 2^64 - 1 (OSUINT64MAX) for range_value when
   the actual range is 2^64, because the encoding will be the same - 64 bits
   are required to encode both range values.
   */
   int stat;

   if (adjusted_value >= range_value && range_value != OSUINT64MAX) {
      return LOG_RTERR (pctxt, RTERR_BADVALUE);
   }

   if ( adjusted_value <= OSUINT32_MAX && range_value <= OSUINT32_MAX) {
      stat = pe_ConsWholeNumber (pctxt, (OSUINT32)adjusted_value, 
         (OSUINT32)range_value);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else {
      OSUINT32 range_bitcnt = rtxUInt64BitLen (range_value - 1);

      /* If unaligned, encode as a non-negative binary integer in the      */
      /* minimum number of bits necessary to represent the range (10.5.6)  */

      if (!pctxt->buffer.aligned) {
         stat = pe_bits64 (pctxt, adjusted_value, range_bitcnt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }

      /* If aligned, encoding depended on range value (10.5.7) */

      else {  /* aligned */
         if ( range_value > OSINTCONST(65536) ) {
            OSUINT32 len_bits = rtxUInt64BitLen (((range_bitcnt + 7) /8) - 1);

            stat = pe_ConsWholeNumRangeGT64K (pctxt, adjusted_value, len_bits);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }
         else if ( range_value > 256 ) {
            /* range > 256 and <= 64k (65536), two-octet case (10.5.7c) */
            return pe_bitsAligned (pctxt, (OSUINT32) adjusted_value, 16);
         }
         else if ( range_value == 256 ) {
            /* range is exactly 256, one-octet case (10.5.7b) */
            return pe_octetAligned (pctxt, (OSUINT8)adjusted_value);
         }
         else {
            /* range is <= 255, bit-field case (10.5.7a) */
            return pe_lsBitsFromOctet
               (pctxt, (OSUINT8) adjusted_value, range_bitcnt);
         }
      }
   }
   return 0;
}

EXTPERMETHOD int pe_ConsWholeNumRangeGT64K
(OSCTXT* pctxt, OSUINT64 adjusted_value, OSUINT32 len_bits)
{
   /* range > 64k, indefinite-length case (10.5.7d)               */
   /* Encode length determinant as a constrained whole number.    */
   /* Constraint is 1 to max number of bytes needed to hold       */
   /* the target integer value..                                  */
   OSUINT32 nocts = adjusted_value == 0 ? 1 : 
      (rtxUInt64BitLen (adjusted_value) + 7) /8;

   int stat = rtxEncBits (pctxt, nocts - 1, len_bits);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   PU_INSLENFLD (pctxt);

   stat = pe_byte_align (pctxt);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   stat = pe_NonNegBinInt64 (pctxt, adjusted_value);
   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}

#endif 

