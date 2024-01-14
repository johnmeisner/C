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

int berDecStrmUnivStr (OSCTXT* pctxt, Asn132BitCharString* object_p,
                       ASN1TagType tagging, int length)
{
   OSUINT32 i;
   int      stat;

   if (tagging == ASN1EXPL) {
      stat = berDecStrmMatchTag (pctxt, ASN_ID_UniversalString, &length, TRUE);
      /* RTERR_IDNOTFOU will be logged later, by the generated code,
         or reset by rtxErrReset (for optional seq elements). */
      if (stat != 0)
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   if (0 == length) {
      object_p->nchars = 0;
      object_p->data = 0;
   }
   else {
      OSUINT32 ll = 0;
      const OSOCTET* data;

      stat = berDecStrmDynOctStr
         (pctxt, &data, &ll, ASN1IMPL, length);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      object_p->data = (OS32BITCHAR*)data;
      if (0 != data) {
         object_p->nchars = ll / 4;
         for (i = 0; i < object_p->nchars; i++, data += 4) {
            object_p->data[i] = (OS32BITCHAR) (((data[0] * 256) + data[1]) *
               65536UL + (((data[2] * 256) + data[3])));
         }
      }
      else
         object_p->nchars = 0;
   }

   return 0;
}
