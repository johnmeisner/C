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

EXTRTMETHOD int rtxUTF8StrnToDynHexStr (OSCTXT* pctxt, const OSUTF8CHAR* utf8str,
   size_t nbytes, OSDynOctStr* pvalue)
{
   OSOCTET* data = 0;
   int stat = 0;
   size_t i;

   if (!pvalue)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   pvalue->numocts = 0;
   pvalue->data = 0;

   data = (OSOCTET*) rtxMemAlloc (pctxt, (nbytes + 1) / 2);
   if (data == 0)
      return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

   for (i = 0; i < nbytes; i++) {
      OSUTF8CHAR c = utf8str[i];
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

      if ((i & 1) == 0)
         data[i >> 1] = (OSOCTET)(c << 4);
      else
         data[i >> 1] |= c;
   }

   if (stat == 0) {
      if ((nbytes & 1) != 0) {/* shift hex str (211 -> 0211) */
         OSUTF8CHAR* p = data + nbytes / 2;

         for (;p != data; p--) {
            *p = (OSUTF8CHAR)((p[-1] << 4) | (*p >> 4));
         }

         *p >>= 4;
      }

      pvalue->data = data;
      pvalue->numocts = (OSUINT32) ((nbytes + 1) / 2);
   }
   else
      rtxMemFreePtr (pctxt, data);

   return stat;
}

EXTRTMETHOD int rtxUTF8StrToDynHexStr
   (OSCTXT* pctxt, const OSUTF8CHAR* utf8str, OSDynOctStr* pvalue)
{
   return rtxUTF8StrnToDynHexStr (pctxt, utf8str, rtxUTF8LenBytes(utf8str),
      pvalue);
}
