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
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxContext.hh"
/* START NOOSS */
#ifdef RTEVAL
#define _CRTLIB
#include "rtxevalsrc/rtxEval.hh"
#else
#define OSRT_CHECK_EVAL_DATE_STAT0(pctxt,stat)
#define OSRT_CHECK_EVAL_DATE_STAT1(pctxt,stat)
#define OSRT_CHECK_EVAL_DATE0(pctxt)
#define OSRT_CHECK_EVAL_DATE1(pctxt)
#endif /* RTEVAL */
/* END NOOSS */

/* define rotate bits macro. MS VC before 2005 will use _rotl intrinsic. */
#if defined(_MSC_VER) && _MSC_VER < 1400 && defined(_M_IX86)
#define ROTL(value, shift) _rotl (value, shift)
#else
#define ROTL(value, shift) (value >> (32 - shift)) | (value << shift)
#endif

/* use pu_bitcnt to fast version when compiler has _BitScanReverse. */
#if defined(_MSC_VER) && _MSC_VER >= 1310 && defined(_M_IX86)

#if _MSC_VER >= 1400 && !defined(_WIN32_WCE)/* vs 2005 and higher has this include; 2003 doesn't */
#include <intrin.h>  /*   have the _BitScanReverse function */
#endif

#define pu_bitcnt pu_bitcnt_
unsigned char _BitScanReverse(unsigned long* Index, unsigned long Mask);
#pragma intrinsic(_BitScanReverse)

static OSUINT32 pu_bitcnt_ (OSUINT32 range)
{
    unsigned long res;
    if (_BitScanReverse (&res, range))
       return res + 1;

    return 0;
}

#endif

/***********************************************************************
 *
 *  COMMON ENCODE FUNCTIONS
 *
 **********************************************************************/

/***********************************************************************
 *
 *  Routine name: pe_GetMsgBitCnt
 *
 *  Description:  This routine returns the number of bits in the
 *                encoded PER message.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct* Pointer to ASN.1 PER context structure
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  length      int     Length (in bits) of encoded message
 *
 *
 **********************************************************************/

EXTPERMETHOD size_t pe_GetMsgBitCnt (OSCTXT* pctxt)
{
   int numBitsInLastByte = 8 - pctxt->buffer.bitOffset;
   return ((pctxt->buffer.byteIndex * 8) + numBitsInLastByte);
}


/***********************************************************************
 *
 *  Routine name: pe_GetMsgPtr
 *
 *  Description:  This routine returns a pointer to the start of the
 *                encoded message and the length of the message.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct* Pointer to ASN.1 PER context structure
 *  nbytes      uint    Number of bytes of additional space required
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     Status of operation
 *
 **********************************************************************/

EXTPERMETHOD OSOCTET* pe_GetMsgPtr (OSCTXT* pctxt, OSINT32* pLength)
{
   if (pLength) *pLength = (OSINT32)pe_GetMsgLen (pctxt);
   return pctxt->buffer.data;
}

EXTPERMETHOD OSOCTET* pe_GetMsgPtrU (OSCTXT* pctxt, OSUINT32* pLength)
{
   if (pLength) *pLength = (OSUINT32)pe_GetMsgLen (pctxt);
   return pctxt->buffer.data;
}

EXTPERMETHOD OSOCTET* pe_GetMsgPtr64 (OSCTXT* pctxt, OSSIZE* pLength)
{
   if (pLength) *pLength = pe_GetMsgLen (pctxt);
   return pctxt->buffer.data;
}

EXTPERMETHOD int pe_bitsAligned (OSCTXT* pctxt, OSUINT32 value, OSUINT32 nbits)
{
   size_t idx = pctxt->buffer.byteIndex;
   unsigned bitOff = pctxt->buffer.bitOffset;
   /* How many bytes past the byte index will we write; if writing only at
    * the current byte index, nmBytes = 0. Does -not- capture the total number
    * of bytes to be written. */
   unsigned nmBytes;
   OSOCTET* p;

#ifdef _DEBUG
   if (nbits > 32)
      return LOG_RTERR (pctxt, RTERR_INVPARAM);
#endif

   if (nbits == 0)
      return 0;

   if (bitOff != 8)
      idx++;

   pctxt->buffer.bitOffset = 8;
   pctxt->buffer.byteIndex = idx;
#ifdef _TRACE
   PU_SETBITOFFSET (pctxt);
#endif

   nmBytes = (nbits - 1) >> 3;

   if (idx + nmBytes >= pctxt->buffer.size) {
      /* expand buffer */
      int stat = pe_ExpandBuffer (pctxt, nmBytes + 1);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      /* byteIndex could be reset by the call to pe_ExpandBuffer if we are
       * writing to a stream. */
      idx = pctxt->buffer.byteIndex;
   }

   p = pctxt->buffer.data + idx;

   pctxt->buffer.byteIndex = idx + (nbits >> 3);
   pctxt->buffer.bitOffset = (OSUINT16) (8 - (nbits & 7));

   value <<= 32 - nbits; /* clear unused bits */
   value = ROTL (value, 8);

   while (nmBytes-- > 0) {
      *p++ = (OSOCTET) value;
      value = ROTL (value, 8);
   }

   *p = (OSOCTET) value;

   return 0;
}

