/*
 * Copyright (c) 2018-2018 Objective Systems, Inc.
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

#include "rtxsrc/rtxMemBuf.h"
#include "rtjsonsrc/osrtjson.h"

#ifndef XMLP_DECODE_SEGSIZE
   #define XMLP_DECODE_SEGSIZE 1
#endif

int rtJsonDecDynHexStr (OSCTXT* pctxt, OSDynOctStr* pvalue)
{
   if ( pvalue )
   {
      int ret;
      OSDynOctStr64 value64;
      value64.data = (OSOCTET*) pvalue->data; /* cast away const */
      value64.numocts = pvalue->numocts;

      ret = rtJsonDecDynHexStr64(pctxt, &value64);
      if ( ret != 0 ) return LOG_RTERR(pctxt, ret);

      if ( value64.numocts > OSUINT32_MAX )
         return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);

      pvalue->data = value64.data;
      pvalue->numocts = (OSUINT32) value64.numocts;

      return 0;
   }
   else return rtJsonDecDynHexStr64(pctxt, 0);
}


int rtJsonDecDynHexStr64 (OSCTXT* pctxt, OSDynOctStr64* pvalue)
{
   OSOCTET data = 0;
   int stat = 0;
   OSUINT32 nchars = 0;
   OSRTMEMBUF membuf;
   OSUTF8CHAR c;

   if (pvalue) {
      pvalue->numocts = 0;
      pvalue->data = 0;
   }

   rtJsonDecSkipWhitespace (pctxt);
   stat = rtJsonDecMatchChar (pctxt, '"');
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   rtxMemBufInit (pctxt, &membuf, 128);

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
         if ((nchars & 1) == 0)
            data = (OSOCTET) (c << 4);
         else {
            data |= c;
            stat = rtxMemBufAppend (&membuf, &data, 1);
            if (0 != stat) return LOG_RTERR (pctxt, stat);
         }
      }

      if (stat < 0)
         return LOG_RTERR (pctxt, stat);

      nchars++;
   }

   if (stat >= 0) {
      stat = 0;

      if (pvalue) {
         if ((nchars & 1) != 0) {/* shift hex str (211 -> 0211) */
            OSOCTET* pdata = membuf.buffer;
            OSUTF8CHAR* p = pdata + nchars / 2;

            for (;p != pdata; p--) {
               *p = (OSOCTET) ((p[-1] << 4) | (*p >> 4));
            }

            *p >>= 4;
         }

         pvalue->numocts = nchars / 2;
         pvalue->data = membuf.buffer;
      }
      else
         rtxMemBufFree (&membuf);
   }
   else
      rtxMemBufFree (&membuf);

   return stat;
}


