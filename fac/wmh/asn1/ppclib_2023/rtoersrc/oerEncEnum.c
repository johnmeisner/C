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

int oerEncUnrestInt32Value
(OSINT32 value, OSOCTET* buffer, size_t bufsiz, size_t* pidx);

int oerEncUnrestUInt32Value
(OSUINT32 value, OSOCTET* buffer, size_t bufsiz, size_t* pidx);

/* Encode an OER enumerated value (2.3.3.2) */

EXTOERMETHOD int oerEncSignedEnum (OSCTXT* pctxt, OSINT32 value)
{
   int stat = 0;

   if (value < 0 || value > 127) {
      OSOCTET tmpbuf[sizeof(OSINT64) + 4];
      size_t  i;

      int len = oerEncUnrestInt32Value (value, tmpbuf, sizeof(tmpbuf), &i);
      if (len < 0) return LOG_RTERR (pctxt, len);

      if (i == 0) return LOG_RTERR (pctxt, RTERR_BUFOVFLW);
      else tmpbuf[--i] = (OSOCTET)(len | 0x80);

      /* Copy encoded value to output stream or buffer */
      stat = rtxWriteBytes (pctxt, &tmpbuf[i], len + 1);
   }
   else {
      /* Write single byte (short form) */
      OSOCTET b = (OSOCTET) value;
      stat = rtxWriteBytes (pctxt, &b, 1);
   }

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}

EXTOERMETHOD int oerEncUnsignedEnum (OSCTXT* pctxt, OSUINT32 value)
{
   return ( value > OSINT32_MAX ) ?
      LOG_RTERR (pctxt, RTERR_TOOBIG) :
      oerEncSignedEnum(pctxt, (OSINT32)value);
}
