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

/* Binary-coded decimal utility functions */

#include "rtsrc/rtBCD.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxCtype.h"

/* Returns number of octets are occupied by BCD string in the bcdStr. */
EXTRTMETHOD int rtStringToBCD
(const char* str, OSOCTET* bcdStr, OSSIZE bufsiz, OSBOOL isTBCD)
{
   OSOCTET         elem = 0, sym;
   register OSSIZE i, maxDigits = bufsiz * 2;
   int             isDigit;
   const char*     pstr = str;
   OSOCTET         *pbcdStr = bcdStr;

   if (isTBCD) {
      return rtStringToTBCD (str, bcdStr, bufsiz);
   }

   if (str == NULL || bcdStr == NULL || 0 == bufsiz)
      return RTERR_INVPARAM;

   OSCRTLMEMSET (bcdStr, 0xFF, bufsiz);

   for (i = 0; *pstr != 0 && i < maxDigits; pstr++, i++) {
      sym = (OSOCTET) OS_TOUPPER (*pstr);
      isDigit = OS_ISDIGIT(sym);
      if (!isDigit && (sym < 'A' || sym > 'F'))
         break;

      if (i % 2 == 1) {  /* non-parity */
         elem |= (OSOCTET) (isDigit ? sym - '0' : sym - 'A' + 10);
         *pbcdStr++ = elem;
      }
      else {
         elem = (((OSOCTET) (isDigit ? sym - '0' : sym - 'A' + 10)) << 4);
      }
   }
   if (i & 1) { /* length is non-parity - write last octet */
      *pbcdStr = (OSOCTET) (elem | 0x0F);
   }

   return (int)((i + 1) / 2);
}

EXTRTMETHOD int rtStringToDynBCD
(OSCTXT* pctxt, const char* str, ASN1DynOctStr* poctstr)
{
   int len;
   OSOCTET* pdata;

   if (0 == poctstr) return RTERR_INVPARAM;
   poctstr->numocts = (OSUINT32)((OSCRTLSTRLEN(str)/2) + 1);

   pdata = (OSOCTET*) rtxMemAlloc (pctxt, poctstr->numocts);
   if (0 == pdata) return RTERR_NOMEM;

   len = rtStringToBCD (str, pdata, poctstr->numocts, FALSE);
   if (len > 0) poctstr->numocts = len;

   poctstr->data = pdata;

   return len;
}

EXTRTMETHOD const char* rtBCDToString (OSUINT32 numocts,
   const OSOCTET* data, char* buffer, OSSIZE bufsiz, OSBOOL isTBCD)
{
   /* BCD digits are : 0,1,2,3,4,5,6,7,8,9 */
   /* Filler digits are used for odd no. of digits:  A,B,C,D,E,F */
   OSSIZE dstByteIndex = 0, srcByteIndex = 0, maxDigits = numocts * 2;
   register OSUINT32 off;

   if (isTBCD) {
      return rtTBCDToString (numocts, data, buffer, bufsiz);
   }

   if (!data || !buffer || !numocts || !bufsiz) return NULL;
   for (off = 0; dstByteIndex < bufsiz && off < maxDigits; off++) {
      OSUINT32 b;
      if (off % 2 == 0) {
         b = data[srcByteIndex] >> 4;
      }
      else {
         b = data[srcByteIndex++] & 0xF;
      }
      if (b >= 0xA) /* filler? end of BCD. */
         break;
      buffer[dstByteIndex++] = (char) ((b < 0xA) ? b + '0': b + 'A' - 10);
   }

   if (dstByteIndex < bufsiz)
      buffer[dstByteIndex] = 0;

   return buffer;
}

/* Returns number of octets are occupied by BCD string in the bcdStr. */
EXTRTMETHOD int rtStringToTBCD
(const char* str, OSOCTET* bcdStr, OSSIZE bufsiz)
{
   OSOCTET         elem = 0, nibble;
   register OSSIZE i, maxDigits = bufsiz * 2;
   const char*     pstr = str;
   OSOCTET         *pbcdStr = bcdStr;

   if (str == NULL || bcdStr == NULL || 0 == bufsiz)
      return RTERR_INVPARAM;

   OSCRTLMEMSET (bcdStr, 0xFF, bufsiz);

   /* Modified to conform to ITU-T Q.825 (see rtTBCDToString) */

   for (i = 0; *pstr != 0 && i < maxDigits; pstr++, i++) {
      if (0 != rtTBCDCharToBin (*pstr, &nibble)) break;

      if (i % 2 == 1) {  /* non-parity */
         elem |= (((OSOCTET) nibble) << 4);
         *pbcdStr++ = elem;
      }
      else {
         elem = nibble;
      }
   }
   if (i & 1) { /* length is non-parity - write last octet */
      *pbcdStr = (OSOCTET) (elem | 0xF0);
   }

   return (int)((i + 1) / 2);
}

EXTRTMETHOD const char* rtTBCDToString (OSUINT32 numocts, const OSOCTET* data,
                                        char* buffer, OSSIZE bufsiz)
{
   OSSIZE dstByteIndex = 0, srcByteIndex = 0, maxDigits = numocts * 2;
   register OSUINT32 off;

   if (!data || !buffer || !numocts || !bufsiz) return NULL;
   for (off = 0; dstByteIndex < bufsiz && off < maxDigits; off++) {
      OSUINT8 b;
      if (off % 2 == 0) {
         b = data[srcByteIndex] & 0xF;
      }
      else {
         b = data[srcByteIndex++] >> 4;
      }

      if (b < 0xA) {
         buffer[dstByteIndex++] = (char)(b + '0');
      } else if (b == 0xA) {
         buffer[dstByteIndex++] = '*';
      } else if (b == 0xB) {
         buffer[dstByteIndex++] = '#';
      } else if (b == 0xC) {
         buffer[dstByteIndex++] = 'a';
      } else if (b == 0xD) {
         buffer[dstByteIndex++] = 'b';
      } else if (b == 0xE) {
         buffer[dstByteIndex++] = 'c';
      }
   }

   if (dstByteIndex < bufsiz)
      buffer[dstByteIndex] = 0;

   return buffer;
}
