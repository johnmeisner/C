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

int berDecStrmBigInt (OSCTXT *pctxt, const char** object_p,
                      ASN1TagType tagging, int length)
{
   int stat = 0;
   int bufsiz, i, ub, off = 2;
   char* tmpstr;
   OSOCTET* ptmpbuf;
   OSUINT32 bufidx = 0;
   OSBOOL leadingZeros = FALSE;

   if (tagging == ASN1EXPL) {
      stat = berDecStrmMatchTag (pctxt, ASN_ID_INT, &length, TRUE);
      /* RTERR_IDNOTFOU will be logged later, by the generated code,
         or reset by rtxErrReset (for optional seq elements). */
      if (stat != 0)
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   ptmpbuf = (OSOCTET*) rtxMemAlloc (pctxt, length);
   if (0 == ptmpbuf) return LOG_RTERR (pctxt, RTERR_NOMEM);

   stat = rtxReadBytes (pctxt, ptmpbuf, length);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   /* check if the first byte is zero */

   if (length > 0 && ptmpbuf[0] == 0) {
      leadingZeros = TRUE;
      bufidx++; length--;
   }

   /* skip all remaining leading zeros */

   while (length > 0 && ptmpbuf[bufidx] == 0) {
      bufidx++; length--;
   }

   bufsiz = (length * 2) + 4;
   tmpstr = (char*) rtxMemAlloc (pctxt, bufsiz);
   if (tmpstr != 0) {
      tmpstr[0] = '0';
      tmpstr[1] = 'x';

      if (length == 0 || (leadingZeros && (ptmpbuf[bufidx] & 0x80)))
         tmpstr [off++] = '0';

      for (i = 0; i < length; i++) {
         OSOCTET oct = ptmpbuf[bufidx];

         ub = (oct >> 4) & 0x0f;
         NIBBLETOHEXCHAR (ub, tmpstr[off++]);

         ub = (oct & 0x0f);
         NIBBLETOHEXCHAR (ub, tmpstr[off++]);

         bufidx++;
      }

      tmpstr[off] = '\0';
      *object_p = tmpstr;
   }
   else
      return LOG_RTERR (pctxt, RTERR_NOMEM);

   rtxMemFreePtr (pctxt, ptmpbuf);

   return 0;
}

int berDecStrmBigEnum (OSCTXT *pctxt, const char** object_p,
                       ASN1TagType tagging, int length)
{
   if (tagging == ASN1EXPL) {
      int stat = berDecStrmMatchTag (pctxt, ASN_ID_ENUM, &length, TRUE);
      /* RTERR_IDNOTFOU will be logged later, by the generated code,
         or reset by rtxErrReset (for optional seq elements). */
      if (stat != 0)
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   return berDecStrmBigInt (pctxt, object_p, ASN1IMPL, length);
}
