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

int berDecStrmReal10 (OSCTXT *pctxt, const char** object_p,
                      ASN1TagType tagging, int length)
{
   int stat;
   OSUINT8 flags, tm;
   ASN1TAG tag;

   if (!object_p)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   if (tagging == ASN1EXPL) {
      stat = berDecStrmTagAndLen (pctxt, &tag, &length);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      if (tag != (TM_UNIV|TM_PRIM|9)) return LOG_RTERR (pctxt, RTERR_INVREAL);
   }

   if (length == 0) { /* zero */
      char *p = (char*) rtxMemAlloc (pctxt, 2);
      if (p == 0) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

      p[0] = '0';
      p[1] = 0;
      *object_p = p;
   }
   else {
      stat = berDecStrmUInt8 (pctxt, &flags, ASN1IMPL, 1);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      if ((flags & 0xC0) == 0) { /* base 10 */
         tm = flags & 0x3F;
         if (tm >= 1 && tm <= 3) {
            stat = berDecStrmCharStr (pctxt, object_p, ASN1IMPL, 0,
               length - 1);
            if (stat < 0) return LOG_RTERR (pctxt, stat);
         }
         else
            return LOG_RTERR (pctxt, RTERR_INVREAL);
      }
      else if (flags & 0x80) { /* base 2 */
         return LOG_RTERR (pctxt, RTERR_INVREAL);
      }
      else { /* +- INF */
         return LOG_RTERR (pctxt, RTERR_INVREAL);
      }
   }

   return 0;
}
