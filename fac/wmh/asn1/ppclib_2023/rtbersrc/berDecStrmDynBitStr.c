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

int berDecStrmDynBitStr64 (OSCTXT *pctxt, const OSOCTET** ppvalue,
                           OSSIZE* pnbits, ASN1TagType tagging,
                           OSSIZE length, OSBOOL indefLen)
{
#ifndef _NO_STREAM
   register int stat = 0;
   OSOCTET* pvalue = 0;

   if (tagging == ASN1EXPL) {
      stat = berDecStrmMatchTag2
         (pctxt, ASN_ID_BITSTR, &length, &indefLen, TRUE);
      if (stat != 0)
      /* RTERR_IDNOTFOU will be logged later, by the generated code,
         or reset by rtxErrReset (for optional seq elements). */
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   if (pctxt->flags & ASN1CONSTAG) { /* constructed case */
      OSSIZE ll, totalLen = 0;
      OSSIZE byteIndex = OSRTSTREAM_BYTEINDEX (pctxt);
      OSOCTET* newbuf;
      OSBOOL indef;
      OSOCTET b = 0;    /* Unused bits octet. */

      for (;;) {
         OSSIZE octidx;    /* Index in dynamic buffer to write to. */

         /* Check for end of fragments. */
         if (indefLen) {
            if (berDecStrmTestEOC2 (pctxt)) break;
         }
         else if (OSRTSTREAM_BYTEINDEX(pctxt) >= (byteIndex + length)) {
            break;
         }

         if (b != 0) {
            /* The previous segment must have set b, but only the last
            segment should set b to non-zero. */
            return LOG_RTERR(pctxt, RTERR_BADVALUE);
         }

         /* Decode tag for next fragment. */
         stat = berDecStrmMatchTag2 (pctxt, ASN_ID_BITSTR, &ll, &indef, TRUE);
         if (stat != 0) break;

         /* Confirm tag was primitive. We're not supporting nested
         fragmentation (constructed tags). Note this also guarantees that
         the length is definite. */
         if (pctxt->flags & ASN1CONSTAG) {
            rtxErrAddStrParm(pctxt,
               "Fragmentation using nested constructed tags.");
            return LOG_RTERRNEW(pctxt, RTERR_NOTSUPP);
         }


         /* Read the unused bits octet. */
         stat = rtxReadBytes(pctxt, &b, 1);
         if (stat < 0) return LOG_RTERR(pctxt, stat);

         ll--;

         octidx = totalLen;
         totalLen += ll;
         newbuf = (OSOCTET*)rtxMemRealloc (pctxt, pvalue, totalLen);
         if (newbuf == NULL) {
            stat = RTERR_NOMEM;
            break;
         }
         pvalue = newbuf;

         stat = rtxReadBytes (pctxt, pvalue + octidx, ll);
         if (stat < 0) break; else stat = 0;
      }

      if (stat != 0) {
         rtxMemFreePtr (pctxt, pvalue);
         return LOG_RTERR (pctxt, stat);
      }

      if (b > 7) { /* initial octet should be 0..7 */
         rtxMemFreePtr(pctxt, pvalue);
         return LOG_RTERR(pctxt, RTERR_BADVALUE);
      }

      if (totalLen < OSSIZE_MAX / 8) {
         *pnbits = (totalLen * 8 - b);
      }
      else return LOG_RTERR(pctxt, RTERR_TOOBIG);

      if (indefLen) {
         stat = berDecStrmMatchEOC (pctxt);
         if (stat != 0) {
            rtxMemFreePtr (pctxt, pvalue);
            return LOG_RTERR (pctxt, stat);
         }
      }
   }
   else { /* primitive case */
      if (indefLen)
         return LOG_RTERR (pctxt, ASN_E_INVLEN);

      if (length > 0) {
         OSOCTET b;
         OSSIZE numToRead = length - 1;

         stat = rtxReadBytes (pctxt, &b, 1);
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         if (b > 7) /* initial octet should be 0..7 */
            return LOG_RTERR (pctxt, RTERR_BADVALUE);

         if (numToRead == 0) {
            if (b == 0)
               *pnbits = 0;
            else
               return LOG_RTERR (pctxt, ASN_E_INVLEN);
         }
         else {
            *pnbits = (numToRead * 8) - b;

            pvalue = (OSOCTET*)rtxMemAlloc (pctxt, numToRead);
            if (pvalue == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);

            stat = rtxReadBytes (pctxt, pvalue, numToRead);
            if (stat < 0) {
               rtxMemFreePtr (pctxt, pvalue);
               return LOG_RTERR (pctxt, stat);
            }
         }
      }
      else {
         *pnbits = 0;
      }
   }
   *ppvalue = pvalue;
#endif

   return 0;
}

int berDecStrmDynBitStr (OSCTXT *pctxt, const OSOCTET** ppvalue,
                         OSUINT32* pnbits, ASN1TagType tagging, int length)
{
#ifndef _NO_STREAM
   OSSIZE numbits;
   OSBOOL indefLen = (OSBOOL)(ASN_K_INDEFLEN == length);
   int ret;

   if (0 == ppvalue || 0 == pnbits) return LOG_RTERR (pctxt, RTERR_INVPARAM);
   if (length < 0 && !indefLen) return LOG_RTERR (pctxt, RTERR_INVLEN);

   ret = berDecStrmDynBitStr64
      (pctxt, ppvalue, &numbits, tagging, (OSSIZE)length, indefLen);

   if (0 != ret) return LOG_RTERR (pctxt, ret);

   if (sizeof(OSSIZE) > sizeof(OSUINT32) && numbits > OSUINT32_MAX) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }
   else *pnbits = (OSUINT32)numbits;
#endif

   return 0;
}

