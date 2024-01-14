/*
 * Copyright (c) 2014-2018 Objective Systems, Inc.
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

EXTOERMETHOD int oerEncBitStr
(OSCTXT* pctxt, const OSOCTET* pvalue, size_t numbits)
{
   int stat;

   if ( numbits == 0 ) {
      /* empty bit string; encode just the zero length */
      stat = oerEncLen (pctxt, 0);
   }
   else {
      size_t numbytes = numbits/8;
      OSOCTET unusedBits = (OSOCTET)(8 - (numbits % 8));

      if ( unusedBits == 8 ) {
         unusedBits = 0;
      }
      else numbytes++;

      stat = oerEncLen (pctxt, numbytes + 1);
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      stat = rtxWriteBytes (pctxt, &unusedBits, 1);
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      if (0 == unusedBits) {
         stat = rtxWriteBytes (pctxt, pvalue, numbytes);
      }
      else {
         /* Set unused bits to zero in last byte */
         OSOCTET lastByte = pvalue[numbytes-1];
         stat = (numbytes > 1) ?
            rtxWriteBytes (pctxt, pvalue, numbytes-1) : 0;

         if (0 == stat) {
            stat = rtxZeroUnusedBits (&lastByte, 8 - unusedBits);
            if (0 == stat) {
               stat = rtxWriteBytes (pctxt, &lastByte, 1);
            }
         }
      }
   }

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
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

