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

#include "rtsrc/asn1intl.h"
#include "rtpersrc/pu_common.hh"

/***********************************************************************
 *
 *  DECODE FUNCTIONS
 *
 **********************************************************************/

/***********************************************************************
 *
 *  Routine name: pd_ConsWholeNumber
 *
 *  Description:  This routine encompasses the rules to decode a
 *                constrained whole number as specified in section
 *                10.5 of the X.691 standard.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pdbuf       struct  Pointer to PER decode buffer structure
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

EXTPERMETHOD int pd_ConsWholeNumber
(OSCTXT* pctxt, OSUINT32* padjusted_value, OSUINT32 range_value)
{
   OSUINT32 nocts, range_bitcnt;
   int stat;

   /* If unaligned, decode non-negative binary integer in the minimum   */
   /* number of bits necessary to represent the range (10.5.6)          */

   if (!pctxt->buffer.aligned) {
      range_bitcnt = pu_bitcnt (range_value - 1);
   }

   /* If aligned, encoding depended on range value (10.5.7) */

   else {  /* aligned */

      /* If range is <= 255, bit-field case (10.5.7a) */

      if (range_value <= 255) {
         range_bitcnt = pu_bitcnt (range_value - 1);
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
         stat = pd_bits (pctxt, &nocts, 2);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         PU_INSLENFLD (pctxt);

         stat = PD_BYTE_ALIGN (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         range_bitcnt = (nocts + 1) * 8;
      }
   }

   return pd_bits (pctxt, padjusted_value, range_bitcnt);
}


/***********************************************************************
 *
 *  Routine name: pd_SmallLength
 *
 *  Description:  This routine encompasses the rules to decode a
 *                normally small length as specified in
 *                section 11.9 of the X.691 standard.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pdbuf       struct  Pointer to PER decode buffer structure
 *  pvalue      uint*   Value to be decoded
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  status      int     Completion status of encode operation
 *
  **********************************************************************/

EXTPERMETHOD int pd_SmallLength (OSCTXT* pctxt, OSUINT32* pvalue)
{
   OSBOOL bitValue;
   int ret;

   if ((ret = PD_BIT (pctxt, &bitValue)) != 0)
      return ret;

   if (bitValue == 0) {
      /* n <= 64. Encoded value is n-1. */
      ret = pd_bits (pctxt, pvalue, 6);
      *pvalue += 1;
      return ret;
   }
   else {
      /*  n > 64. Encoded value is n. */
      return pd_Length (pctxt, pvalue);
   }
}


/***********************************************************************
 *
 *  Routine name: pd_SmallNonNegWholeNumber
 *
 *  Description:  This routine encompasses the rules to decode a
 *                small non-negative whole number as specified in
 *                section 10.6 of the X.691 standard.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pdbuf       struct  Pointer to PER decode buffer structure
 *  pvalue      uint*   Value to be decoded
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  status      int     Completion status of encode operation
 *
  **********************************************************************/

EXTPERMETHOD int pd_SmallNonNegWholeNumber (OSCTXT* pctxt, OSUINT32* pvalue)
{
   OSBOOL bitValue;
   OSUINT32 len;
   int ret;

   if ((ret = PD_BIT (pctxt, &bitValue)) != 0)
      return ret;

   if (bitValue == 0) {
      return pd_bits (pctxt, pvalue, 6);   /* 10.6.1 */
   }
   else {
      if ((ret = pd_Length (pctxt, &len)) < 0)
         return ret;

      if ((ret = PD_BYTE_ALIGN (pctxt)) != 0)
         return ret;

      return pd_bits (pctxt, pvalue, len*8);
   }
}

/***********************************************************************
 *
 *  Routine name: pd_Length
 *
 *  Description:  This routine encompasses the rules to decode a
 *                length determinant as specified in section
 *                10.9 of the X.691 standard.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct  Pointer to PER decode buffer structure
 *  value       uint*   Pointer to value to receive decoded result
 *  lower       uint    Lower constraint value
 *  upper       uint    Upper constraint value
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  status      int     Completion status of operation
 *
 **********************************************************************/

