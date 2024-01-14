
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

#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxErrCodes.h"
#include "osrtjson.h"

/* This file depends on predefined constants defined in corresponding
 * source files:
 *
 * OSJSONUNIENCFUNC (e.g., rtJsonEncUnicodeData)
 * OSJSONUNICHAR    (e.g., OSUNICHAR or OS32BITCHAR) */

static const OSUINT32 encoding_mask[] = {
   0xfffff800, 0xffff0000, 0xffe00000, 0xfc000000
};

static const unsigned char encoding_byte[] = {
   0xc0, 0xe0, 0xf0, 0xf8, 0xfc
};

int OSJSONUNIENCFUNC
(OSCTXT* pctxt, const OSJSONUNICHAR* value, OSSIZE nchars)
{
   int step;
   OSUINT32 inbufx;
   OSJSONUNICHAR wc;

   OSRTSAFEPUTCHAR (pctxt, '"');

   for (inbufx = 0; inbufx < nchars; inbufx++) {
      wc = value[inbufx];

      if (wc < 0x80) {
         char c = (char) wc;
         /* One byte sequence */
         OSRTSAFEPUTCHAR (pctxt, c);
      }
      else {
         char *outbuf;
         int sz;

         for (step = 2; step < 6; ++step)
            if ((wc & encoding_mask[step - 2]) == 0)
               break;
         sz = step;

         outbuf = (char *)rtxMemAllocZ (pctxt, sz+1);

         /* Copy multi-byte sequence to output buffer */
         outbuf[0] = encoding_byte[step - 2];
         --step;
         do {
            outbuf[step] = (OSOCTET) (0x80 | (wc & 0x3f));
            wc >>= 6;
         }
         while (--step > 0);
         outbuf[0] |= wc;

         rtxCopyAsciiText (pctxt, (void*)outbuf);

         rtxMemFreePtr (pctxt, (void *)outbuf);
      }
   }

   OSRTSAFEPUTCHAR (pctxt, '"');

   return 0;
}


