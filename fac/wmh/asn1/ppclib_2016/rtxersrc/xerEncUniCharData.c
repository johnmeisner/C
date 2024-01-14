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

#include "xee_common.hh"

/* UTF-8 validation and conversion functions */

static OSUINT32 encoding_mask[] = {
   0xfffff800, 0xffff0000, 0xffe00000, 0xfc000000
};

static unsigned char encoding_byte[] = {
   0xc0, 0xe0, 0xf0, 0xf8, 0xfc
};

int xerEncUniCharData (OSCTXT* pctxt, const OSUNICHAR* value, OSUINT32 nchars)
{
   int stat, step;
   OSUINT32 inbufx;
   OSUNICHAR wc;

   for (inbufx = 0; inbufx < nchars; inbufx++) {
      wc = value[inbufx];

      if (wc < 0x80) {
         char c = (char) wc;
         /* One byte sequence */
         stat = xerPutCharStr (pctxt, &c, 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      else {
         char outbuf[10];
         int sz;

         for (step = 2; step < 6; ++step)
            if ((wc & encoding_mask[step - 2]) == 0)
               break;
         sz = step;

         /* Copy multi-byte sequence to output buffer */
         outbuf[0] = encoding_byte[step - 2];
         --step;
         do {
            outbuf[step] = (OSOCTET) (0x80 | (wc & 0x3f));
            wc >>= 6;
         }
         while (--step > 0);
         outbuf[0] |= wc;

         stat = xerPutCharStr (pctxt, (char*) outbuf, sz);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
   }

   return 0;
}

