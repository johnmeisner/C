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
#include "rtxsrc/rtxUtil.h"

EXTOERMETHOD OSSIZE oerTagLength(ASN1TAG tag)
{
   OSUINT32   ident = tag & TM_IDCODE;
   if ( ident < 63 ) return 1;
   else return 1 + rtxGetIdent64ByteCount (ident);
}

EXTOERMETHOD int oerWriteTag(ASN1TAG tag, OSOCTET* buffer, OSSIZE bufsize)
{
   int        retval;
   ASN1TAG    ltag;
   OSUINT32   ident = tag & TM_IDCODE;
   OSOCTET    b, tagClass;

   if ( bufsize == 0 ) return RTERR_BUFOVFLW;

   /* Split tag into class and ID code components */

   ltag = tag >> ((sizeof(ltag) * 8) - 2);
   tagClass = (OSOCTET)(ltag << 6);

   /* Encode components */

   if (ident < 63) {

      buffer[0] = (OSOCTET) (tagClass + ident);
      retval = 1;
   }
   else { /* multi-byte tag */
      b = (OSOCTET) (tagClass + 63L);
      buffer[0] = b;

      retval = oerWriteIdent(ident, buffer + 1, bufsize - 1);
      if ( retval >= 0 ) retval++;  /* +1 for first byte */
   }

   return retval;
}

/* OER tags differ from BER tags in the following ways:
   - Form bit (primitive or constructed) is not used.
   - Because of this, 6 bits can be used to hold the ID value in the first
     octet which makes it possible to hold a max tag value of 62 instead
     of 30 as is the case for BER. */

EXTOERMETHOD int oerEncTag (OSCTXT* pctxt, ASN1TAG tag)
{
   OSOCTET  tmpbuf[32];
   int stat;

   stat = oerWriteTag(tag, tmpbuf, sizeof(tmpbuf));
   if ( stat < 0 ) return LOG_RTERR (pctxt, stat);

   stat = rtxWriteBytes (pctxt, tmpbuf, stat);
   if (0 != stat) LOG_RTERR (pctxt, stat);

   return stat;
}