/***********************************************************************
 *
 *  Routine name: encBitsFromOctet
 *
 *  Description:  This routine encodes bits from a given octet to
 *                the output buffer. It either encodes the most significant
 *                or least significant bits based on the msbits flag.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct*  Pointer to ASN.1 PER context structure
 *  value       octet   Value of bits to be encoded
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

static int encBitsFromOctet
(OSCTXT* pctxt, OSOCTET value, OSUINT32 nbits, OSBOOL msbits)
{
   OSSIZE idx = pctxt->buffer.byteIndex;
   OSUINT32 bitOff = pctxt->buffer.bitOffset;

#ifdef _DEBUG
   if (nbits > 8)
      return LOG_RTERR (pctxt, RTERR_INVPARAM);
#endif

   if (nbits == 0)
      return 0;

   if (bitOff == 8) {
      /* field aligned */

      if (idx >= pctxt->buffer.size) {
         /* expand buffer */
         int stat = pe_ExpandBuffer (pctxt, 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         idx = pctxt->buffer.byteIndex;
      }

      pctxt->buffer.byteIndex = idx + (nbits >> 3);
      pctxt->buffer.bitOffset = (OSUINT16) (8 - (nbits & 7));

      if (msbits)
         value &= (OSOCTET) (0xFF00 >> nbits); /* clear unused bits */
      else
         value <<= 8 - nbits; /* clear unused bits */

      pctxt->buffer.data[idx] = value;
   }
   else {
      /* field unaligned */
      /* usedBits: bits already used in current byte */
      unsigned usedBits = 8 - bitOff;
      /* nextOff: bits that will be used, starting with current byte, after
         writing the nbits. */
      unsigned nextOff = nbits + usedBits;
      /* nmBytes: number of bytes we'll write to, excluding current byte. */
      unsigned nmBytes = (nextOff - 1) >> 3;
      OSOCTET* p;

      if (idx + nmBytes >= pctxt->buffer.size) {
         /* expand buffer */
         int stat = pe_ExpandBuffer (pctxt, nmBytes + 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         idx = pctxt->buffer.byteIndex;
      }

      p = pctxt->buffer.data + idx;
      pctxt->buffer.bitOffset = (OSUINT16) (8 - (nextOff & 0x7));

      if (msbits)
         value &= (OSOCTET) (0xFF00 >> nbits); /* clear unused bits */
      else
         value <<= 8 - nbits; /* clear unused bits */

      *p++ |= value >> usedBits;

      if (nextOff > 7) {
         /* The current byte was filled, advance the index. */
         pctxt->buffer.byteIndex = idx + 1;

         if (nextOff > 8) {
            /* We have bits to write into the next byte (where p now points). */
            *p = (OSOCTET) (value << bitOff);
         }
      }
   }

   return 0;
}

/* formerly pe_bitsFromOctet - encode most-significant bits from octet */
int pe_msBitsFromOctet(OSCTXT* pctxt, OSUINT8 value, OSUINT32 nbits)
{
   return encBitsFromOctet(pctxt, value, nbits, TRUE);
}

/* formerly pe_bits8 - encode least-significant bits from octet */
int pe_lsBitsFromOctet(OSCTXT* pctxt, OSUINT8 value, OSUINT32 nbits)
{
   return encBitsFromOctet(pctxt, value, nbits, FALSE);
}

/* formerly pe_bits8Aligned */
int pe_lsBitsFromOctetAligned (OSCTXT* pctxt, OSUINT8 value, OSUINT32 nbits)
{
   if (nbits == 0)
      return 0;
#ifdef _DEBUG
   else if (nbits > 8)
      return LOG_RTERR (pctxt, RTERR_INVPARAM);
#endif

   if (pctxt->buffer.bitOffset != 8) {
      pctxt->buffer.bitOffset = 8;
      pctxt->buffer.byteIndex++;
   }
#ifdef _TRACE
   PU_SETBITOFFSET (pctxt);
#endif
   return encBitsFromOctet(pctxt, value, nbits, FALSE);
}

int pe_octetAligned (OSCTXT* pctxt, OSUINT8 value)
{
   if (pctxt->buffer.bitOffset != 8) {
      pctxt->buffer.bitOffset = 8;
      pctxt->buffer.byteIndex++;
   }
#ifdef _TRACE
   PU_SETBITOFFSET (pctxt);
#endif
   return pe_octet(pctxt, value);
}

