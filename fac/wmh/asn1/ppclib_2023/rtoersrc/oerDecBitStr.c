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
#include "rtxsrc/rtxBuffer.h"

/* Decode an OER bit string. This assumes a variable length string.
   Fixed-sized strings (i.e SIZE(N)) are encoded with no length or
   unused bit descriptors.  This is handled by the compiler. */

EXTOERMETHOD int oerDecBitStr
(OSCTXT *pctxt, OSOCTET* pvalue, size_t bufsiz, OSUINT32* pnbits)
{
   size_t len, numToRead;
   OSOCTET unusedBits;
   int retval = 0;

   /* Decode length */
   int stat = oerDecLen (pctxt, &len);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   if (len == 0) {
      if (0 != pnbits) *pnbits = 0;
      return retval;
   }
   numToRead = len - 1;

   /* Decode number of unused bits in last byte */
   stat = rtxReadBytes (pctxt, &unusedBits, 1);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   if (unusedBits > 7) /* initial octet should be 0..7 */
      return LOG_RTERR (pctxt, RTERR_BADVALUE);

   /* Decode contents */
   if (numToRead > 0) {
      OSUINT32 nbits = (OSUINT32)((numToRead * 8) - unusedBits);

      stat = rtxReadBytesSafe (pctxt, pvalue, bufsiz, numToRead);
      if (0 == stat) {
         stat = rtxCheckUnusedBitsZero (pvalue, nbits);
      }
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      if (0 != pnbits) *pnbits = nbits;
   }
   else if (0 != pnbits) {
      *pnbits = 0;
   }

   return retval;
}

EXTOERMETHOD int oerDecBitStrExt(OSCTXT* pctxt, OSOCTET* pvalue,
                                 size_t bufsiz, OSUINT32* pnbits,
                                 OSOCTET** extdata)
{
   size_t extLen, len, numToRead;
   OSOCTET *pExtData = NULL;
   int stat;
   OSOCTET unusedBits;

   /* Decode length */
   stat = oerDecLen (pctxt, &len);
   if (0 != stat)
   {
      return LOG_RTERR (pctxt, stat);
   }

   if (len == 0)
   {
      if (NULL != pnbits)
      {
         *pnbits = 0;
      }
      return 0;
   }
   numToRead = len - 1;

   /* Decode number of unused bits in last byte */
   stat = rtxReadBytes (pctxt, &unusedBits, 1);
   if (stat < 0)
   {
      return LOG_RTERR (pctxt, stat);
   }

   if (unusedBits > 7) /* initial octet should be 0..7 */
   {
      return LOG_RTERR (pctxt, RTERR_BADVALUE);
   }

   /* Decode contents */
   if (numToRead > 0)
   {
      stat = rtxReadBytesSafe (pctxt, pvalue, bufsiz, (numToRead < bufsiz) ? numToRead : bufsiz);
      if (stat < 0)
      {
         return LOG_RTERR (pctxt, stat);
      }

      if (NULL != pnbits)
      {
         *pnbits = (OSUINT32)((numToRead * 8) - unusedBits);
      }
   }
   else if (NULL != pnbits)
   {
      *pnbits = 0;
   }

   /* Allocate and decode extdata contents, if any... */
   if (numToRead > bufsiz)
   {
      extLen = numToRead - bufsiz;
      pExtData = (OSOCTET *)rtxMemAlloc(pctxt, extLen);
      stat = rtxReadBytesSafe (pctxt, pExtData, extLen, extLen);
      if (stat < 0)
      {
         return LOG_RTERR (pctxt, stat);
      }
      *extdata = pExtData;
   }

   return 0;
}

