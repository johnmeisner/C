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

int oerDecUnrestUInt64 (OSCTXT* pctxt, OSUINT64* pvalue)
{
   register int stat = 0;
   OSOCTET  tmpbuf[16];
   OSUINT32 bufidx = 0;
   OSOCTET  ub;     /* unsigned */
   OSOCTET  len;

   /* Length will be short form (a single byte) */

   stat = rtxReadBytes (pctxt, &len, 1);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   /* Make sure integer will fit in target variable */

   if (len > (sizeof(OSUINT64))) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }

   /* Read encoded integer contents into memory */

   stat = rtxReadBytes (pctxt, tmpbuf, len);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   /* Decode integer contents */

   *pvalue = 0;

   while (len > 0) {
      ub = tmpbuf[bufidx++];
      *pvalue = (*pvalue * 256) + ub;
      len--;
   }

   return 0;
}


EXTOERMETHOD int oerDecUInt64 (OSCTXT* pctxt, OSUINT64* pvalue)
{
   OSOCTET tmpbuf[8];

   int stat = rtxReadBytes (pctxt, tmpbuf, 8);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   if (0 != pvalue) {
      int i;
      *pvalue = tmpbuf[0];
      for (i = 1; i < 8; i++) {
         *pvalue = (*pvalue * 256) + tmpbuf[i];
      }
   }

   return 0;
}
