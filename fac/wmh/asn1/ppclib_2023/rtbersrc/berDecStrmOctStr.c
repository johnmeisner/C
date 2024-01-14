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

#include "rtbersrc/asn1berStream.h"
#include "rtxsrc/rtxContext.hh"

int berDecStrmOctStr64 (OSCTXT *pctxt, OSOCTET* pvalue, OSSIZE* pnocts,
                        ASN1TagType tagging, OSSIZE length, OSBOOL indefLen)
{
#ifndef _NO_STREAM
   int stat = 0;
   OSSIZE bufsiz = *pnocts;

   if (tagging == ASN1EXPL) {
      stat = berDecStrmMatchTag2
         (pctxt, ASN_ID_OCTSTR, &length, &indefLen, TRUE);

      if (stat != 0)
         /* RTERR_IDNOTFOU will be logged later, by the generated code,
            or reset by rtxErrReset (for optional seq elements). */
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   if (pctxt->flags & ASN1CONSTAG) { /* constructed case */
      OSSIZE totalLen = 0, octidx = 0, ll;
      OSBOOL indef2;

      for (;;) {
         if (indefLen) {
            if (berDecStrmTestEOC2(pctxt)) break;
         }
         stat = berDecStrmMatchTag2 (pctxt, ASN_ID_OCTSTR, &ll, &indef2, TRUE);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
         if (indef2) return LOG_RTERR (pctxt, ASN_E_INVLEN);

         totalLen += ll;
         if (totalLen > bufsiz)
            return LOG_RTERR (pctxt, RTERR_STROVFLW);

         stat = rtxReadBytes (pctxt, pvalue + octidx, ll);
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         octidx = totalLen;
      }

      *pnocts = totalLen;

      if (indefLen) {
         stat = berDecStrmMatchEOC (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
   }
   else { /* primitive case */
      if (indefLen)
         return LOG_RTERR (pctxt, ASN_E_INVLEN);

      if (length > 0) {
         OSSIZE numToRead = length;

         *pnocts = numToRead;

         if (numToRead > bufsiz)
            return LOG_RTERR (pctxt, RTERR_STROVFLW);

         stat = rtxReadBytes (pctxt, pvalue, numToRead);
         if (stat < 0) return LOG_RTERR (pctxt, stat);
      }
      else {
         *pnocts = 0;
      }
   }

   LCHECKBER (pctxt);
#endif

   return 0;
}

int berDecStrmOctStr (OSCTXT *pctxt, OSOCTET* pvalue, OSUINT32* pnocts,
                      ASN1TagType tagging, int length)
{
   int ret = 0;
#ifndef _NO_STREAM
   OSSIZE numocts = *pnocts;
   OSBOOL indefLen = (OSBOOL)(length == ASN_K_INDEFLEN);
   OSSIZE len2 = (length > 0) ? (OSSIZE)length : 0;

   ret = berDecStrmOctStr64(pctxt, pvalue, &numocts, tagging, len2, indefLen);

   if (0 == ret) {
      if (sizeof(numocts) > 4 && numocts > OSUINT32_MAX)
         return (LOG_RTERR (pctxt, RTERR_TOOBIG));

      *pnocts = (OSUINT32)numocts;
   }
#endif
   return ret;
}
