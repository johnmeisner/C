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

#include <limits.h>
#include "rtbersrc/asn1berStream.h"

int berDecStrmBitStr (OSCTXT *pctxt, OSOCTET* pvalue, OSUINT32* pnbits,
                      ASN1TagType tagging, int length)
{
#ifndef _NO_STREAM
   register int stat;
   OSUINT32 bufsiz = (*pnbits - 1) / 8 + 1;  /* Size, in bytes, of static
                                                buffer. */

   if (tagging == ASN1EXPL) {
      stat = berDecStrmMatchTag (pctxt, ASN_ID_BITSTR, &length, TRUE);
      /* RTERR_IDNOTFOU will be logged later, by the generated code,
         or reset by rtxErrReset (for optional seq elements). */
      if (stat != 0)
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   if (pctxt->flags & ASN1CONSTAG) { /* constructed case */
      OSUINT32 totalLen = 0;
      ASN1CCB ccb;
      OSOCTET b = 0;  /* Unused bits octet. */

      ccb.len = length;
      ccb.bytes = OSRTSTREAM_BYTEINDEX (pctxt);
      while (!BS_CHKEND (pctxt, &ccb)) {
         int ll;
         OSSIZE octidx; /* Index in static buffer to write to. */

         if (b != 0) {
            /* The previous segment must have set b, but only the last
            segment should set b to non-zero. */
            return LOG_RTERR(pctxt, RTERR_BADVALUE);
         }

         /* Read tag for fragment. */
         if ((stat = berDecStrmMatchTag (pctxt, ASN_ID_BITSTR, &ll, TRUE)) !=
              0)
            return LOG_RTERR (pctxt, stat);
         if (ll <= 0)
            return LOG_RTERR (pctxt, ASN_E_INVLEN);

         /* Read unused bits octet. */
         stat = rtxReadBytes(pctxt, &b, 1);
         if (stat < 0) return LOG_RTERR(pctxt, stat);

         ll--;

         octidx = totalLen;
         totalLen += ll;
         if (totalLen > bufsiz)
            return LOG_RTERR (pctxt, RTERR_STROVFLW);

         stat = rtxReadBytes (pctxt, pvalue + octidx, ll);
         if (stat < 0) return LOG_RTERR (pctxt, stat);
      }
      if (ccb.stat != 0)
         return LOG_RTERR (pctxt, ccb.stat);

      if (b > 7) /* initial octet should be 0..7 */
         return LOG_RTERR(pctxt, RTERR_BADVALUE);

      *pnbits = totalLen * 8 - b;

      if (length == ASN_K_INDEFLEN) {
         stat = berDecStrmMatchEOC (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
   }
   else { /* primitive case */
      if (length < 0)
         return LOG_RTERR (pctxt, ASN_E_INVLEN);

      if (length > 0) {
         int c;
         OSOCTET b;
         OSUINT32 numToRead = ((OSUINT32)length) - 1;

         if (numToRead > bufsiz)
            return LOG_RTERR (pctxt, RTERR_STROVFLW);

         stat = rtxReadBytes (pctxt, &b, 1);
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         c = b;

         if (c > 7) /* initial octet should be 0..7 */
            return LOG_RTERR (pctxt, RTERR_BADVALUE);

         if (numToRead == 0) {
            if (c == 0)
               *pnbits = 0;
            else
               return LOG_RTERR (pctxt, ASN_E_INVLEN);
         }
         else {
            *pnbits = (numToRead * 8) - c;

            stat = rtxReadBytes (pctxt, pvalue, numToRead);
            if (stat < 0) return LOG_RTERR (pctxt, stat);
         }
      }
      else {
         *pnbits = 0;
      }
   }
#endif

   return 0;
}
