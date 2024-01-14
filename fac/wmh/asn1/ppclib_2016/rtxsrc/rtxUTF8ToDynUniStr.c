/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
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

#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxMemBuf.h"
#include "rtxsrc/rtxUTF8.h"
#include "rtxsrc/rtxError.h"

EXTRTMETHOD int rtxUTF8ToDynUniStr (OSCTXT* pctxt, const OSUTF8CHAR* utf8str,
   const OSUNICHAR** ppdata, OSUINT32* pnchars)
{
   OSUNICHAR* data = 0;
   long stat = 0;
   size_t nchars;

   if (!utf8str || !ppdata || !pnchars)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   *ppdata = 0;
   *pnchars = 0;

   nchars = rtxUTF8Len(utf8str);

   data = (OSUNICHAR*) rtxMemAlloc (pctxt, nchars  * sizeof (OSUNICHAR));
   if (data == 0)
      return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

   stat = rtxUTF8ToUnicode (pctxt, utf8str, data, nchars);
   if (stat < 0) {
      rtxMemFreePtr (pctxt, data);
      return LOG_RTERR (pctxt, (int) stat);
   }

   *ppdata = data;
   *pnchars = (OSUINT32) nchars;

   return 0;
}

EXTRTMETHOD int rtxUTF8ToDynUniStr32 (OSCTXT* pctxt, const OSUTF8CHAR* utf8str,
   const OS32BITCHAR** ppdata, OSUINT32* pnchars)
{
   OS32BITCHAR* data = 0;
   long stat = 0;
   size_t nchars;

   if (!utf8str || !ppdata || !pnchars)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   *ppdata = 0;
   *pnchars = 0;

   nchars = rtxUTF8Len(utf8str);

   data = (OS32BITCHAR*) rtxMemAlloc (pctxt, nchars  * sizeof (OS32BITCHAR));
   if (data == 0)
      return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

   stat = rtxUTF8ToUnicode32 (pctxt, utf8str, data, nchars);
   if (stat < 0) {
      rtxMemFreePtr (pctxt, data);
      return LOG_RTERR (pctxt, (int) stat);
   }

   *ppdata = data;
   *pnchars = (OSUINT32) nchars;

   return 0;
}

#if 0
#ifndef UTF8TODYNUNISTR_SEGSIZE
   #define UTF8TODYNUNISTR_SEGSIZE 32
#endif

EXTRTMETHOD int rtxUTF8ToDynUniStr1 (OSCTXT* pctxt, const OSUTF8CHAR* utf8str,
   const OSUNICHAR** ppdata, OSUINT32* pnchars)
{
   int stat = 0;
   size_t i, uniCharLen;
   OSUNICHAR curUniChar;
   int curCharOctet = 0;
   int curCharLen = -1;
   OSRTMEMBUF memBuf;

   if (!utf8str || !ppdata || !pnchars)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   *ppdata = 0;
   *pnchars = 0;

   rtxMemBufInit (pctxt, &memBuf, UTF8TODYNUNISTR_SEGSIZE);

   for (i = 0; *utf8str; i++, utf8str++) {
      OSUTF8CHAR c = *utf8str;

      if (curCharOctet == 0) { /* first octet */
         /* get char len */

         if (c < 0x80) {
            curCharLen = 1;
            curUniChar = (OSUNICHAR)(c & 0x7F);
         }
         else if (c < 0xE0) {
            curCharLen = 2;
            curUniChar = (OSUNICHAR)(c & 0x1F);
         }
         else if (c < 0xF0) {
            curCharLen = 3;
            curUniChar = (OSUNICHAR)(c & 0xF);
         }
         else if (c < 0xF8) {
            curCharLen = 4;
            curUniChar = (OSUNICHAR)(c & 0x7);
         }
         else if (c < 0xFC) {
            curCharLen = 5;
            curUniChar = (OSUNICHAR)(c & 0x3);
         }
         else if (c < 0xFE) {
            curCharLen = 6;
            curUniChar = (OSUNICHAR)(c & 0x1);
         }
         else { /* invalid utf8 character */
            rtxErrNewNode (pctxt);
            rtxErrAddUIntParm (pctxt, (OSUINT32)i);
            stat = LOG_RTERRNEW (pctxt, RTERR_INVUTF8);
            break;
         }
         uniCharLen = sizeof(OSUNICHAR);
         if (curCharLen > 3 && uniCharLen < 4) {
            /* character not fit to USC2 */
            stat = LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
            break;
         }
      }
      else if ((c >= 0x80) && (c < 0xC0)) { /* next octet */
         curUniChar = (OSUNICHAR)((curUniChar << 6) | (c & 0x3F));
      }
      else { /* invalid utf8 character */
         rtxErrNewNode (pctxt);
         rtxErrAddUIntParm (pctxt, (OSUINT32)i);
         stat = LOG_RTERRNEW (pctxt, RTERR_INVUTF8);
         break;
      }

      curCharOctet++;

      if (curCharOctet == curCharLen) {
         rtxMemBufAppend (&memBuf, (OSOCTET*) &curUniChar,
            sizeof (curUniChar));
         curCharOctet = 0;
      }
   }

   if (stat >= 0 && curCharOctet != 0) {
      stat = LOG_RTERRNEW (pctxt, RTERR_ENDOFBUF);
   }

   if (stat == 0) {
      /* shrink block */
      *ppdata = (OSUNICHAR*) rtxMemRealloc (pctxt, OSMEMBUFPTR(&memBuf),
         OSMEMBUFUSEDSIZE(&memBuf));
      *pnchars = (OSUINT32) (OSMEMBUFUSEDSIZE(&memBuf) / sizeof (OSUNICHAR));
   }
   else
      rtxMemBufFree (&memBuf);

   return stat;
}
#endif
