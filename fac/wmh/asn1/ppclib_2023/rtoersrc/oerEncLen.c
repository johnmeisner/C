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
#include "rtxsrc/rtxUtil.h"

EXTOERMETHOD OSSIZE oerLenLength(size_t length)
{
   if ( length <= 127 ) return 1;
   else return 1 + rtxSizeByteCount(length);
}

EXTOERMETHOD int oerWriteLen (size_t length, OSOCTET* buffer, OSSIZE bufsize)
{
   if (length > 127) {
      int nbytes;
      size_t tmpidx;

      /* bits to shift currect octet of value into least sig byte */
      int shift = 8 * (sizeof(OSSIZE) - 1 );

      OSOCTET b;

      /* skip over leading zero bytes */
      while ( 0 == (b = (OSOCTET) (length >> shift) ) )
      {
         shift -= 8;
      }

      /* assert: b is the next octet to write; shift is the # of bits
         we had to shift to get it.
      */

      nbytes = 1 + shift/8;

      /* Set the length descriptor. */
      buffer[0] = 0x80 | nbytes;

      tmpidx = 1;
      for(;;) {
         if ( tmpidx >= bufsize ) return RTERR_TOOBIG;

         buffer[tmpidx++] = b;

         if ( shift == 0 ) break;   /* b was the LSB; all done */
         else {
            shift -= 8;
            b = (OSOCTET) (length >> shift);
         }
      }

      return nbytes + 1;
   }
   else {
      /* Write single length byte (short form) */
      if ( bufsize == 0 ) return RTERR_TOOBIG;

      buffer[0] = (OSOCTET) length;
      return 1;
   }
}


/* Encode an OER length.  This is identical to BER length encoding except
   that indefinite length form is not currently supported. */

EXTOERMETHOD int oerEncLen (OSCTXT* pctxt, size_t length)
{
   int stat = 0;

   if (length > 127) {
      OSOCTET tmpbuf[16];

      stat = oerWriteLen(length, tmpbuf, sizeof(tmpbuf));
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      /* Copy encoded length to output stream or buffer */
      stat = rtxWriteBytes (pctxt, tmpbuf, (OSSIZE)stat);
   }
   else {
      /* Write single length byte (short form) */
      OSOCTET b = (OSOCTET) length;
      stat = rtxWriteBytes (pctxt, &b, 1);
   }

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}
