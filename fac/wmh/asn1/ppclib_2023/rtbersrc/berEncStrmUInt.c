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

#include "rtbersrc/xse_common.hh"

int berEncStrmUInt (OSCTXT* pctxt, OSUINT32 value, ASN1TagType tagging)
{
   OSOCTET barr[sizeof(OSUINT32) + 1], lb;
   register int i = sizeof(OSUINT32) + 1, aal = 0;
   int stat;
   OSUINT32 temp;

   temp = value;

   do {
      lb = (OSOCTET) (temp % 256);
      temp /= 256;
      barr[--i] = lb;
      aal++;
   } while (temp != 0 && i > 0);

   /* If bit 8 of the leading byte is set, copy a zero byte to the      */
   /* contents to signal a positive number..                            */

   if (lb & 0x80) {
      barr[--i] = 0;
      aal++;
   }

   if (tagging == ASN1EXPL)
      stat = berEncStrmTagAndLen (pctxt, TM_UNIV|TM_PRIM|ASN_ID_INT, aal);
   else
      stat = berEncStrmLength (pctxt, aal);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   stat = berEncStrmWriteOctets (pctxt, (OSOCTET*)&barr[i], aal);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}
