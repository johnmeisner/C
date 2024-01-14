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

#include "rtxmlsrc/osrtxml.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxErrCodes.h"

EXTXMLMETHOD int rtXmlDecHexStrValue (OSCTXT* pctxt,
                         const OSUTF8CHAR* const inpdata, size_t nbytes,
                         OSOCTET* pvalue,
                         OSUINT32* pnbits, OSINT32 bufsize)
{
   if ( pnbits )
   {
      int ret;
      OSSIZE nbits;

      ret = rtXmlDecHexStrValue64(pctxt, inpdata, nbytes, pvalue, &nbits,
                                    bufsize);
      if ( ret != 0 ) return LOG_RTERR(pctxt, ret);

      if ( nbits > OSUINT32_MAX)
         return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);

      *pnbits = (OSUINT32) nbits;

      return 0;
   }
   else return rtXmlDecHexStrValue64(pctxt, inpdata, nbytes, pvalue, 0,
                                    bufsize);
}


EXTXMLMETHOD int rtXmlDecHexStrValue64 (OSCTXT* pctxt,
                         const OSUTF8CHAR* const inpdata, size_t nbytes,
                         OSOCTET* pvalue,
                         OSSIZE* pnbits, OSSIZE bufsize)
{
   size_t i;
   OSSIZE di = 0;
   OSSIZE nbits = 0;
   OSOCTET hbyte;

   *pvalue = 0;
   if (0 == inpdata) return 0;

   /* Handle case of string having odd number of bytes (ED, 10/25/03) */

   i = 0;
   if ((nbytes % 2) != 0) {
      HEXCHARTONIBBLE (inpdata[0], pvalue[di]);
      i++; di++;
      nbits += 8; /* !AB, 03/22/06, fix the problem with odd number of
                     bytes, i.e. "211" was decoded as 02 01, should be 02 11*/
   }

   /* Convert characters to a bit string value */

   for ( ; i < nbytes; i++) {
      char c = (char)inpdata[i];
      if (!OS_ISSPACE (c)) {
         if (di >= bufsize) {
            return LOG_RTERRNEW (pctxt, RTERR_STROVFLW);
         }
         if (c >= '0' && c <= '9')
            hbyte = (OSOCTET)(c - '0');
         else if (c >= 'A' && c <= 'F')
            hbyte = (OSOCTET)(c - 'A' + 10);
         else if (c >= 'a' && c <= 'f')
            hbyte = (OSOCTET)(c - 'a' + 10);
         else
            return LOG_RTERRNEW (pctxt, RTERR_INVHEXS);

         if (nbits % 8 == 0)
            pvalue [di] = (OSOCTET)(hbyte << 4);
         else
            pvalue [di ++] |= (hbyte & 0xF);

         nbits += 4;
      }
      else
         return LOG_RTERRNEW (pctxt, RTERR_INVHEXS);
   }
   if (pnbits != NULL) *pnbits = nbits;

   return (0);
}


