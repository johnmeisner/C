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

int berDecStrmOctStr (OSCTXT *pctxt, OSOCTET* pvalue, OSUINT32* pnocts,
                      ASN1TagType tagging, int length)
{
   register int stat = 0;
   OSUINT32 bufsiz = *pnocts;

   if (tagging == ASN1EXPL) {
      if ((stat = berDecStrmMatchTag (pctxt, ASN_ID_OCTSTR, &length, TRUE)) != 0)
      /* RTERR_IDNOTFOU will be logged later, by the generated code,
         or reset by rtxErrReset (for optional seq elements). */
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   if (pctxt->flags & ASN1CONSTAG) { /* constructed case */
      OSUINT32 totalLen = 0, octidx = 0;
      ASN1CCB ccb;

      ccb.len = length;
      ccb.bytes = OSRTSTREAM_BYTEINDEX (pctxt);
      while (!BS_CHKEND (pctxt, &ccb)) {
         int ll;
         if ((stat = berDecStrmMatchTag (pctxt, ASN_ID_OCTSTR, &ll, TRUE)) != 0)
            return LOG_RTERR (pctxt, stat);
         if (ll <= 0)
            return LOG_RTERR (pctxt, ASN_E_INVLEN);

         totalLen += ll;
         if (totalLen > bufsiz)
            return LOG_RTERR (pctxt, RTERR_STROVFLW);

         stat = rtxReadBytes (pctxt, pvalue + octidx, ll);
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         octidx = totalLen;
      }

      if (ccb.stat != 0)
         return LOG_RTERR (pctxt, ccb.stat);

      *pnocts = totalLen;

      if (length == ASN_K_INDEFLEN) {
         stat = berDecStrmMatchEOC (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
   }
   else { /* primitive case */
      if (length < 0)
         return LOG_RTERR (pctxt, ASN_E_INVLEN);
      if (length > 0) {
         int numToRead = length;

         *pnocts = numToRead;

         if (numToRead > (int)bufsiz)
            return LOG_RTERR (pctxt, RTERR_STROVFLW);

         stat = rtxReadBytes (pctxt, pvalue, numToRead);
         if (stat < 0) return LOG_RTERR (pctxt, stat);
      }
      else {
         *pnocts = 0;
      }
   }

   LCHECKBER (pctxt);

   return 0;
}
