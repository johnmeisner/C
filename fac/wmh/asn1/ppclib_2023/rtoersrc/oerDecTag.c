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

#include "rtoersrc/asn1oer.h"

/* OER tags differ from BER tags in the following ways:
   - Form bit (primitive or constructed) is not used.
   - Because of this, 6 bits can be used to hold the ID value in the first
     octet which makes it possible to hold a max tag value of 62 instead
     of 30 as is the case for BER. */

EXTOERMETHOD int oerDecTag (OSCTXT* pctxt, ASN1TAG* ptag)
{
   ASN1TAG tagclass, id_code;
   OSINT32 lcnt = 0, b, stat;
   OSOCTET ub;

   stat = rtxReadBytes (pctxt, &ub, 1);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   b = ub; *ptag = 0;

   tagclass = (ASN1TAG)(b & TM_CLASS);
   tagclass <<= ASN1TAG_LSHIFT;

   if ((id_code = (b & 63L)) == 63L) {
      id_code = 0;
      do {
         stat = rtxReadBytes (pctxt, &ub, 1);
         if (stat < 0) return stat;

         b = ub;
         id_code = (id_code * 128) + (b & 0x7F);

         if (id_code > 63L || lcnt++ > 8)
            return LOG_RTERR (pctxt, ASN_E_BADTAG);

      } while (b & 0x80);
   }

   *ptag = tagclass | id_code;

   return (0);
}
