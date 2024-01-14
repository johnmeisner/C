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

int oerEncUnrestUInt64Value (OSUINT64 value, OSOCTET* tmpbuf, size_t bufsiz,
                              size_t* pidx )
{
   OSOCTET lb;
   size_t  i = bufsiz;
   OSOCTET len = 0;

   do {
      lb = (OSOCTET) (value % 256);
      value /= 256;
      tmpbuf[--i] = lb;
      if (0 == i)  return RTERR_BUFOVFLW;
      len++;
   } while (value != 0 && i > 0);

   *pidx = i;

   return len;
}

EXTOERMETHOD int oerEncUnrestUInt64 (OSCTXT* pctxt, OSUINT64 value)
{
   OSOCTET tmpbuf[sizeof(OSINT64) + 4];
   size_t  i;
   int     stat = 0;
   OSOCTET len;

   stat = oerEncUnrestUInt64Value(value, tmpbuf, sizeof(tmpbuf), &i);
   if ( stat < 0 ) return LOG_RTERRNEW(pctxt, stat);
   else len = (OSOCTET)stat;

   tmpbuf[--i] = len;

   stat = rtxWriteBytes (pctxt, (OSOCTET*)&tmpbuf[i], len + 1);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}

EXTOERMETHOD int oerEncUnrestSignedUInt64 (OSCTXT* pctxt, OSUINT64 value)
{
   if ( value <= OSINT64MAX ) {
      /* cast to OSINT64 and use usual function */
      return oerEncUnrestInt64(pctxt, (OSINT64) value);
   }
   else {
      /* encoding will be length (1 octet, equal to 9), sign byte (zero),
         and then the unsigned value (8 bytes), for a total of 10 bytes.
      */
      OSOCTET tmpbuf[10];
      size_t i;
      int len, stat;

      len = oerEncUnrestUInt64Value (value, tmpbuf, sizeof(tmpbuf), &i);
      if (len < 0) return LOG_RTERR (pctxt, len);

      OSRTASSERT(len == 8 && i == 2); /* Should have encoded 8 bytes, with
                                       first one in tmpbuf[2]. */

      tmpbuf[0] = 9;    /* the length */
      tmpbuf[1] = 0;    /* leading sign byte */

      stat = rtxWriteBytes (pctxt, tmpbuf, 10);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      return stat;
   }
}
