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

int berDecStrmUInt (OSCTXT *pctxt, OSUINT32 *object_p,
                    ASN1TagType tagging, int length)
{
   register int stat = 0;
   OSOCTET  tmpbuf[16];
   OSUINT32 bufidx = 0;
   OSOCTET  ub;     /* unsigned */
   OSBOOL   negative;

   if (tagging == ASN1EXPL) {
      stat = berDecStrmMatchTag (pctxt, ASN_ID_INT, &length, TRUE);
      /* RTERR_IDNOTFOU will be logged later, by the generated code,
         or reset by rtxErrReset (for optional seq elements). */
      if (stat != 0)
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   /* Make sure integer will fit in target variable */

   if (length > (int)(sizeof(OSUINT32) + 1))
      return LOG_RTERR (pctxt, RTERR_TOOBIG);

   /* Read encoded integer contents into memory */

   stat = rtxReadBytes (pctxt, tmpbuf, length);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   if (length == (sizeof(OSUINT32) + 1)) {
      /* first byte must be zero */
      if (0 != tmpbuf[0])
         return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }

   /* Check for negative value */

   negative = ((tmpbuf[0] & 0x80) != 0);

   /* Now use unsigned bytes to add in the rest of the integer */

   *object_p = 0;

   while (length > 0) {
      ub = tmpbuf[bufidx++];
      *object_p = (*object_p * 256) + ub;
      length--;
   }

   if (negative) {
      OSINT32 signedValue = (OSINT32) *object_p;
      if (signedValue >= 0) signedValue = -signedValue;
      rtxErrAddStrParm (pctxt, "_uint32");
      rtxErrAddIntParm (pctxt, signedValue);
      return LOG_RTERRNEW (pctxt, RTERR_CONSVIO);
   }

   LCHECKBER (pctxt);
   return 0;
}
