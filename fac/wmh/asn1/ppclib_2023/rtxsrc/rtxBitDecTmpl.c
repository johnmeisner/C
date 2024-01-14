/*
 * Copyright (c) 2014-2023 Objective Systems, Inc.
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

/* Decode bits to an integer value. User must define the following items:

RTXDECBITSFN - Decode function name
RTXDECBITSINTTYPE - Integer type of value

and then include this template file.
*/
EXTRTMETHOD int RTXDECBITSFN
(OSCTXT* pctxt, RTXDECBITSINTTYPE* pvalue, OSSIZE nbits)
{
   static const OSOCTET bitmask[] =
      { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };
   int stat;

   if (nbits == 0) {
      if (pvalue) {
         *pvalue = 0;
      }
      return 0;
   }
   else if (nbits > sizeof(RTXDECBITSINTTYPE)*8) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }

   /* If the number of bits is less than the current bit offset, mask   */
   /* off the required number of bits and return..                      */

   if (/*pctxt->buffer.bitOffset >= 0 &&*/
       nbits <= (OSSIZE)pctxt->buffer.bitOffset)
   {
      /* Check if buffer contains number of bits requested */

      if (pctxt->buffer.byteIndex >= pctxt->buffer.size) {
         stat = rtxCheckInputBuffer (pctxt, 1);
         if (stat < 0)
            return LOG_RTERR (pctxt, stat);
      }

      pctxt->buffer.bitOffset =
         (OSINT16) (pctxt->buffer.bitOffset - nbits);

      if (0 != pvalue) {
         *pvalue = ((pctxt->buffer.data[pctxt->buffer.byteIndex]) >>
                    pctxt->buffer.bitOffset) & ((OSINTCONST(1) << nbits) - 1);
      }

      if (pctxt->buffer.bitOffset == 0) {
         pctxt->buffer.bitOffset = 8;
         pctxt->buffer.byteIndex++;
      }

      return 0;
   }

   /* Otherwise, we first need to mask off the remaining bits in the    */
   /* current byte, followed by a loop to extract bits from full bytes, */
   /* followed by logic to mask of remaining bits from the start of     */
   /* of the last byte..                                                */

   else {
      OSUINT32 val;
      const OSOCTET* p;

      /* Check if buffer contains number of bits requested */

      OSSIZE nbytes = (nbits - pctxt->buffer.bitOffset + 15) / 8;

      if (pctxt->buffer.byteIndex + nbytes > pctxt->buffer.size) {
         stat = rtxCheckInputBuffer (pctxt, nbytes);
         if (stat < 0)
            return LOG_RTERR (pctxt, stat);
      }

      p = pctxt->buffer.data + pctxt->buffer.byteIndex;

      /* first read current byte remaining bits */
      val = (OSOCTET) (*p++ & bitmask[pctxt->buffer.bitOffset]);

      nbits -= pctxt->buffer.bitOffset;
      pctxt->buffer.bitOffset = 8;
      pctxt->buffer.byteIndex++;

      /* second read bytes from next byteIndex */
      while (nbits >= 8) {
         val <<= 8;
         val |= *p++;
         pctxt->buffer.byteIndex++;
         nbits -= 8;
      }

      /* third read bits & set bitoffset of the byteIndex */
      if (nbits > 0) {
         pctxt->buffer.bitOffset = (OSUINT16)(8 - nbits);
         val <<= nbits;
         val |= *p >> pctxt->buffer.bitOffset;
      }

      if (0 != pvalue) { *pvalue = val; }
   }

   return 0;
}

#undef RTXDECBITSFN
#undef RTXDECBITSINTTYPE
