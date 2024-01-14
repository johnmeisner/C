#include "rtpersrc/pe_common.hh"
#include "rtxsrc/rtxUtil.h"

/***********************************************************************
 *
 *  Routine name: pe_NonNegBinInt64
 *
 *  Description:  This routine encompasses the rules to encode a
 *                non-negative binary 64-bit integer as specified in section 
 *                10.3 of the X.691 standard.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct* Pointer to ASN.1 PER context structure
 *  value       uint    Value to be encoded
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     Status of operation
 *
 *
 **********************************************************************/

#ifndef _NO_INT64_SUPPORT
EXTPERMETHOD int pe_NonNegBinInt64 (OSCTXT* pctxt, OSUINT64 value)
{
   /* 10.3.6  A minimum octet non-negative binary integer encoding of   */
   /* the whole number (which does not predetermine the number of       */
   /* octets to be used for the encoding) has a field which is a        */
   /* multiple of 8 bits and also satisifies the condition that the     */
   /* leading eight bits of the field shall not be zero unless the      */
   /* field is precisely 8 bits long.                                   */
 
   OSUINT32 bitcnt = (value == 0) ? 1 : rtxUInt64BitLen (value);

   /* round-up to nearest 8-bit boundary */

   bitcnt = (bitcnt + 7) & (~7);

   /* encode bits */

   return pe_bits64 (pctxt, value, bitcnt);
}

#endif

