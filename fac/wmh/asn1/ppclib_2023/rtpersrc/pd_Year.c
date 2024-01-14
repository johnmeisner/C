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

#include "rtpersrc/asn1per.h"
#include "rtxsrc/rtxCharStr.h"

EXTPERMETHOD int pd_YearInt (OSCTXT* pctxt, OSINT32* pvalue)
{
   OSUINT8 idx;
   OSINT32 year = 0;
   int stat = 0;

   /* decode choice index */
   stat = pd_ConsUInt8 (pctxt, &idx, OSUINTCONST(0), OSUINTCONST(3));
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   switch (idx) {
      case 0: /* immediate */
         stat = pd_ConsInteger (pctxt, &year, 2005, 2020);
         break;
      case 1: /* near future */
         stat = pd_ConsInteger (pctxt, &year, 2021, 2276);
         break;
      case 2: /* near past */
         stat = pd_ConsInteger (pctxt, &year, 1749, 2004);
         break;
      case 3: /* min .. 1748 | 2277 .. max */
         stat = pd_UnconsInteger (pctxt, &year);
         break;
   }

   if (stat < 0) return LOG_RTERR (pctxt, stat);

   *pvalue = year;

   return 0;
}
