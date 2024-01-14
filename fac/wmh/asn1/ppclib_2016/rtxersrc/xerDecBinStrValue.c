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

int xerDecBinStrValue (OSCTXT* pctxt, OSOCTET* pvalue,
                       OSUINT32* pnbits, size_t bufsize,
                       OSBOOL skipWhitespaces)
{
   OSSIZE nbits64;
   int stat = 0;

   stat = xerDecBinStrValue64(pctxt, pvalue, &nbits64, bufsize,
                              skipWhitespaces);
   if (stat >= 0)
   {
      if ((sizeof(nbits64) > 4) && (nbits64 > OSUINT32_MAX))
      {
         return LOG_RTERR(pctxt, RTERR_TOOBIG);
      }
      else
      {
         if (pnbits)
         {
            *pnbits = (OSUINT32)nbits64;
         }
      }
   }

   return stat;
}

int xerDecBinStrValue64 (OSCTXT* pctxt, OSOCTET* pvalue,
                         OSSIZE* pnbits, OSSIZE bufsize,
                         OSBOOL skipWhitespaces)
{
   XMLCHAR* inpdata = (XMLCHAR*) ASN1BUFPTR (pctxt);
   size_t i, di = 0;
   int mask = 0x80;
   OSSIZE nbits = 0;

   /* Convert Unicode characters to a bit string value */

   for (i = 0; inpdata[i] != 0; i++ ) {
      char c = (char)inpdata[i];
      if (!OS_ISSPACE (c)) {
         if (di >= bufsize) {
            return LOG_RTERR (pctxt, RTERR_STROVFLW);
         }
         if (nbits % 8 == 0)
            pvalue [di] = 0;
         if (c == '1')
            pvalue [di] |= mask;
         else if (c != '0')
            return LOG_RTERR (pctxt, ASN_E_INVBINS);
         nbits ++;
         mask >>= 1;
         if (mask == 0) {
            mask = 0x80;
            di ++;
         }
      }
      else if (!skipWhitespaces)
         return LOG_RTERR (pctxt, ASN_E_INVBINS);
   }
   if (pnbits != NULL) *pnbits = nbits;

   return (0);
}

