/*
 * Copyright (c) 2014-2023 Objective Systems, Inc.
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

/* Encode an OER unsigned unrestricted integer */

int oerEncUnrestUInt32Value
(OSUINT32 value, OSOCTET* buffer, size_t bufsiz, size_t* pidx)
{
   OSUINT32 temp = value;
   size_t   i = bufsiz;
   int      len = 0;
   OSOCTET  lb;

   do {
      lb = (OSOCTET) (temp % 256);
      temp /= 256;
      if (0 == i)  return RTERR_BUFOVFLW;
      buffer[--i] = lb;
      len++;
   } while (temp != 0);

   *pidx = i;

   return len;
}

EXTOERMETHOD int oerEncUnrestSignedUInt32 (OSCTXT* pctxt, OSUINT32 value)
{
   if ( value <= OSINT32_MAX ) {
      /* cast to OSINT32 and use usual function */
      return oerEncUnrestInt32(pctxt, (OSINT32) value);
   }
   else {
      /* encoding will be length (1 octet, equal to 5), sign byte (zero),
         and then the unsigned value (4 bytes), for a total of 6 bytes.
      */
      OSOCTET tmpbuf[6];
      size_t i;
      int len, stat;

      len = oerEncUnrestUInt32Value (value, tmpbuf, sizeof(tmpbuf), &i);
      if (len < 0) return LOG_RTERR (pctxt, len);

      OSRTASSERT(len == 4 && i == 2); /* Should have encoded 4 bytes, with
                                       first one in tmpbuf[2]. */

      tmpbuf[0] = 5;    /* the length */
      tmpbuf[1] = 0;    /* leading sign byte */

      stat = rtxWriteBytes (pctxt, tmpbuf, 6);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      return stat;
   }
}

EXTOERMETHOD int oerEncUnrestUInt32 (OSCTXT* pctxt, OSUINT32 value)
{
   OSOCTET tmpbuf[sizeof(OSUINT32) + 4];
   size_t  i;
   int     len, stat = 0;

   len = oerEncUnrestUInt32Value (value, tmpbuf, sizeof(tmpbuf), &i);
   if (len < 0) return LOG_RTERR (pctxt, len);

   if (i == 0) return LOG_RTERR (pctxt, RTERR_BUFOVFLW);
   tmpbuf[--i] = len;

   stat = rtxWriteBytes (pctxt, (OSOCTET*)&tmpbuf[i], len + 1);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}
