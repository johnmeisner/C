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

#include "rtpersrc/pe_common.hh"
#include "rtxsrc/rtxDateTime.hh"

EXTPERMETHOD int pe_YearInt (OSCTXT* pctxt, OSINT32 value)
{
   int stat = 0;

   if (value >= 2005) {
      if (value <= 2020) { /* immediate */
         stat = pe_ConsUnsigned (pctxt, 0, OSUINTCONST(0), OSUINTCONST(3));
         if (stat == 0)
            stat = pe_ConsInteger (pctxt, value, 2005, 2020);
      }
      else if (value <= 2276) { /* near future */
         stat = pe_ConsUnsigned (pctxt, 1, OSUINTCONST(0), OSUINTCONST(3));
         if (stat == 0)
            stat = pe_ConsInteger (pctxt, value, 2021, 2276);
      }
      else {
         stat = pe_ConsUnsigned (pctxt, 3, OSUINTCONST(0), OSUINTCONST(3));
         if (stat == 0)
            stat = pe_UnconsInteger (pctxt, value);
      }
   }
   else if (value >= 1749)  { /* near past */
      stat = pe_ConsUnsigned (pctxt, 2, OSUINTCONST(0), OSUINTCONST(3));
      if (stat == 0)
         stat = pe_ConsInteger (pctxt, value, 1749, 2004);
   }
   else {
      stat = pe_ConsUnsigned (pctxt, 3, OSUINTCONST(0), OSUINTCONST(3));
      if (stat == 0)
         stat = pe_UnconsInteger (pctxt, value);
   }

   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return stat;
}
