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

/* Decode an OER bit string into a dynamic buffer. This assumes a
   variable length string.  Fixed-sized strings (i.e SIZE(N)) are encoded
   with no length or unused bit descriptors.  This is handled by the
   compiler. */

EXTOERMETHOD int oerDecDynBitStr64
(OSCTXT *pctxt, OSOCTET** ppvalue, OSSIZE* pnbits)
{
   OSSIZE len, numToRead;
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
      if (0 != ppvalue) *ppvalue = 0;
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
      if (0 != ppvalue) {
         if ((pctxt->flags & ASN1FASTCOPY) != 0 && !OSRTISSTREAM (pctxt)) {
            *ppvalue = OSRTBUFPTR (pctxt) + 1;
            pctxt->buffer.byteIndex += numToRead;
         }
         else {
            OSOCTET* pdata = (OSOCTET*) rtxMemAlloc (pctxt, numToRead);
            *ppvalue = pdata;
            if (*ppvalue == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);

            stat = rtxReadBytes (pctxt, pdata, numToRead);
         }
      }
      else {
         stat = rtxReadBytes (pctxt, 0, numToRead);
      }
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      if (0 != pnbits) *pnbits = ((numToRead * 8) - unusedBits);
   }
   else {
      if (unusedBits != 0) return LOG_RTERR (pctxt, ASN_E_INVLEN);
      else if (0 != pnbits) *pnbits = 0;
   }

   return retval;
}

EXTOERMETHOD int oerDecDynBitStr
(OSCTXT *pctxt, const OSOCTET** ppvalue, OSUINT32* pnbits)
{
   OSSIZE nbits;
   int ret;

   ret = oerDecDynBitStr64(pctxt, (OSOCTET**)ppvalue, &nbits);
   if (0 != ret && ret != ASN_E_NOTCANON)
      return LOG_RTERR (pctxt, ret);

#if (SIZE_MAX > LLONG_MAX)
   if (nbits > OSUINT32_MAX) return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);
#endif
   if (pnbits) *pnbits = (OSUINT32) nbits;

   return ret;
}
