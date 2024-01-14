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

#include "rtoersrc/asn1oer.h"
#include "rtxsrc/rtxUtil.h"

/* This routine encodes an ASN.1 identifier as used in tag and object
   identifier definitions.  The encoding defined to be a series of octets,
   each of which contains a 7 bit unsigned number. The 8th bit (MSB) of
   each octet is used as a continuation flag to indicate that more
   octets follow in the sequence. */

int oerEncIdent (OSCTXT *pctxt, OSUINT64 ident)
{
   OSUINT64 mask = 0x7f;
   int nshifts, stat;

   /* Find starting point in identifier value */
   nshifts = rtxGetIdent64ByteCount (ident);
   mask <<= (7 * nshifts);

   /* Encode bytes */
   if (nshifts > 0) {
      OSUINT32 lv;
      OSOCTET  tmpbuf[32];
      size_t   idx = 0;

      while (nshifts > 0) {
         mask = ((OSUINT64)0x7f) << (7 * (nshifts - 1));
         nshifts--;
         lv = (OSUINT32)((ident & mask) >> (nshifts * 7));
         if (nshifts != 0) { lv |= 0x80; }

         if (idx < sizeof(tmpbuf)) {
            tmpbuf[idx++] = (OSOCTET) lv;
         }
         else return RTERR_BUFOVFLW;
      }

      stat = rtxWriteBytes (pctxt, tmpbuf, idx);
   }
   else {
      /* encode a single zero byte */
      OSOCTET b = 0;
      stat = rtxWriteBytes (pctxt, &b, 1);
   }

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}
