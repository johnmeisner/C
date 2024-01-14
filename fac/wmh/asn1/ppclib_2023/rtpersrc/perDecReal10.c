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

#ifndef _NO_ASN1REAL
#include "rtpersrc/asn1per.h"

EXTPERMETHOD int pd_Real10 (OSCTXT *pctxt, const char** object_p)
{
   int stat;
   OSUINT32 length;
   OSUINT32 flags, tm;

   stat = pd_Length (pctxt, &length);
   if (stat < 0) return LOG_RTERRNEW (pctxt, stat);

   if (length == 0) { /* zero */
      if (object_p) {
         char *p = (char*) rtxMemAlloc (pctxt, 2);
         if (p == 0) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

         p[0] = '0';
         p[1] = 0;
         *object_p = p;
      }

      PU_NEWFIELD(pctxt, "Real10");
   }
   else {
      stat = pd_byte_align (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      PU_NEWFIELD(pctxt, "Real10");

      stat = pd_bits (pctxt, &flags, 8);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      if ((flags & 0xC0) == 0) { /* base 10 */
         tm = flags & 0x3F;
         if (tm >= 1 && tm <= 3) {
            OSUINT32 i;
            char* p = 0;

            if (object_p) {
               p = (char*) rtxMemAlloc (pctxt, length);
               if (p == 0) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
            }

            length--;

            for (i = 0; i < length; i++) {
               stat = pd_bits (pctxt, &tm, 8);
               if (stat < 0) return LOG_RTERR (pctxt, stat);

               if (object_p) p[i] = (char)tm;
            }

            if (object_p) {
               p[length] = 0;
               *object_p = p;
            }
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

   PU_SETBITCOUNT(pctxt);
   return 0;
}

#endif
