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
#include "rtxsrc/rtxBase64.h"

int rtJsonDecBase64Str (OSCTXT* pctxt,
                        OSOCTET* pvalue, OSUINT32* pnocts,
                        OSSIZE bufsize)
{
   OSSIZE nocts64;
   int stat = 0;

   stat = rtJsonDecBase64Str64(pctxt, pvalue, &nocts64, bufsize);
   if (stat >= 0)
   {
      if ((sizeof(nocts64) > 4) && (nocts64 > OSUINT32_MAX))
      {
         return LOG_RTERR(pctxt, RTERR_TOOBIG);
      }
      else
      {
         if (pnocts)
         {
            *pnocts = (OSUINT32)nocts64;
         }
      }
   }

   return stat;
}

int rtJsonDecBase64Str64 (OSCTXT* pctxt,
                          OSOCTET* pvalue, OSSIZE* pnocts,
                          OSSIZE bufsize)
{
   OSSIZE dstPos = 0;
   OSSIZE srcPos = 0;
   OSBOOL pad = FALSE;
   int prevVal = 0; /* remove warning C4701 */
   int stat = 0;
   OSOCTET c;

   if (pnocts)
      *pnocts = 0;

   rtJsonDecSkipWhitespace (pctxt);
   stat = rtJsonDecMatchChar (pctxt, '"');
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   for (;;) {
      int val;
      stat = rtxReadBytes (pctxt, &c, 1);
      if (0 != stat) return LOG_RTERR (pctxt, stat);

      if (c == ' ') continue;
      if (c == ',' || c == '}') {
         pctxt->buffer.byteIndex--;
         break;
      }

      /* Consume characters until ending double-quote character (") is found */
      if (c == '"') break;

      if (c == '=') {
         if ((srcPos & 3) >= 2) {
            pad = TRUE;
            srcPos++;
            continue;
         }
         else {
            stat = LOG_RTERRNEW (pctxt, RTERR_INVBASE64);
            break;
         }
      }

      if (pad) {
         stat = LOG_RTERRNEW (pctxt, RTERR_INVBASE64);
         break;
      }

      val = rtxBase64toInt(c);

      if (val < 0) {
         stat = LOG_RTERRNEW (pctxt, RTERR_INVBASE64);
         break;
      }

      if (dstPos >= bufsize) {
         stat = LOG_RTERRNEW (pctxt, RTERR_STROVFLW);
         break;
      }

      if (pvalue) {
         switch (srcPos & 3) {
         case 0:
            pvalue[dstPos] = (OSOCTET)(val << 2);
            break;
         case 1:
            pvalue[dstPos] |= (OSOCTET)(val >> 4);
            dstPos++;
            break;
         case 2:
            pvalue[dstPos] = (OSOCTET)((prevVal << 4) | (val >> 2));
            dstPos++;
            break;
         case 3:
            pvalue[dstPos] = (OSOCTET)((prevVal << 6) | val);
            dstPos++;
            break;
         }
      }
      else if ((srcPos & 3) > 0)
         dstPos++;

      srcPos++;
      prevVal = val;
   }

   if (stat >= 0 && (srcPos % 4) != 0 &&
       c != ',' && c != '}') {
      stat = LOG_RTERRNEW (pctxt, RTERR_INVBASE64);
   }

   if (stat == 0) {
      if (pnocts)
         *pnocts = dstPos;
   } else
      LOG_RTERR (pctxt, RTERR_INVBASE64);

   return stat;
}
