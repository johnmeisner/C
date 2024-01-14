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
#include "rtxsrc/rtxBuffer.h"

/* Decode an OER length.  This is identical to BER length encoding except
   that indefinite length form is not currently supported. */

EXTOERMETHOD int oerDecLen (OSCTXT* pctxt, OSSIZE* plength)
{
   OSOCTET b;
   int stat = rtxReadBytes (pctxt, &b, 1);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   if (b > 0x80) {
      OSOCTET nbytes = (OSOCTET)(b & 0x7F);
      OSOCTET lbuf[sizeof(size_t)];
      OSSIZE length;
      OSOCTET i;

      /* Guard against length variable overflow */
      if (nbytes > sizeof(size_t))
         return LOG_RTERR (pctxt, ASN_E_INVLEN);

      stat = rtxReadBytes (pctxt, lbuf, nbytes);
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      /* compute the length.  This is required in order to return an
         ASN_E_NOTCANON status if the length is not canonical. */
      length = 0;
      for (i = 0; i < nbytes; i++) {
         length = (length * 256) + lbuf[i];
      }

      if ( plength != 0 ) *plength = length;

      if ( length <= 127 )
      {
         /* Length < 128 encoded using long form */
         return ASN_E_NOTCANON;
      }
   }
   else if (b < 0x80) {
      if (0 != plength) *plength = b;
   }
   else { /* b == 0x80 */
      /* Indefinite length is currently not supported in OER (2.2.3.3) */
      return LOG_RTERR (pctxt, ASN_E_INVLEN);
   }

   return 0;
}

EXTOERMETHOD int oerDecLen32 (OSCTXT* pctxt, OSUINT32* plength)
{
   OSSIZE len;
   int ret = oerDecLen (pctxt, &len);
   if (0 != ret && ASN_E_NOTCANON != ret)
      return LOG_RTERR (pctxt, ret);
   else if (sizeof(len) > 4 && len > OSUINT32_MAX)
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   else {
      if (0 != plength) *plength = (OSUINT32) len;
      return ret;
   }
}

