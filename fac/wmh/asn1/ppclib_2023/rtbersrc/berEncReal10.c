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

#include "rtbersrc/asn1ber.h"

int xe_real10 (OSCTXT *pctxt, const char* object_p, ASN1TagType tagging)
{
   int ll0 = 0;
   int stat;
   int length = 0;
   OSOCTET form = 1;
   const char* p;
   char c;
   OSBOOL flZero = TRUE;

   if (!object_p)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   for (p = object_p; (c = *p) != 0; p++, length++) {
      if (c == '.')
         form = 2;
      else if (c == 'e' || c == 'E')
         form = 3;
      else if (form != 3 && c > '0' && c <= '9')
         flZero = FALSE;
   }

   if (!flZero) {
      stat = xe_memcpy (pctxt, (const OSOCTET*) object_p, length);
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      XE_CHKBUF(pctxt, 1)
      XE_PUT1(pctxt, form)

      ll0 += length + 1;
   }

   if (tagging == ASN1EXPL) {
      stat = xe_tag_len (pctxt, TM_UNIV|TM_PRIM|9, ll0);
      if (stat < 0) return LOG_RTERR (pctxt, stat);
      ll0 = stat;
   }

   return ll0;
}
