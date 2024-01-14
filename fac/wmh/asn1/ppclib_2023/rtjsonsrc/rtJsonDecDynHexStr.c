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

/**
 * Decode hexadecimal characters into a memory buffer.  Characters are read
 * until a non-hexadecimal character is found.
 *
 * @param pctxt
 * @param pmembuf Pre-initialized memory buffer to receive decoded data.
 *          If NULL, the hexadecimal data is read but not retained.
 */
int decDynHexData (OSCTXT* pctxt, OSRTMEMBUF* pmembuf)
{
   OSOCTET data = 0;
   int stat = 0;
   OSUINT32 nchars = 0;
   OSUTF8CHAR c;

   for (;;) {
      stat = rtxReadBytes (pctxt, &c, 1);
      if (stat == RTERR_ENDOFBUF) break;
      else if (0 != stat) return LOG_RTERR (pctxt, stat);

      /* Consume characters until non hex char is found */

      if (c >= '0' && c <= '9')
         c -= '0';
      else if (c >= 'a' && c <= 'f')
         c -= 'a' - 10;
      else if (c >= 'A' && c <= 'F')
         c -= 'A' - 10;
      else {
         /* Push character back to be reread again */
         OSRTASSERT (pctxt->buffer.byteIndex > 0);
         pctxt->buffer.byteIndex--;
         break;
      }

      if (pmembuf) {
         if ((nchars & 1) == 0)
            data = (OSOCTET) (c << 4);
         else {
            data |= c;
            stat = rtxMemBufAppend (pmembuf, &data, 1);
            if (0 != stat) return LOG_RTERR (pctxt, stat);
         }
      }

      if (stat < 0)
         return LOG_RTERR (pctxt, stat);

      nchars++;
   }

   if (stat >= 0) {
      stat = 0;

      if (pmembuf) {
         if ((nchars & 1) != 0) {/* shift hex str (211 -> 0211) */
            OSOCTET* pdata = pmembuf->buffer;
            OSUTF8CHAR* p = pdata + nchars / 2;

            for (;p != pdata; p--) {
               *p = (OSOCTET) ((p[-1] << 4) | (*p >> 4));
            }

            *p >>= 4;
         }
      }
   }

   return stat;
}


int rtJsonDecDynHexData64 (OSCTXT* pctxt, OSDynOctStr64* pvalue)
{
   int stat = 0;
   OSRTMEMBUF membuf;

   if (pvalue) {
      pvalue->numocts = 0;
      pvalue->data = 0;
      rtxMemBufInit (pctxt, &membuf, 128);
   }

   stat = decDynHexData(pctxt, pvalue ? &membuf : 0);

   if (stat == 0) {
      if (pvalue) {
         pvalue->numocts = membuf.usedcnt;
         pvalue->data = membuf.buffer;
      }
   }
   else if ( pvalue ) rtxMemBufFree (&membuf);

   return stat;
}

int rtJsonDecHexToCharStr (OSCTXT* pctxt, OSUTF8CHAR** ppvalue)
{
   int stat = 0;
   OSRTMEMBUF membuf;

   rtJsonDecSkipWhitespace (pctxt);
   stat = rtJsonDecMatchChar (pctxt, '"');
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   if (ppvalue) {
      rtxMemBufInit (pctxt, &membuf, 128);
   }

   stat = decDynHexData(pctxt, ppvalue ? &membuf : 0);

   if (stat == 0) {
      if (ppvalue) {
         /* add null terminator to decoded data */
         OSOCTET zero = 0;
         stat = rtxMemBufAppend (&membuf, &zero, 1);
         if (0 != stat) return LOG_RTERR (pctxt, stat);

         *ppvalue = (OSUTF8CHAR*) membuf.buffer;
      }
   }
   else if ( ppvalue ) rtxMemBufFree (&membuf);

   stat = rtJsonDecMatchChar (pctxt, '"');
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   return stat;
}


int rtJsonDecDynHexStr64 (OSCTXT* pctxt, OSDynOctStr64* pvalue)
{
   int stat = 0;

   rtJsonDecSkipWhitespace (pctxt);
   stat = rtJsonDecMatchChar (pctxt, '"');
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   stat = rtJsonDecDynHexData64(pctxt, pvalue);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   stat = rtJsonDecMatchChar (pctxt, '"');
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   return stat;
}

