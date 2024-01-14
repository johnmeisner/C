#include "rtpersrc/pe_common.hh"

/***********************************************************************
 *
 *  Routine name: pe_bits64
 *
 *  Description:  This routine encodes multiple bits into the 
 *                output buffer.  Up to the number of bits included
 *                in an unsigned integer can be encoded in a single
 *                call.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct* Pointer to ASN.1 PER context structure
 *  value       uint64  Value of bits to be encoded
 *  nbits       uint    Number of bits to be encoded
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

EXTPERMETHOD int pe_bits64 (OSCTXT* pctxt, OSUINT64 value, OSUINT32 nbits)
{ 
   int stat = 0;

   if (nbits > sizeof (OSUINT32) * 8) {
      stat = pe_bits (pctxt, (OSUINT32) (value >> (sizeof (OSUINT32) * 8)), 
         nbits - sizeof (OSUINT32) * 8);
      nbits = sizeof (OSUINT32) * 8;
   }
   if (stat == 0)
      stat = pe_bits (pctxt, (OSUINT32) (value & OSUINT32_MAX), nbits);
   return 0;
}

#endif

