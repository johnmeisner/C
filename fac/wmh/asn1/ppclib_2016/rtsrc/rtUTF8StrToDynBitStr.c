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

#include "rtsrc/asn1type.h"

EXTRTMETHOD int rtUTF8StrnToASN1DynBitStr
(OSCTXT* pctxt, const OSUTF8CHAR* utf8str, size_t nbytes,
 ASN1DynBitStr* pvalue)
{
   OSOCTET* data = 0;
   OSUINT32 bitIndex = 0;
   int stat = 0;
   size_t i;

   if (!pvalue)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   pvalue->numbits = 0;
   pvalue->data = 0;

   data = (OSOCTET*) rtxMemAlloc (pctxt, (nbytes + 7) / 8);
   if (data == 0)
      return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

   for (i = 0; i < nbytes; i++) {
      OSUTF8CHAR c = utf8str[i];

      if (bitIndex % 8 == 0)
         data[bitIndex / 8] = 0;

      if (c == '1')
         rtxSetBit (data, (OSUINT32) nbytes, bitIndex);
      else if (c != '0') {
         stat = LOG_RTERRNEW (pctxt, RTERR_INVCHAR);
         break;
      }

      bitIndex++;
   }

   if (stat >= 0) {
      stat = 0;
      pvalue->numbits = bitIndex;
      pvalue->data = data;
      }
   else
      rtxMemFreePtr (pctxt, data);

   return stat;
}

EXTRTMETHOD int rtUTF8StrToASN1DynBitStr
   (OSCTXT* pctxt, const OSUTF8CHAR* utf8str, ASN1DynBitStr* pvalue)
{
   return rtUTF8StrnToASN1DynBitStr (pctxt, utf8str, rtxUTF8LenBytes(utf8str),
      pvalue);
}
