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

#include "rtpersrc/asn1per.h"
#include "rtxsrc/rtxUtil.h"

/* This method encodes an ASN.1 identifier value */

int pe_identifier (OSCTXT* pctxt, OSUINT32 ident) {
   OSUINT32 mask;
   int nshifts = 0, stat;

   if (ident !=0) {
      OSUINT32 lv;
      nshifts = rtxGetIdentByteCount(ident);
      while (nshifts > 0) {
         /* avoid mask overflow (AB, 06/15/2002) */
         mask = ((OSUINT32)0x7f) << (7 * (nshifts - 1));
         nshifts--;
         lv = (OSUINT32)((ident & mask) >> (nshifts * 7));
         if (nshifts != 0) { lv |= 0x80; }
         if ((stat = pe_bits (pctxt, lv, 8)) != 0)
            return LOG_RTERR (pctxt, stat);
      }
   }
   else {
      /* encode a single zero byte */
      if ((stat = pe_bits (pctxt, 0, 8)) != 0)
         return LOG_RTERR (pctxt, stat);
   }
   return 0;
}
