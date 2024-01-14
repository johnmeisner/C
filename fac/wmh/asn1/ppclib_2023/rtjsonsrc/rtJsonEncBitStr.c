/*
 * Copyright (c) 2017-2023 Objective Systems, Inc.
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

#include "osrtjson.h"

static void encBitStrContent (OSCTXT* pctxt, OSSIZE nbits, const OSOCTET* data)
{
   OSSIZE i;
   OSUINT32 mask = 0x80;
   char c;

   for (i = 0; i < nbits; i++) {
      /* output a 1 or 0 depending on the mask */
      c = (data[i/8] & mask) ? '1' : '0';
      OSRTPUTCHAR (pctxt, c);
      mask >>= 1;

      /* reset the mask at the end */
      if (mask == 0) mask = 0x80;
   }
}

int rtJsonEncBitStrValueV72 (OSCTXT* pctxt, OSSIZE nbits, const OSOCTET* data)
{
   int stat;
   char dq = '"';

   /* Verify output will fit in encode buffer */

   stat = rtxCheckOutputBuffer (pctxt, nbits*8 + 2);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   OSRTPUTCHAR (pctxt, dq);
   encBitStrContent (pctxt, nbits, data);
   OSRTPUTCHAR (pctxt, dq);

   return 0;
}

int rtJsonEncBitStrValueExtV72(OSCTXT* pctxt, OSSIZE nbits, const OSOCTET* data,
                            OSSIZE dataSize, const OSOCTET* extData)
{
   OSSIZE dataSizeBits = dataSize * 8;
   int stat;

   /* Verify output will fit in encode buffer */
   stat = rtxCheckOutputBuffer (pctxt, nbits + 2);
   if (stat != 0)
   {
      return LOG_RTERR (pctxt, stat);
   }

   if (nbits <= dataSizeBits) {
      stat = rtJsonEncBitStrValue (pctxt, nbits, data);
   }
   else {
      OSRTSAFEPUTCHAR (pctxt, '"');

      /* encode all bits in the static array */
      encBitStrContent (pctxt, dataSizeBits, data);

      /* encode bits in the extended data array */
      encBitStrContent (pctxt, nbits - dataSizeBits, extData);

      OSRTSAFEPUTCHAR (pctxt, '"');
   }

   return 0;
}

int rtJsonEncBitStrValue (OSCTXT* pctxt, OSSIZE nbits, const OSOCTET* data)
{
   int stat;

   OSRTSAFEPUTCHAR(pctxt, '{');
   rtJsonEncIncrIndent(pctxt);

   /* encode the value */
   rtJsonEncIndent(pctxt);
   stat = rtJsonEncChars(pctxt, "value", 5);
   if ( stat != 0 ) return LOG_RTERR(pctxt, stat);

   OSRTSAFEPUTCHAR(pctxt, ':');

   /* encode the hex value; this part is the same as if fixed length */
   stat = rtJsonEncFixedBitStrValue(pctxt, nbits, data);
   if ( stat != 0 ) return LOG_RTERR(pctxt, stat);

   OSRTSAFEPUTCHAR(pctxt, ',');

   /* encode the length */
   rtJsonEncIndent(pctxt);

   stat = rtJsonEncChars(pctxt, "length", 6);
   if ( stat != 0 ) return LOG_RTERR(pctxt, stat);

   OSRTSAFEPUTCHAR(pctxt, ':');

   /* encode the length value */
   stat = rtJsonEncUInt64Value(pctxt, nbits);
   if ( stat != 0 ) return LOG_RTERR(pctxt, stat);

   rtJsonEncDecrIndent(pctxt);
   rtJsonEncIndent(pctxt);
   OSRTSAFEPUTCHAR(pctxt, '}');

   return 0;
}

int rtJsonEncFixedBitStrValue (OSCTXT* pctxt, OSSIZE nbits, const OSOCTET* data)
{
   int stat;
   OSSIZE nocts;
   OSSIZE finalBits;    /* number of bits in last octet; < 8 */

   /* encode the hex value */
   nocts = nbits / 8;

   OSRTSAFEPUTCHAR(pctxt, '"');

   stat = rtJsonEncHexValue(pctxt, nocts, data);
   if ( stat != 0 ) return LOG_RTERR(pctxt, stat);

   finalBits = nbits % 8;
   if ( finalBits != 0 )
   {
      /* Encode final octet with unused bits set to zero */
      OSOCTET mask = 0xFF << (8 - finalBits );
      OSOCTET value = data[nocts] & mask;
      stat = rtJsonEncHexValue(pctxt, 1, &value);
      if ( stat != 0 ) return LOG_RTERR(pctxt, stat);
   }

   OSRTSAFEPUTCHAR(pctxt, '"');

   return 0;
}

int rtJsonEncBitStrValueExt (OSCTXT* pctxt, OSSIZE nbits,
                        const OSOCTET* data, OSSIZE dataSize,
                        const OSOCTET* extData)
{
   int stat;
   OSSIZE nocts;
   OSSIZE finalBits;

   if ( nbits <= dataSize * 8 )
   {
      /* All nbits come from data; extData is irrelevant */
      return rtJsonEncBitStrValue(pctxt, nbits, data);
   }

   OSRTSAFEPUTCHAR(pctxt, '{');
   rtJsonEncIncrIndent(pctxt);

   /* encode the value */
   rtJsonEncIndent(pctxt);
   stat = rtJsonEncChars(pctxt, "value", 5);
   if ( stat != 0 ) return LOG_RTERR(pctxt, stat);

   OSRTSAFEPUTCHAR(pctxt, ':');
   OSRTSAFEPUTCHAR(pctxt, '"');

   /* encode the hex value for the part in data, then for the part in extData*/

   stat = rtJsonEncHexValue(pctxt, dataSize, data);
   if ( stat != 0 ) return LOG_RTERR(pctxt, stat);

   /* Avoiding overflow, figure out how many bytes comes from extData. */
   nocts = nbits / 8;
   finalBits = nbits % 8;
   nocts = nocts - dataSize;     /* # of full octets to encode from extData */

   stat = rtJsonEncHexValue(pctxt, nocts, extData);
   if ( stat != 0 ) return LOG_RTERR(pctxt, stat);

   if ( finalBits > 0 )
   {
      /* Encode final octet with unused bits set to zero */
      OSOCTET mask = 0xFF << (8 - finalBits );
      OSOCTET value = extData[nocts] & mask;
      stat = rtJsonEncHexValue(pctxt, 1, &value);
      if ( stat != 0 ) return LOG_RTERR(pctxt, stat);
   }

   OSRTSAFEPUTCHAR(pctxt, '"');
   OSRTSAFEPUTCHAR(pctxt, ',');

   /* encode the length */
   rtJsonEncIndent(pctxt);

   stat = rtJsonEncChars(pctxt, "length", 6);
   if ( stat != 0 ) return LOG_RTERR(pctxt, stat);

   OSRTSAFEPUTCHAR(pctxt, ':');

   /* encode the length value */
   stat = rtJsonEncUInt64Value(pctxt, nbits);
   if ( stat != 0 ) return LOG_RTERR(pctxt, stat);

   rtJsonEncDecrIndent(pctxt);
   rtJsonEncIndent(pctxt);
   OSRTSAFEPUTCHAR(pctxt, '}');

   return 0;
}
