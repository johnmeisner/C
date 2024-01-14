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

int berEncStrmInt16 (OSCTXT* pctxt, OSINT16 value, ASN1TagType tagging)
{
   register OSOCTET lb;
   OSOCTET buf[2], *p = (OSOCTET*)&buf[1];
   int  aal = 0, stat;
   register OSINT32 temp;

   temp = value;

   *p = lb = (OSOCTET) (temp % 256);

   temp /= 256;
   if (temp < 0 && lb != 0) temp--; /* two's complement adjustment */

   if (value > 0 && temp == 0) {
      if (lb & 0x80) {
         *--p = 0;
      }
   }
   else if (value < 0 && (temp == -1 || temp == 0)) {
      if (!(lb & 0x80)) {
         *--p = 0xFF;
      }
   }
   else if (temp != 0) {
      *--p  = (OSOCTET)temp;
   }
   aal = (int)(((OSOCTET*)buf) + sizeof (buf) - p);
   if (tagging == ASN1EXPL)
      stat = berEncStrmTagAndLen (pctxt, TM_UNIV|TM_PRIM|ASN_ID_INT, aal);
   else
      stat = berEncStrmLength (pctxt, aal);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   stat = berEncStrmWriteOctets (pctxt, p, aal);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}

