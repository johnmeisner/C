/*
 * Copyright (c) 1997-2018 Objective Systems, Inc.
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

#include "xed_common.hh"

int xerDecBitStrMemBuf (OSRTMEMBUF* pMemBuf,
                        const XMLCHAR* inpdata,
                        int length,
                        OSBOOL skipWhitespaces)
{
   int      i, stat, di, mask;
   OSOCTET* pdest;
   OSUINT32 nbits = 0;

   /* Count bits */
   for (i = 0; i < length; i++) {
      char c = (char)inpdata[i];
      if (!OS_ISSPACE (c)) {
         if (c != '0' && c != '1')
            return LOG_RTERR (pMemBuf->pctxt, ASN_E_INVBINS);
         nbits++;
      }
      else if (!skipWhitespaces)
         return LOG_RTERR (pMemBuf->pctxt, ASN_E_INVBINS);
   }

   stat = rtxMemBufPreAllocate (pMemBuf, (nbits + 7) / 8);
   if (stat != 0) return LOG_RTERR (pMemBuf->pctxt, stat);

   pdest = OSMEMBUFENDPTR (pMemBuf);
   nbits = (OSUINT32) pMemBuf->bitOffset;
   if (nbits != 0) {
      pdest--;
      pMemBuf->usedcnt--;
   }
   mask = (0x80 >> nbits);

   /* Convert Unicode characters to a bit string value */

   for (i = di = 0; i < length; i++ ) {
      char c = (char)inpdata[i];
      if (!OS_ISSPACE (c)) {
         if (nbits % 8 == 0)
            pdest [di] = 0;
         if (c == '1')
            pdest [di] |= mask;
         else if (c != '0')
            return LOG_RTERR (pMemBuf->pctxt, ASN_E_INVBINS);
         nbits ++;
         mask >>= 1;
         if (mask == 0) {
            mask = 0x80;
            di ++;
         }
      }
      else if (!skipWhitespaces)
         return LOG_RTERR (pMemBuf->pctxt, ASN_E_INVBINS);
   }

   pMemBuf->bitOffset = (nbits % 8);
   pMemBuf->usedcnt += (nbits + 7) / 8;

   return (0);
}

