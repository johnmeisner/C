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

int berEncStrmInt64 (OSCTXT* pctxt, OSINT64 value, ASN1TagType tagging)
{
   OSOCTET barr[sizeof(OSINT64) + 1], lb;
   int i = sizeof(OSINT64) + 1, aal = 0, stat = 0;
   OSINT64 temp;

   temp = value;

   do {
      lb = (OSOCTET) (temp % 256);
      temp /= 256;
      if (temp < 0 && lb != 0) temp--; /* two's complement adjustment */
      barr[--i] = lb;
      aal++;
   } while (temp != 0 && temp != -1 && i > 0);

   /* If the value is positive and bit 8 of the leading byte is set,    */
   /* copy a zero byte to the contents to signal a positive number..    */

   if (value > 0 && (lb & 0x80))
   {
      barr[--i] = 0;
      aal++;
   }

   /* If the value is negative and bit 8 of the leading byte is clear,  */
   /* copy a -1 byte (0xFF) to the contents to signal a negative        */
   /* number..                                                          */

   else if (value < 0 && ((lb & 0x80) == 0))
   {
      barr[--i] = 0xFF;
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

