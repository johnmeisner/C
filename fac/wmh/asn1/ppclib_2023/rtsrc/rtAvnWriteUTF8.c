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

#include "rtAvn.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxError.h"

#ifndef UTF8WRITEFN
#define UTF8WRITEFN BogusFunctionToMakeIdeHappy
#define UTF8WRITEFN_CHAR OSUNICHAR
#endif


/* Mask used to check for a value using 2, 3, 4, 5 bytes in UTF-8. */
static const OSUINT32 encoding_mask[] = {
   0xfffff800, 0xffff0000, 0xffe00000, 0xfc000000
};

/* The value of the high bits for the first byte when there are 2, 3, 4, 5, 6
   bytes in the UTF-8 encoding
*/
static const unsigned char encoding_byte[] = {
   0xc0, 0xe0, 0xf0, 0xf8, 0xfc
};

EXTRTMETHOD int UTF8WRITEFN(OSCTXT* pctxt, const UTF8WRITEFN_CHAR* str,
                              OSSIZE nchars)
{
   /* 8 bits less than that size of each character.  This is the # of bits to
      right shift to move highest byte to lowest byte.
   */
   const OSSIZE RIGHT = (sizeof(UTF8WRITEFN_CHAR) - 1) * 8;
   OSUINT32 inbufx;
   UTF8WRITEFN_CHAR wc;

   OSRTSAFEPUTCHAR(pctxt, '"');

   for (inbufx = 0; inbufx < nchars; inbufx++) {
      wc = str[inbufx];

      if (wc < 0x80) {
         char c = (char)wc;
         /* One byte sequence */
         OSRTSAFEPUTCHAR(pctxt, c);

         /* Double up double-quotes. */
         if ( c == '"') OSRTSAFEPUTCHAR(pctxt, c);
      }
      else {
         int nbytes;
         OSOCTET b;
         int ret;

         /* Determine the number of bytes in the UTF-8 encoding. */
         for (nbytes = 2; nbytes < 6; ++nbytes) {
            if ((wc & encoding_mask[nbytes - 2]) == 0)
               break;
         }

         ret = rtxCheckOutputBuffer(pctxt, nbytes);
         if ( ret < 0 ) return LOG_RTERR(pctxt, 0);

         /* The total bits to be encoded = 5 * nbytes + 1.  Shift left so
            these bits are in the highest position (they might already be
            there.
         */
         wc = wc << (sizeof(UTF8WRITEFN_CHAR) * 8 - (5 * nbytes + 1));

         /* The first byte of the UTF-8 encoding uses nbytes+1 bits as signal
            bits.  Right shift so that the highest byte is moved to the lowest
            byte, plus the additional nybtes + 1 bits.  The set the high
            nbytes + 1 bits in the byte.
         */
         b = (OSOCTET)
               (wc >> ( nbytes + 1 + RIGHT ));
         b = b | encoding_byte[nbytes - 2];

         OSRTPUTCHAR(pctxt, b);


         /* Encode remaining bytes (there's at least one). These use '10'b and
            6 bits of the value.

            The first byte used 7 - nbytes bits of the value.
            Shift left 7 - nbytes bits and then right 2 bits to position
            next 6 bits in the low 6 bits of the highest byte.
         */
         if (5 > nbytes)
         {
            wc = wc << (5 - nbytes);
         }
         else if (5 < nbytes)
         {
            /* Net effect is right shift. */
            wc = wc >> (nbytes - 5);
         }

         nbytes--;

         for(;;)
         {
            b = (OSOCTET) (wc >> RIGHT );
            b = (b & 0x3F) | 0x80;

            OSRTPUTCHAR(pctxt, b);

            nbytes--;

            if ( nbytes == 0 ) break;
            else {
               /* Move next 6 bits into position. */
               wc = wc << 6;
            }
         }
      }
   }

   OSRTSAFEPUTCHAR(pctxt, '"');

   return 0;
}
