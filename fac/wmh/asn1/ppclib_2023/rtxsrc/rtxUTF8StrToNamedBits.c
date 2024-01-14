/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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

#include <string.h>
#include "rtxsrc/rtxBitString.h"
#include "rtxsrc/rtxMemBuf.h"
#include "rtxsrc/rtxUTF8.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxDiag.h"

#define BUFF_SZ 32

static OSUINT32 FindBitMapItem (const OSBitMapItem* pBitMap,
   const OSOCTET* str, size_t strSz)
{
   OSUINT32 i = 0;
   for ( ; pBitMap[i].name != 0; i++) {
      if (pBitMap[i].namelen == strSz &&
            memcmp (pBitMap[i].name, str, strSz) == 0)
         return pBitMap[i].bitno;
   }

   return ~0u;
}

EXTRTMETHOD int rtxUTF8StrToNamedBits (OSCTXT* pctxt, const OSUTF8CHAR* utf8str,
   const OSBitMapItem* pBitMap, OSOCTET* pvalue, OSUINT32* pnbits,
   OSUINT32 bufsize)
{
   OSUINT32 numbits = bufsize * 8;
   size_t nbytes = rtxUTF8LenBytes(utf8str);
   int stat = 0;
   size_t i;
   OSUINT32 maxbit = 0;

   OSOCTET buffer[BUFF_SZ];
   OSRTMEMBUF memBuf;
   size_t buffPos = 0;
   size_t buffSz = BUFF_SZ;

   if (!pvalue || !pnbits || !pBitMap)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   rtxMemBufInitBuffer (pctxt, &memBuf, buffer, BUFF_SZ, 1);

   *pnbits = 0;

   if (bufsize)
      OSCRTLMEMSET (pvalue, 0, bufsize);

   for (i = 0; i < nbytes; i++) {
      OSOCTET c = (OSOCTET) utf8str[i];

      if (c == ' ') {
         OSUINT32 bitno = FindBitMapItem (pBitMap,
            OSMEMBUFPTR(&memBuf), buffPos);

         if (bitno == ~0u)
            RTDIAG3 (pctxt,
               "rtXmlpDecNamedBits: BitMapItem = '%*s' not found\n",
               OSMEMBUFPTR(&memBuf), buffPos);
         else {
            rtxSetBit (pvalue, numbits, bitno);
            if (bitno > maxbit)
               maxbit = bitno;
         }

         rtxMemBufReset (&memBuf);
         buffPos = 0;
      }
      else {
         if (buffPos == buffSz) { /* enlarge buffer */
            buffSz *= 2;
            stat = rtxMemBufPreAllocate (&memBuf, buffSz);
            /* memBuf.usedcnt always 0; memRealloc save buffer content*/
            if (stat < 0)
               LOG_RTERR (pctxt, stat);
         }

      OSMEMBUFPTR(&memBuf)[buffPos++] = c;
      }
   }

   if (stat >= 0 && buffPos > 0) {
      OSUINT32 bitno = FindBitMapItem (pBitMap, OSMEMBUFPTR(&memBuf), buffPos);

      if (bitno == ~0u) {
         RTDIAG3 (pctxt,
            "rtxUTF8StrToNamedBits: BitMapItem = '%*s' not found\n",
            OSMEMBUFPTR(&memBuf), buffPos);
      }
      else {
         rtxSetBit (pvalue, numbits, bitno);
         if (bitno > maxbit)
            maxbit = bitno;
      }
   }

   if (stat > 0) {
      stat = 0;
      *pnbits = maxbit + 1;
   }

   rtxMemBufFree (&memBuf);

   return stat;
}

