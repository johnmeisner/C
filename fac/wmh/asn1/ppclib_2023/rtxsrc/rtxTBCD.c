/*
 * Copyright (c) 1997-2023 Objective Systems, Inc.
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

/* Telephony binary-coded decimal utility functions */

#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxTBCD.h"

EXTRTMETHOD int rtxQ825TBCDToString
(OSSIZE numocts, const OSOCTET* data, char* buffer, OSSIZE bufsiz)
{
   OSSIZE dstByteIndex = 0, srcByteIndex = 0, maxDigits = numocts * 2;
   OSUINT32 off;
   int ret = 0;

   if (0 == data || 0 == buffer) return RTERR_NULLPTR;

   for (off = 0; dstByteIndex < bufsiz && off < maxDigits; off++) {
      OSUINT8 b;
      if (off % 2 == 0) {
         b = data[srcByteIndex] & 0xF;
      }
      else {
         b = data[srcByteIndex++] >> 4;
      }
      if (b == 0xF) /* filler? end of TBCD */
         break;

      /* This type (Telephony Binary Coded Decimal String) is used to
         represent digits from 0 through 9, *, #, a, b, c, two digits per
         octet, each digit encoded 0000 to 1001 (0 to 9),
         1010 (*) 1011(#), 1100 (a), 1101 (b) or 1110 (c); 1111 (end of
         pulsing signal-ST); 0000 is used as a filler when there is an odd
         number of digits. */

      ret = rtxTBCDBinToChar (b, &buffer[dstByteIndex]);
      if (0 != ret) return ret;

      dstByteIndex++;
   }

   if (dstByteIndex < bufsiz)
      buffer[dstByteIndex] = 0;
   else
      ret = RTERR_TOOBIG;

   return ret;
}

EXTRTMETHOD int rtxDecQ825TBCDString
(OSCTXT* pctxt, OSSIZE numocts, char* buffer, OSSIZE bufsiz)
{
   OSSIZE i, j;
   OSOCTET ub, b;
   int ret;

   if (bufsiz < (numocts * 2) + 1) {
      return LOG_RTERR (pctxt, RTERR_BUFOVFLW);
   }

   for (i = 0, j = 0; i < numocts; i++) {
      ret = rtxReadBytes (pctxt, &ub, 1);
      if (ret < 0) return LOG_RTERR (pctxt, ret);

      /* 1st digit is in lower nibble */
      b = ub & 0xF;
      ret = rtxTBCDBinToChar (b, &buffer[j++]);
      if (0 != ret) return LOG_RTERR (pctxt, ret);

      /* 2nd digit is in upper nibble */
      b = ub >> 4;
      ret = rtxTBCDBinToChar (b, &buffer[j++]);
      if (0 != ret) return LOG_RTERR (pctxt, ret);
   }

   buffer[j] = '\0';

   return 0;
}

EXTRTMETHOD int rtxEncQ825TBCDString (OSCTXT* pctxt, const char* str)
{
   OSOCTET  elem = 0, nibble;
   OSSIZE   i, slen;
   int      ret;

   if (OS_ISEMPTY(str)) return 0; /* nothing to encode */

   slen = OSCRTLSTRLEN (str);

   for (i = 0; i < slen; i++) {
      /* 1st character */
      ret = rtxTBCDCharToBin (str[i++], &elem);
      if (0 != ret) return LOG_RTERR (pctxt, ret);

      /* 2nd character */
      if (i < slen) {
         ret = rtxTBCDCharToBin (str[i], &nibble);
         if (0 != ret) return LOG_RTERR (pctxt, ret);

         elem |= (((OSOCTET) nibble) << 4);
      }
      else {
         elem |= (OSOCTET)0xF0;
      }

      /* Encode digit */
      ret = rtxWriteBytes (pctxt, &elem, 1);
      if (0 != ret) return LOG_RTERR (pctxt, ret);
   }

   return 0;
}

EXTRTMETHOD int rtxTBCDBinToChar (OSUINT8 bcdDigit, char* pdigit)
{
   switch (bcdDigit) {
   case 0x0A: *pdigit = '*'; break;
   case 0x0B: *pdigit = '#'; break;
   case 0x0C: *pdigit = 'a'; break;
   case 0x0D: *pdigit = 'b'; break;
   case 0x0E: *pdigit = 'c'; break;
   case 0x0F: *pdigit = '\0'; break;
   default: *pdigit = bcdDigit + '0';
   }
   return 0;
}

EXTRTMETHOD int rtxTBCDCharToBin (char digit, OSUINT8* pbyte)
{
   switch (digit) {
   case '*': *pbyte = 0x0A; break;
   case '#': *pbyte = 0x0B; break;
   case 'a': *pbyte = 0x0C; break;
   case 'b': *pbyte = 0x0D; break;
   case 'c': *pbyte = 0x0E; break;
   case '\0': *pbyte = 0x0F; break;
   default:
      if (digit >= '0' && digit <= '9') {
         *pbyte = digit - '0';
      }
      else return RTERR_INVCHAR;
   }
   return 0;
}
