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

#include "rtoersrc/asn1oer.h"
#include "rtxsrc/rtxBitString.h"
#include "rtxsrc/rtxBuffer.h"

/* Encode an OER bit string. This assumes a variable length string.
   Fixed-sized strings (i.e SIZE(N)) are encoded with no length or
   unused bit descriptors.  This is handled by the compiler. */

EXTOERMETHOD int oerEncBitStrPad
(OSCTXT* pctxt, const OSOCTET* pvalue, size_t numbits, size_t encBits)
{
   int stat;

   if ( encBits == 0 ) {
      /* empty bit string; encode just the zero length */
      stat = oerEncLen (pctxt, 0);
   }
   else {
      /* numbytes: # of bytes to encode. */
      size_t numbytes = encBits/8;

      /* unusedBits: # of bits left unused in the final encoding. 0-7. */
      OSOCTET unusedBits = (OSOCTET)(8 - (encBits % 8));

      /* finalBits: # of bits encoded from the final byte of pvalue. 1-8. */
      OSOCTET finalBits;

      /* dataBytes: # of bytes encoded from data in pvalue. */
      size_t dataBytes;

      if ( unusedBits == 8 ) {
         unusedBits = 0;
      }
      else numbytes++;

      if (numbits == encBits)
      {
         finalBits = 8 - unusedBits;
         dataBytes = numbytes;
      }
      else {
         finalBits = numbytes % 8;
         dataBytes = numbytes / 8;
         if ( finalBits == 0 ) finalBits = 8;
         else dataBytes += 1;
      }

      stat = oerEncLen (pctxt, numbytes + 1);
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      /* Encode the number of unused bits. */
      stat = rtxWriteBytes (pctxt, &unusedBits, 1);
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      if (finalBits == 8)
      {
         /* All bytes encoded from pvalue are encoded in their entirety.
            Encode those bytes.
         */
         stat = rtxWriteBytes(pctxt, pvalue, dataBytes);
      }
      else {
         /* The final byte from pvalue is partially used.  Make sure unused bits
            are zeroed. */
         OSOCTET lastByte = pvalue[dataBytes - 1];
         stat = (dataBytes > 1) ?
            rtxWriteBytes(pctxt, pvalue, dataBytes - 1) : 0;

         if (0 == stat) {
            stat = rtxZeroUnusedBits(&lastByte, finalBits);
            if (0 == stat) {
               stat = rtxWriteBytes(pctxt, &lastByte, 1);
            }
         }
      }

      /* If trailing bytes for trailing zero bits need to be encoded, do so. */
      while (stat == 0 && numbytes > dataBytes)
      {
         /* Encode zero bytes for trailing zero bits. */
         OSOCTET zero = 0;
         stat = rtxWriteBytes(pctxt, &zero, 1);
         numbytes--;
      }
   }

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}

EXTOERMETHOD int oerEncBitStr
(OSCTXT* pctxt, const OSOCTET* pvalue, size_t numbits)
{
   return oerEncBitStrPad(pctxt, pvalue, numbits, numbits);
}


EXTOERMETHOD int oerEncBitStrExt(OSCTXT* pctxt,
                                 const OSOCTET* pvalue, size_t numbits,
                                 const OSOCTET* extdata, size_t dataSize)
{
   size_t numbytes = numbits/8;
   int stat;
   OSOCTET unusedBits = (OSOCTET)(8 - (numbits % 8));

   if ( numbits == 0 )
   {
      /* empty bit string; encode just the zero length */
      stat = oerEncLen (pctxt, 0);
   }
   else
   {
      if ( unusedBits == 8 )
      {
         unusedBits = 0;
      }
      else numbytes++;

      stat = oerEncLen (pctxt, numbytes + 1);
      if (stat < 0)
      {
         return LOG_RTERR (pctxt, stat);
      }

      stat = rtxWriteBytes (pctxt, &unusedBits, 1);
      if (stat < 0)
      {
         return LOG_RTERR (pctxt, stat);
      }

      stat = rtxWriteBytes (pctxt, pvalue, dataSize);
      if (stat < 0)
      {
         return LOG_RTERR (pctxt, stat);
      }

      if (0 == unusedBits) {
         stat = rtxWriteBytes (pctxt, extdata, numbytes - dataSize);
      }
      else {
         /* Set unused bits to zero in last byte */
         OSOCTET lastByte;
         numbytes -= dataSize;
         lastByte = extdata[numbytes-1];

         stat = (numbytes > 1) ?
            rtxWriteBytes (pctxt, extdata, numbytes-1) : 0;

         if (0 == stat) {
            stat = rtxZeroUnusedBits (&lastByte, 8 - unusedBits);
            if (0 == stat) {
               stat = rtxWriteBytes (pctxt, &lastByte, 1);
            }
         }
      }

      if (stat < 0)
      {
         return LOG_RTERR (pctxt, stat);
      }
   }

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}

