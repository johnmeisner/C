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

int xerDecHexStrValue (OSCTXT* pctxt, OSOCTET* pvalue,
                       OSUINT32* pnbits, size_t bufsize,
                       OSUINT32 nsemiocts, OSBOOL skipWhitespaces)
{
   OSSIZE nbits64;
   int stat = 0;

   stat = xerDecHexStrValue64(pctxt, pvalue, &nbits64, bufsize, nsemiocts, skipWhitespaces);
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

int xerDecHexStrValue64 (OSCTXT* pctxt, OSOCTET* pvalue,
                         OSSIZE* pnbits, OSSIZE bufsize,
                         OSSIZE nsemiocts, OSBOOL skipWhitespaces)
{
   XMLCHAR* inpdata = (XMLCHAR*) ASN1BUFPTR (pctxt);
   OSUINT32 i;
   size_t di = 0;
   OSSIZE nbits = 0;
   OSOCTET hbyte;

   *pvalue = 0;

   if (nsemiocts == 0) {
      /* Count semi-octets */
      for (i = 0; i < pctxt->buffer.size && inpdata[i] != 0; i++) {
         char c = (char)inpdata[i];
         if (!OS_ISSPACE (c)) {
            if (!OS_ISXDIGIT (c))
               return LOG_RTERR (pctxt, RTERR_INVHEXS);
            nsemiocts++;
         }
         else if (!skipWhitespaces)
            return LOG_RTERR (pctxt, RTERR_INVHEXS);
      }
   }

   /* Convert Unicode characters to a bit string value */

   for (i = 0; i < pctxt->buffer.size && inpdata[i] != 0; i++) {
      char c = (char)inpdata[i];
      if (!OS_ISSPACE (c)) {
         if (di >= bufsize) {
            return LOG_RTERR (pctxt, RTERR_STROVFLW);
         }
         if (c >= '0' && c <= '9')
            hbyte = c - '0';
         else if (c >= 'A' && c <= 'F')
            hbyte = c - 'A' + 10;
         else if (c >= 'a' && c <= 'f')
            hbyte = c - 'a' + 10;
         else
            return LOG_RTERR (pctxt, RTERR_INVHEXS);

         if (nsemiocts % 2 == 0)
            pvalue [di] = (hbyte << 4);
         else
            pvalue [di ++] |= (hbyte & 0xFF);
         nbits += 4;
         nsemiocts--;
      }
      else if (!skipWhitespaces)
         return LOG_RTERR (pctxt, RTERR_INVHEXS);
   }
   if (pnbits != NULL) *pnbits = nbits;

   return (0);
}