int pe_octet (OSCTXT* pctxt, OSUINT8 value)
{
   if (pctxt->buffer.bitOffset == 8) {
      if (pctxt->buffer.byteIndex >= pctxt->buffer.size) {
         /* expand buffer */
         int stat = pe_ExpandBuffer (pctxt, 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      pctxt->buffer.data[pctxt->buffer.byteIndex++] = value;
   }
   else {
      if (pctxt->buffer.byteIndex+1 >= pctxt->buffer.size) {
         /* expand buffer */
         int stat = pe_ExpandBuffer (pctxt, 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      pctxt->buffer.data[pctxt->buffer.byteIndex++] |=
         value >> (8 - pctxt->buffer.bitOffset);

      pctxt->buffer.data[pctxt->buffer.byteIndex] =
         (OSOCTET)(value << pctxt->buffer.bitOffset);
   }

   return 0;
}

/***********************************************************************
 *
 *  Routine name: pe_byte_align
 *
 *  Description:  Align the encode buffer on the next byte boundary.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct* Pointer to ASN.1 PER context structure
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     Status of operation
 *
 *
 **********************************************************************/

EXTPERMETHOD int pe_byte_align (OSCTXT* pctxt)
{
   /* increment byteIndex when bitOffset != 8 */
   if (pctxt->buffer.aligned && pctxt->buffer.bitOffset != 8) {
      pctxt->buffer.bitOffset = 8;
      pctxt->buffer.byteIndex++;
      PU_SETBITOFFSET (pctxt);
   }

   return 0;
}

/***********************************************************************
 *
 *  Routine name: pe_ConsWholeNumber
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

EXTPERMETHOD int pe_ConsWholeNumber (OSCTXT* pctxt,
                        OSUINT32 adjusted_value, OSUINT32 range_value)
{
   OSUINT32 nocts, range_bitcnt = pu_bitcnt (range_value - 1);
   int stat;

   if (adjusted_value >= range_value && range_value != OSUINT32_MAX) {
      return LOG_RTERR (pctxt, RTERR_BADVALUE);
   }

   /* If unaligned, encode as a non-negative binary integer in the      */
   /* minimum number of bits necessary to represent the range (10.5.6)  */

   if (!pctxt->buffer.aligned) {
      return pe_bits (pctxt, adjusted_value, range_bitcnt);
   }

   /* If aligned, encoding depended on range value (10.5.7) */

   else {  /* aligned */

      /* If range is <= 255, bit-field case (10.5.7a) */

      if (range_value <= 255) {
         return encBitsFromOctet
            (pctxt, (OSUINT8)adjusted_value, range_bitcnt, FALSE);
      }

      /* If range is exactly 256, one-octet case (10.5.7b) */

      else if (range_value == 256) {
         return pe_octetAligned(pctxt, (OSUINT8)adjusted_value);
      }

      /* If range > 256 and <= 64k (65536), two-octet case (10.5.7c) */

      else if (range_value <= OSINTCONST(65536)) {
         return pe_bitsAligned (pctxt, adjusted_value, 16);
      }

      /* If range > 64k, indefinite-length case (10.5.7d) */

      else {
         /* Encode length determinant as a constrained whole number.    */
         /* Constraint is 1 to max number of bytes needed to hold       */
         /* the target integer value..                                  */

         if (adjusted_value < 256) nocts = 1;
         else if (adjusted_value < OSINTCONST(65536)) nocts = 2;
         else if (adjusted_value < OSINTCONST(0x1000000)) nocts = 3;
         else nocts = 4;

         stat = pe_bits (pctxt, nocts - 1, 2);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         PU_INSLENFLD (pctxt);

         pe_byte_align (pctxt);
         return pe_NonNegBinInt (pctxt, adjusted_value);
      }
   }
}

/***********************************************************************
 *
 *  Routine name: pe_Length
 *
 *  Description:  This routine encodes a length determinant value
 *                as described in section 10.9 or the X.691 standard.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct* Pointer to ASN.1 PER context structure
 *  value       uint    Length value to be encoded
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  status      int     Completion status of encode operation
 *
 **********************************************************************/

EXTPERMETHOD int pe_Length (OSCTXT* pctxt, OSSIZE value)
{
   OSBOOL extendable, extendedSize;
   Asn1SizeValueRange sizeValueRange;
   int enclen, stat = pu_checkSizeExt
      (&(ACINFO(pctxt)->sizeConstraint), value, &extendable,
       &sizeValueRange, &extendedSize);

   /* If size constraints exist and the given length did not fall       */
   /* within the range of any of them, signal constraint violation      */
   /* error..                                                           */

   if (0 != stat) {
      #ifndef _COMPACT
      if (RTERR_CONSVIO == stat) {
         rtxErrAddElemNameParm (pctxt);
         rtxErrAddSizeParm (pctxt, value);
      }
      return LOG_RTERR (pctxt, stat);
      #else
      return stat;
      #endif
   }

   /* Reset the size constraint in the context block structure */

   PU_INITSIZECONSTRAINT (pctxt);

   /* If size constraint is present and extendable, encode extension    */
   /* bit..                                                             */

   if (extendable) {
      PU_NEWFIELD (pctxt, "length extension");

      stat = pe_bit (pctxt, extendedSize);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      PU_SETBITCOUNT (pctxt);
   }

   PU_NEWFIELD (pctxt, "length");

   /* If upper limit is less than 64k, constrained case */

   if (sizeValueRange.upper < OSINTCONST(65536) && !extendedSize) {
      if (sizeValueRange.lower == sizeValueRange.upper) {
         stat = 0;
      }
      else {
#ifndef _NO_INT64_SUPPORT
         stat = pe_ConsWholeNumber64
            (pctxt, value - sizeValueRange.lower,
             sizeValueRange.upper - sizeValueRange.lower + 1);
#else
         stat = pe_ConsWholeNumber
            (pctxt, value - sizeValueRange.lower,
             sizeValueRange.upper - sizeValueRange.lower + 1);
#endif
      }
      enclen = (stat == 0) ? (int)value : stat;
   }
   else {
      /* unconstrained case or Constrained with upper bound >= 64K*/
      enclen = pe_UnconsLength (pctxt, value);
   }

   PU_SETBITCOUNT (pctxt);

   OSRT_CHECK_EVAL_DATE0 (pctxt);

   return (enclen < 0) ? LOG_RTERR (pctxt, enclen) : enclen;
}

/***********************************************************************
 *
 *  Routine name: pe_SmallLength
 *
 *  Description:  This routine encompasses the rules to encode a
 *                normally small length as specified in
 *                section 11.9 of the X.691 standard.
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
 *  status      int     Completion status of encode operation
 *
  **********************************************************************/

EXTPERMETHOD int pe_SmallLength (OSCTXT* pctxt, OSSIZE value)
{
   int stat;

   if (value <= 64) {
      stat = pe_bit (pctxt, 0);  /*  indicate <= 64 */
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      stat = pe_lsBitsFromOctet(pctxt, (OSUINT8)(value-1), 6); /* encode n-1 */
   }
   else {
      int enclen;
      stat = pe_bit (pctxt, TRUE);  /* indicate value > 64 */
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      enclen = pe_Length(pctxt, value);  /* encode n */
      if ( enclen < 0 ) return LOG_RTERR(pctxt, enclen);
   }

   LCHECKPER (pctxt);

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}


/* Unconstrained length */

EXTPERMETHOD int pe_UnconsLength (OSCTXT* pctxt, OSSIZE value)
{
   int enclen, stat;

   if (pctxt->buffer.aligned) {
      /* 1 octet case */
      if (value < 128) {
         stat = pe_octetAligned (pctxt, (OSUINT8)value);
         enclen = (stat == 0) ? (int)value : stat;
      }
      /* 2 octet case */
      else if (value < 16384) {
         stat = pe_bitsAligned (pctxt, (OSUINT32)(value | 0x8000u), 16);
         enclen = (stat == 0) ? (int)value : stat;
      }
      /* fragmentation case */
      else {
         int multiplier = (value > 64*1024) ? 4 : (int)value/16384;
         stat = pe_octetAligned(pctxt, (OSOCTET) (multiplier | 0xC0u));
         enclen = (stat == 0) ? 16384 * multiplier : stat;
      }
   }
   else {
      /* 1 octet case */
      if (value < 128) {
         stat = pe_octet (pctxt, (OSUINT8)value);
         enclen = (stat == 0) ? (int)value : stat;
      }
      /* 2 octet case */
      else if (value < 16384) {
         stat = pe_bits (pctxt, (OSUINT32)(value | 0x8000u), 16);
         enclen = (stat == 0) ? (int)value : stat;
      }
      /* fragmentation case */
      else {
         int multiplier = (value > 64*1024) ? 4 : (int)value/16384;
         stat = pe_octet(pctxt, (OSUINT8) (multiplier | 0xC0u));
         enclen = (stat == 0) ? 16384 * multiplier : stat;
      }
   }
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   LCHECKPER (pctxt);

   return enclen;
}


/* Reset encode buffer pointer */

EXTPERMETHOD int pe_resetBuffer (OSCTXT* pctxt)
{
   pctxt->buffer.byteIndex = 0;
   pctxt->buffer.bitOffset = 8;
   if (0 != pctxt->pBitFldList) {
      rtxSListFree (&pctxt->pBitFldList->fieldList);
      pctxt->pBitFldList->pLastPrinted = 0;
   }
   return 0;
}

