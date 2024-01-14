/*
 * Copyright (c) 2017-2018 Objective Systems, Inc.
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

int rtJsonEncBitStrValue (OSCTXT* pctxt, OSSIZE nbits, const OSOCTET* data)
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

int rtJsonEncBitStrValueExt(OSCTXT* pctxt, OSSIZE nbits, const OSOCTET* data,
                            OSSIZE dataSize, const OSOCTET* extData)
{
   OSSIZE dataSizeBits = dataSize * 8;
   int stat;

   /* Verify output will fit in encode buffer */
   stat = rtxCheckOutputBuffer (pctxt, nbits * 8 + 2);
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