EXTPERMETHOD int pd_Length64 (OSCTXT* pctxt, OSSIZE* pvalue)
{
   Asn1SizeValueRange* pSize;
   OSSIZE lower, upper;
   OSBOOL extbit;
   int stat;
   OSUINT32 tmp = 0;

   /* If size constraint is present and extendable, decode extension    */
   /* bit..                                                             */

   if (ACINFO(pctxt)->sizeConstraint.ext.upper > 0) {
      PU_NEWFIELD (pctxt, "length extension");

      stat = PD_BIT (pctxt, &extbit);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      PU_SETBITCOUNT (pctxt);
   }
   else extbit = 0;

   /* Now use the value of the extension bit to select the proper       */
   /* size constraint range specification..                             */

   pSize = PU_GETSIZECONSTRAINT (pctxt, extbit);

   lower = pSize->lower;
   upper = (pSize->upper > 0) ? pSize->upper : OSSIZE_MAX;

   /* Reset the size constraint in the context block structure */

   PU_INITSIZECONSTRAINT (pctxt);

   /* If upper limit is less than 64k, constrained case */

   if (upper < OSINTCONST(65536)) {
      PU_NEWFIELD (pctxt, "length");

      if (lower == upper) {
         *pvalue = 0;
         stat = 0;
      }
      else
         stat = pd_ConsWholeNumber
            (pctxt, &tmp, (OSUINT32)(upper - lower + 1));

      if (stat == 0) {
         *pvalue = tmp + lower;
         if ( *pvalue > upper ) {
            rtxErrAddStrParm (pctxt, "'length'");
            rtxErrAddUInt64Parm (pctxt, *pvalue);
            return LOG_RTERR (pctxt, RTERR_CONSVIO);
         }
      }
      else return LOG_RTERR (pctxt, stat);

      PU_SETBITCOUNT (pctxt);
   }
   else {
      /* unconstrained case OR Constrained with upper bound >= 64K */
      stat = pd_UnconsLength64 (pctxt, pvalue);
   }

   return stat;
}

EXTPERMETHOD int pd_Length (OSCTXT* pctxt, OSUINT32* pvalue)
{
   OSSIZE len = 0;
   int ret = pd_Length64 (pctxt, &len);
   if (ret < 0) return LOG_RTERR (pctxt, ret);
   else if (sizeof(len) > 4 && len > OSUINT32_MAX) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }
   *pvalue = (OSUINT32)len;
   return ret;
}

EXTPERMETHOD int pd_GetComponentLength64
(OSCTXT* pctxt, OSUINT32 itemBits, OSSIZE* plength)
{
   OSCTXT lctxt;
   OSSIZE len, totalLen = 0;
   int stat;

   if (OSRTISSTREAM (pctxt))
      return LOG_RTERR (pctxt, RTERR_NOTSUPP);

   stat = rtInitSubContext (&lctxt, pctxt);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   stat = pu_initContextBuffer (&lctxt, pctxt);
   if (stat != 0) return LOG_RTERR_AND_FREE (pctxt, stat, &lctxt);

   ACINFO(&lctxt)->sizeConstraint = ACINFO(pctxt)->sizeConstraint;

   for (;;) {
      stat = pd_Length64 (&lctxt, &len);
      if (stat < 0) return LOG_RTERR_AND_FREE (pctxt, stat, &lctxt);

      if (totalLen > (OSSIZE_MAX - len)) {
         rtFreeContext (&lctxt);
         return LOG_RTERR_AND_FREE (pctxt, RTERR_TOOBIG, &lctxt);
      }
      totalLen += len;

      if (stat == RT_OK_FRAG) {
         if (len <= (OSUINT32_MAX / itemBits)) {
            stat = pd_moveBitCursor (&lctxt, (OSUINT32)(len * itemBits));
         }
         else {
            return LOG_RTERR_AND_FREE (pctxt, RTERR_TOOBIG, &lctxt);
         }
         if (stat < 0)
            return LOG_RTERR_AND_FREE (pctxt, stat, &lctxt);
      }
      else break;
   }

   rtFreeContext (&lctxt);

   if (0 != plength) *plength = totalLen;

   return 0;
}

