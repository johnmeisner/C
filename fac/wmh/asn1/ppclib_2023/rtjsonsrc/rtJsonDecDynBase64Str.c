/*
 * Copyright (c) 2019-2023 Objective Systems, Inc.
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
#include "rtxsrc/rtxBase64.h"
#include "rtxsrc/rtxCtype.h"
#include "rtjsonsrc/osrtjson.h"

#ifndef JSON_DECODE_SEGSIZE
   #define JSON_DECODE_SEGSIZE 1
#endif

int rtJsonDecDynBase64Str (OSCTXT* pctxt, OSDynOctStr* pvalue)
{
   int ret = 0;

   if (pvalue)
   {
      OSDynOctStr64 value64;
      value64.data = (OSOCTET*) pvalue->data; /* cast away const */
      ret = rtJsonDecDynBase64Str64(pctxt, &value64);
      if ( ret == 0 ) {
         if ( value64.numocts > OSUINT32_MAX )
            return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);

         pvalue->data = value64.data;
         pvalue->numocts = (OSUINT32) value64.numocts;
         return 0;
      }
      else return LOG_RTERR(pctxt, ret);
   }
   else return rtJsonDecDynBase64Str64(pctxt, 0);
}


int rtJsonDecDynBase64Str64 (OSCTXT* pctxt, OSDynOctStr64* pvalue)
{
   OSOCTET data;
   int stat = 0;
   OSUINT32 nocts = 0;
   OSRTMEMBUF membuf;
   size_t srcPos = 0;
   OSBOOL pad = FALSE;
   int prevVal = 0; /* remove warning C4701 */
   OSOCTET c;

   if (pvalue) {
      pvalue->numocts = 0;
      pvalue->data = 0;
   }

   rtJsonDecSkipWhitespace (pctxt);
   stat = rtJsonDecMatchChar (pctxt, '"');
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   rtxMemBufInit (pctxt, &membuf, 128);

   /* Consume characters until ending double-quote character (") is found */
   for (;;) {
      int val;
      stat = rtxReadBytes (pctxt, &c, 1);
      if (0 != stat) return LOG_RTERR (pctxt, stat);

      if (c == ' ') continue;
      if (c == ',' || c == '}') {
         pctxt->buffer.byteIndex--;
         break;
      }

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

      val = rtxBase64CharToIdx (c, FALSE);

      if (val < 0) {
         stat = LOG_RTERRNEW (pctxt, RTERR_INVBASE64);
         break;
      }

      switch (srcPos & 3) {
         case 0:
            data = (OSOCTET)(val << 2);
            stat = rtxMemBufAppend (&membuf, &data, 1);
            if (0 != stat) return LOG_RTERR (pctxt, stat);
            break;
         case 1:
            rtxMemBufCut (&membuf, membuf.usedcnt-1, 1);
            data |= (OSOCTET)(val >> 4);
            stat = rtxMemBufAppend (&membuf, &data, 1);
            if (0 != stat) return LOG_RTERR (pctxt, stat);
            nocts++;
            break;
         case 2:
            data = (OSOCTET)((prevVal << 4) | (val >> 2));
            stat = rtxMemBufAppend (&membuf, &data, 1);
            if (0 != stat) return LOG_RTERR (pctxt, stat);
            nocts++;
            break;
         case 3:
            data = (OSOCTET)((prevVal << 6) | val);
            stat = rtxMemBufAppend (&membuf, &data, 1);
            if (0 != stat) return LOG_RTERR (pctxt, stat);
            nocts++;
            break;
      }

      srcPos++;
      prevVal = val;
   }

   if (stat >= 0 && (srcPos % 4) != 0 &&
       c != ',' && c != '}') {
      stat = LOG_RTERRNEW (pctxt, RTERR_INVBASE64);
   }

   if (stat >= 0) {
      stat = 0;

      if (pvalue) {
         pvalue->numocts = nocts;
         pvalue->data = membuf.buffer;
      }
      else
         rtxMemBufFree (&membuf);
   }
   else
      rtxMemBufFree (&membuf);

   return stat;
}


