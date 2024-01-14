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

#include "rtjsonsrc/osrtjson.h"

int rtJsonDecHexStr
(OSCTXT* pctxt, OSOCTET* pvalue, OSUINT32* pnocts, size_t bufsize)
{
   OSSIZE nocts64;
   int stat = 0;

   stat = rtJsonDecHexStr64(pctxt, pvalue, &nocts64, bufsize);
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

int rtJsonDecHexStr64
(OSCTXT* pctxt, OSOCTET* pvalue, OSSIZE* pnocts, size_t bufsize)
{
   OSSIZE oldsize = 0;
   int stat = 0;
   OSUTF8CHAR c;

   if (pnocts)
      *pnocts = 0;

   rtJsonDecSkipWhitespace (pctxt);
   stat = rtJsonDecMatchChar (pctxt, '"');
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   for (;;) {
      stat = rtxReadBytes (pctxt, &c, 1);
      if (stat == RTERR_ENDOFBUF) break;
      else if (0 != stat) return LOG_RTERR (pctxt, stat);

      if (c == ',' || c == '}') {
         /* Push character back to be reread again */
         OSRTASSERT (pctxt->buffer.byteIndex > 0);
         pctxt->buffer.byteIndex--;
         break;
      }

      /* Consume characters until ending double-quote character (") is found */
      if (c == '"') break;

      if (c >= '0' && c <= '9')
         c -= '0';
      else if (c >= 'a' && c <= 'f')
         c -= 'a' - 10;
      else if (c >= 'A' && c <= 'F')
         c -= 'A' - 10;
      else {
         stat = LOG_RTERRNEW (pctxt, RTERR_INVHEXS);
         break;
      }

      if (pvalue) {
         if ((oldsize & 1) == 0)
            pvalue[oldsize >> 1] = (OSOCTET) (c << 4);
         else
            pvalue[oldsize >> 1] |= c;
      }

      if (stat < 0)
         return LOG_RTERR (pctxt, stat);

      oldsize++;

   }

   if (stat >= 0) {
      stat = 0;

      if (pvalue) {
         if ((oldsize & 1) != 0) {/* shift hex str (211 -> 0211) */
            OSUTF8CHAR* p = pvalue + oldsize / 2;

            for (;p != pvalue; p--) {
               *p = (OSOCTET) ((p[-1] << 4) | (*p >> 4));
            }

            *p >>= 4;
         }
      }

      if (pnocts)
      {
         *pnocts = (oldsize + 1) / 2;
      }
   }

   return stat;
}