EXTPERMETHOD int pd_GetComponentLength (OSCTXT* pctxt, OSUINT32 itemBits)
{
   OSSIZE len;
   int ret = pd_GetComponentLength64 (pctxt, itemBits, &len);
   if (0 != ret) return LOG_RTERR (pctxt, ret);
   else if (len > OSINT32_MAX) return LOG_RTERR (pctxt, RTERR_TOOBIG);
   else return (int)len;
}

EXTPERMETHOD int pd_GetBinStrDataOffset
(OSCTXT* pctxt, OSUINT32* pnumbits, OSBOOL bitStrFlag)
{
   OSUINT32 len;
   size_t byteIndex = pctxt->buffer.byteIndex;
   OSUINT16 bitOffset = pctxt->buffer.bitOffset;
   Asn1SizeCnst sizeCnst = ACINFO(pctxt)->sizeConstraint;
   size_t msgBitOffset = PU_GETCTXTBITOFFSET (pctxt);
   int stat;

   if (0 == pnumbits) return LOG_RTERR (pctxt, RTERR_NULLPTR);

   stat = pd_Length (pctxt, &len);
   if (stat < 0) return LOG_RTERR (pctxt, stat);
   else if (stat == RT_OK_FRAG) {
      rtxErrAddStrParm (pctxt, "determine length of fragmented length field");
      return LOG_RTERR (pctxt, RTERR_NOTSUPP);
   }

   if (pctxt->buffer.aligned) {
      if (pu_BitAndOctetStringAlignmentTest (&sizeCnst, len, bitStrFlag)) {
         stat = PD_BYTE_ALIGN (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
   }

   *pnumbits = (OSUINT32)(PU_GETCTXTBITOFFSET (pctxt) - msgBitOffset);

   pctxt->buffer.byteIndex = byteIndex;
   pctxt->buffer.bitOffset = bitOffset;
   ACINFO(pctxt)->sizeConstraint = sizeCnst;

   return 0;
}

EXTPERMETHOD int pd_UnconsLength64 (OSCTXT* pctxt, OSSIZE* pvalue)
{
   int stat = 0;
   OSUINT32 u32value;
   OSBOOL bitValue;

   if (pctxt->buffer.aligned) {
      stat = PD_BYTE_ALIGN (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   PU_NEWFIELD (pctxt, "length");

   stat = PD_BIT (pctxt, &bitValue);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   if (bitValue == 0) {
      stat = pd_bits (pctxt, &u32value, 7);   /* 10.9.3.6 */
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      else *pvalue = u32value;
   }
   else {
      stat = PD_BIT (pctxt, &bitValue);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      if (bitValue == 0) {
         stat = pd_bits (pctxt, &u32value, 14);  /* 10.9.3.7 */
         if (stat != 0) return LOG_RTERR (pctxt, stat);
         else *pvalue = u32value;
      }
      else {
         OSUINT32 multiplier;

         stat = pd_bits (pctxt, &multiplier, 6);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
         else if (multiplier > 4) return LOG_RTERR (pctxt, RTERR_INVLEN);

         *pvalue = 16384 * multiplier;

         stat = RT_OK_FRAG;
      }
   }

   PU_SETBITCOUNT (pctxt);

   return stat;
}

EXTPERMETHOD int pd_UnconsLength (OSCTXT* pctxt, OSUINT32* pvalue)
{
   OSSIZE len;
   int ret = pd_UnconsLength64 (pctxt, &len);
   if (ret != 0) return LOG_RTERR (pctxt, ret);

   if (sizeof(len) > 4 && len > OSUINT32_MAX)
      return LOG_RTERR (pctxt, RTERR_TOOBIG);

   *pvalue = (OSUINT32) len;
   return 0;
}
