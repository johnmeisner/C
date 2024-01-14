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

#include "rtxsrc/rtxErrCodes.h"
#include "rtxmlsrc/osrtxml.hh"

/* UTF-8 validation and conversion functions */

static const OSUINT32 encoding_mask[] = {
   0xfffff800, 0xffff0000, 0xffe00000, 0xfc000000
};

static const unsigned char encoding_byte[] = {
   0xc0, 0xe0, 0xf0, 0xf8, 0xfc
};

static int rtXmlEncUnicodeData
(OSCTXT* pctxt, const OSUNICHAR* value, OSSIZE nchars)
{
   int step;
   OSSIZE inbufx;
   OSUNICHAR wc;

   for (inbufx = 0; inbufx < nchars; inbufx++) {
      wc = value[inbufx];

      if (wc < 0x80) {
         char c = (char) wc;
         /* One byte sequence */
         rtXmlPutChar (pctxt, c);
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

         rtXmlWriteChars (pctxt, (void*)outbuf, sz);
      }
   }

   return 0;
}

EXTXMLMETHOD int rtXmlEncUnicodeStr
(OSCTXT* pctxt, const OSUNICHAR* value, OSSIZE nchars,
 const OSUTF8CHAR* elemName, OSXMLNamespace* pNS)
{
   int stat;

   if (0 != elemName) {
      stat = rtXmlEncStartElement (pctxt, elemName, pNS, 0, TRUE);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   /* Encode string contents */

   if (0 != value && nchars > 0) {
      if (pctxt->state != OSXMLATTR)
         pctxt->state = OSXMLDATA;

      stat = rtXmlEncUnicodeData (pctxt, value, nchars);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   if (0 != elemName) {
      stat = rtXmlEncEndElement (pctxt, elemName, pNS);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}


