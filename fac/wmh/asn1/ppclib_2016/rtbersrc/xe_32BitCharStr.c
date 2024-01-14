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

#include "rtbersrc/asn1ber.h"

int xe_32BitCharStr (OSCTXT* pctxt, Asn132BitCharString* object_p,
                     ASN1TagType tagging, ASN1TAG tag)
{
   OSOCTET lbuf[4];
   OSSIZE i;
   int ll, ll0 = 0;

   if (0 == object_p) return LOG_RTERR(pctxt, RTERR_BADVALUE);
   if (0 == object_p->nchars) return 0;

   for (i = object_p->nchars - 1; ; ) {
      OS32BITCHAR sym = object_p->data[i];
      lbuf[0] = (OSOCTET) (sym / 256 / 256 / 256);
      lbuf[1] = (OSOCTET) (sym / 256 / 256 % 256);
      lbuf[2] = (OSOCTET) (sym / 256 % 256 );
      lbuf[3] = (OSOCTET) (sym % 256);

      ll = xe_memcpy (pctxt, lbuf, 4);
      if (ll >= 0) ll0 += ll; else return (ll);

      if (0 == i) break; else i--;
   }

   if (tagging == ASN1EXPL)
      ll0 = xe_tag_len (pctxt, tag, ll0);

   return (ll0);
}

