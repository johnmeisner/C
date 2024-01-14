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

#include "rtxsrc/rtxCtype.h"
#include "rtxmlsrc/osrtxml.hh"

EXTXMLMETHOD int rtXmlDecBase64Binary
(OSRTMEMBUF* pMemBuf, const OSUTF8CHAR* inpdata, OSSIZE length)
{
   int stat, di = 0, lshift, rshift;
   OSSIZE si, j, numocts;
   int ch0, ch1, ich = '=', prevCh;
   OSOCTET* pdest;

   stat = rtXmlGetBase64StrDecodedLen (inpdata, length, &numocts, 0);
   if (stat < 0)
      return LOG_RTERRNEW (pMemBuf->pctxt, stat);

   if (numocts >= (OSSIZE_MAX - 8))
      return LOG_RTERRNEW (pMemBuf->pctxt, RTERR_TOOBIG);
   else
      numocts += 8;

   stat = rtxMemBufPreAllocate (pMemBuf, numocts);
   if (stat != 0) return LOG_RTERR (pMemBuf->pctxt, stat);

   pdest = OSMEMBUFENDPTR (pMemBuf);
   if (pMemBuf->bitOffset != 0) {

      /* restore parameters necessary to continue decoding */

      if (pMemBuf->bitOffset & 1) {
         pdest--;
         pMemBuf->usedcnt--;
         prevCh = *pdest;
      }
      else
         prevCh = 0;
      lshift = ((pMemBuf->bitOffset >> 1) & 7) * 2;
      rshift = ((pMemBuf->bitOffset >> 4) & 7) * 2;
   }
   else {

      /* set default values to start decoding */

      lshift = 2; rshift = 4;
      prevCh = 0;
   }

   /* process byte-by-byte decoding */

   for (si = 0, j = pMemBuf->bitOffset; si < length; ) {
      ich = inpdata[si];
      if (ich == '=')
         break;
      if (OS_ISSPACE (ich)) {
         si++;
         continue;
      }
      ch0 = BASE64TOINT (ich);
      if (ch0 < 0) {
         return LOG_RTERRNEW (pMemBuf->pctxt, RTERR_INVBASE64);
      }

      if ((j & 1) == 0) {
         ch1 = prevCh | (ch0 << lshift);
         prevCh = ch1;
         lshift += 2;
         si++;
      }
      else {
         ch1 = prevCh | (ch0 >> rshift);
         prevCh = 0;
         rshift -= 2;
         pdest[di++] = (OSOCTET)ch1;
      }
      if (rshift < 0) {
         lshift = 2; rshift = 4;
         si++;
      }
      j++;
   }
   if (ich != '=') {
      /* save parameters necessary to continue decoding */
      if ((j & 1) != 0) {
         if (prevCh >= 0 && prevCh <= 255)
            pdest[di++] = (OSOCTET) prevCh;
         else
            return LOG_RTERRNEW (pMemBuf->pctxt, RTERR_INVBASE64);
      }
      pMemBuf->bitOffset = (j & 1) | (lshift/2 << 1) | (rshift/2 << 4);
   }
   pMemBuf->usedcnt += di;

   return (0);
}
