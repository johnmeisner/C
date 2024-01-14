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
#include "rtxsrc/rtxBuffer.h"

/* Encode an OER length.  This is identical to BER length encoding except
   that indefinite length form is not currently supported. */

EXTOERMETHOD int oerEncLen (OSCTXT* pctxt, size_t length)
{
   int stat = 0;

   if (length > 127) {
      OSOCTET tmpbuf[16], nbytes = 0;
      size_t  tmpidx = sizeof(tmpbuf) - 1;
      do {
         /* We check for 1 byte left instead of zero in order to save space
            for the length descriptor byte below. */
         if (1 == tmpidx) return LOG_RTERR (pctxt, RTERR_TOOBIG);

         /* Shave off LSB of length and add to temp buffer*/
         tmpbuf[tmpidx--] = (OSOCTET) (length % 256);

         length /= 256; nbytes++;
      }
      while (length > 0);

      tmpbuf[tmpidx] = nbytes | 0x80;

      /* Copy encoded length to output stream or buffer */
      stat = rtxWriteBytes (pctxt, &tmpbuf[tmpidx], nbytes + 1);
   }
   else {
      /* Write single length byte (short form) */
      OSOCTET b = (OSOCTET) length;
      stat = rtxWriteBytes (pctxt, &b, 1);
   }

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}
