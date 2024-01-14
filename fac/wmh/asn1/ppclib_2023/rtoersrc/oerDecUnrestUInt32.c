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

int oerDecUnrestUInt32Value (OSCTXT* pctxt, OSUINT32* pvalue, size_t len)
{
   /* len shall be > 0 */

   int      stat, retval = 0;
   OSOCTET  tmp;
   OSUINT32 value;

   /* read the first byte and assign it to value
   */
   stat = rtxReadBytes(pctxt, &tmp, 1);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   value = tmp;
   len--;

   if ( value == 0 && len > 0 )
   {
      /* leading zero byte was not needed */
      retval = ASN_E_NOTCANON;
   }

   /* loop over remaining bytes (if any) and see if we can fit them in
   */
   while ( len > 0 ) {
      stat = rtxReadBytes(pctxt, &tmp, 1);
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      if ( value <= 0xFFFFFF )
      {
         /* the high byte is 0x00, meaning we can shift in another byte */
         value = (value * 256) + tmp;
      }
      else {
         /* high byte cannot be shifted off; value doesn't fit */
         return LOG_RTERR (pctxt, RTERR_TOOBIG);
      }

      len--;
   }

   if (0 != pvalue) {
      *pvalue = value;
   }

   return retval;
}

EXTOERMETHOD int oerDecUnrestUInt32 (OSCTXT* pctxt, OSUINT32* pvalue)
{
   int      stat, retval = 0;
   OSSIZE  len;

   stat = oerDecLen(pctxt, &len);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   /* Decode value */

   stat = oerDecUnrestUInt32Value (pctxt, pvalue, len);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   return retval;
}

EXTOERMETHOD int oerDecUnrestUInt8 (OSCTXT* pctxt, OSUINT8* pvalue)
{
   OSUINT32 tmpval;
   int ret = oerDecUnrestUInt32 (pctxt, &tmpval);
   if (0 == ret) {
      if (tmpval <= 255) {
         if (0 != pvalue) *pvalue = (OSUINT8)tmpval;
      }
      else ret = RTERR_TOOBIG;
   }
   if (0 != ret) {
      return (ASN_E_NOTCANON == ret) ? ret : LOG_RTERR (pctxt, ret);
   }
   return 0;
}

EXTOERMETHOD int oerDecUnrestUInt16 (OSCTXT* pctxt, OSUINT16* pvalue)
{
   OSUINT32 tmpval;
   int ret = oerDecUnrestUInt32 (pctxt, &tmpval);
   if (0 == ret) {
      if (tmpval <= 65535UL) {
         if (0 != pvalue) *pvalue = (OSUINT16)tmpval;
      }
      else ret = RTERR_TOOBIG;
   }
   if (0 != ret) {
      return (ASN_E_NOTCANON == ret) ? ret : LOG_RTERR (pctxt, ret);
   }
   return 0;
}
