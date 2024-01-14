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

#include "rtbersrc/asn1berStream.h"
#include "rtxsrc/rtxContext.hh"

int berDecStrmDynOctStr64 (OSCTXT* pctxt, OSOCTET** ppvalue,
                           OSSIZE* pnocts, ASN1TagType tagging,
                           OSSIZE length, OSBOOL indefLen)
{
   register int stat = 0;
   OSOCTET* pvalue = 0;

   if (0 == ppvalue || 0 == pnocts) return LOG_RTERR (pctxt, RTERR_INVPARAM);

   if (tagging == ASN1EXPL) {
      stat = berDecStrmMatchTag2
         (pctxt, ASN_ID_OCTSTR, &length, &indefLen, TRUE);
      if (stat != 0)
      /* RTERR_IDNOTFOU will be logged later, by the generated code,
         or reset by rtxErrReset (for optional seq elements). */
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   if (pctxt->flags & ASN1CONSTAG) { /* constructed case */
      OSSIZE ll, totalLen = 0, octidx = 0;
      OSSIZE byteIndex = OSRTSTREAM_BYTEINDEX (pctxt);
      OSOCTET* newbuf;
      OSBOOL indef;

      for (;;) {
         if (indefLen) {
            if (berDecStrmTestEOC2 (pctxt)) break;
         }
         else if (OSRTSTREAM_BYTEINDEX(pctxt) >= (byteIndex + length)) {
            break;
         }

         stat = berDecStrmMatchTag2 (pctxt, ASN_ID_OCTSTR, &ll, &indef, TRUE);
         if (stat != 0) break;

         if (indef) {
            stat = ASN_E_INVLEN;
            break;
         }
         else if (0 == ll) continue;

         totalLen += ll;
         newbuf = (OSOCTET*)rtxMemRealloc (pctxt, pvalue, totalLen);
         if (newbuf == NULL) {
            stat = RTERR_NOMEM;
            break;
         }
         pvalue = newbuf;

         stat = rtxReadBytes (pctxt, pvalue + octidx, ll);
         if (stat < 0) break; else stat = 0;

         octidx = totalLen;
      }

      if (stat != 0) {
         rtxMemFreePtr (pctxt, pvalue);
         return LOG_RTERR (pctxt, stat);
      }
      *pnocts = totalLen;

      if (indefLen)
         berDecStrmMatchEOC (pctxt);
   }
   else { /* primitive case */
      if (indefLen)
         return LOG_RTERR (pctxt, ASN_E_INVLEN);

      if (length > 0) {
         OSSIZE numToRead = length;

         *pnocts = numToRead;

         pvalue = (OSOCTET*)rtxMemAlloc (pctxt, numToRead);
         if (pvalue == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);

         stat = rtxReadBytes (pctxt, pvalue, numToRead);
         if (stat != 0) {
            rtxMemFreePtr (pctxt, pvalue);
            return LOG_RTERR (pctxt, stat);
         }
      }
      else {
         *pnocts = 0;
      }
   }
   *ppvalue = pvalue;

   LCHECKBER (pctxt);

   return 0;
}

int berDecStrmDynOctStr
(OSCTXT *pctxt, const OSOCTET** ppvalue, OSUINT32* pnocts,
 ASN1TagType tagging, int length)
{
   OSSIZE numocts;
   OSBOOL indefLen = (OSBOOL)(ASN_K_INDEFLEN == length);
   OSOCTET* pvalue = 0;
   int ret;

   if (0 == ppvalue || 0 == pnocts) return LOG_RTERR (pctxt, RTERR_INVPARAM);
   if (length < 0 && !indefLen) return LOG_RTERR (pctxt, RTERR_INVLEN);

   ret = berDecStrmDynOctStr64
      (pctxt, &pvalue, &numocts, tagging, (OSSIZE)length, indefLen);

   if (0 != ret) return LOG_RTERR (pctxt, ret);

   if (sizeof(OSSIZE) > sizeof(OSUINT32) && numocts > OSUINT32_MAX) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }
   else *pnocts = (OSUINT32)numocts;

   *ppvalue = pvalue;

   return 0;
}
