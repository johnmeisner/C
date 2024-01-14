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

#define BASE64TOINT(c) ((((unsigned)(c)) < 128) ? decodeTable [(c) - 40] : -1)
/*
 * This array is a lookup table that translates characters
 * drawn from the "Base64 Alphabet" (as specified in Table 1 of RFC 2045)
 * into their 6-bit positive integer equivalents. Characters that
 * are not in the Base64 alphabet but fall within the bounds of the
 * array are translated to -1. Note, first 40 values are omitted, because
 * all of them are -1. Use offset -40 to fetch values from this table.
 */
static const char decodeTable[] = {
   /*
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1, */
   -1, -1, -1, 62, -1, -1, -1, 63,
   52, 53, 54, 55, 56, 57, 58, 59,
   60, 61, -1, -1, -1, -1, -1, -1,
   -1,  0,  1,  2,  3,  4,  5,  6,
   7,   8,  9, 10, 11, 12, 13, 14,
   15, 16, 17, 18, 19, 20, 21, 22,
   23, 24, 25, -1, -1, -1, -1, -1,
   -1, 26, 27, 28, 29, 30, 31, 32,
   33, 34, 35, 36, 37, 38, 39, 40,
   41, 42, 43, 44, 45, 46, 47, 48,
   49, 50, 51, -1, -1, -1, -1, -1
};


long xerGetBase64StrDecodedLen (OSCTXT* pctxt, size_t srcDataSize)
{
   XMLCHAR* inpdata = (XMLCHAR*) ASN1BUFPTR (pctxt);
   size_t numGroups = srcDataSize/4;
   int missingBytesInLastGroup = 0;

   if (4*numGroups != srcDataSize)
      return LOG_RTERR (pctxt, RTERR_INVBASE64);
   if (srcDataSize != 0) {
      if (inpdata[srcDataSize - 1] == '=')
         missingBytesInLastGroup++;
      if (inpdata[srcDataSize - 2] == '=')
         missingBytesInLastGroup++;
   }
   return (long)(3*numGroups - missingBytesInLastGroup);
}

int xerDecBase64StrValue (OSCTXT* pctxt,
                          OSOCTET* pvalue, OSUINT32* pnocts,
                          size_t bufSize, size_t srcDataSize)
{
   OSSIZE nocts64;
   int stat = 0;

   stat = xerDecBase64StrValue64(pctxt, pvalue, &nocts64,
                                 bufSize, srcDataSize);
   if (stat >= 0)
   {
      if ((sizeof(nocts64) > 4) && (nocts64 > OSUINT32_MAX))
      {
         return LOG_RTERR(pctxt, RTERR_TOOBIG);
      }
      else
      {
         if (pnocts)
         {
            *pnocts = (OSUINT32)nocts64;
         }
      }
   }

   return stat;
}

int xerDecBase64StrValue64 (OSCTXT* pctxt,
                            OSOCTET* pvalue, OSSIZE* pnocts,
                            OSSIZE bufSize, OSSIZE srcDataSize)
{
   XMLCHAR* inpdata = (XMLCHAR*) ASN1BUFPTR (pctxt);
   OSOCTET* pvalueStart = pvalue;
   size_t numGroups = srcDataSize/4;
   size_t missingBytesInLastGroup = 0;
   size_t i, numFullGroups = numGroups;
   int ch0, ch1, ch2, ch3;

   *pvalue = 0;
   if (4*numGroups != srcDataSize)
      return LOG_RTERR (pctxt, RTERR_INVBASE64);

   if (srcDataSize != 0) {
      if (inpdata[srcDataSize - 1] == '=') {
         missingBytesInLastGroup++;
         numFullGroups--;
      }
      if (inpdata[srcDataSize - 2] == '=')
         missingBytesInLastGroup++;
   }

   if (bufSize < 3*numGroups - missingBytesInLastGroup)
      return LOG_RTERR (pctxt, RTERR_INVBASE64);

   /* Translate all full groups from base64 to byte array elements */
   for (i = 0; i < numFullGroups; i++) {
      ch0 = BASE64TOINT (*inpdata); inpdata++;
      ch1 = BASE64TOINT (*inpdata); inpdata++;
      ch2 = BASE64TOINT (*inpdata); inpdata++;
      ch3 = BASE64TOINT (*inpdata); inpdata++;
      if ((ch0 | ch1 | ch2 | ch3) < 0)
         return LOG_RTERR (pctxt, RTERR_INVBASE64);
      *pvalue++ = (OSOCTET) ((ch0 << 2) | (ch1 >> 4));
      *pvalue++ = (OSOCTET) ((ch1 << 4) | (ch2 >> 2));
      *pvalue++ = (OSOCTET) ((ch2 << 6) | ch3);
   }

   /* Translate partial group, if present */
   if (missingBytesInLastGroup != 0) {
      ch0 = BASE64TOINT (*inpdata); inpdata++;
      ch1 = BASE64TOINT (*inpdata); inpdata++;
      if ((ch0 | ch1) < 0)
         return LOG_RTERR (pctxt, RTERR_INVBASE64);
      *pvalue++ = (OSOCTET) ((ch0 << 2) | (ch1 >> 4));

      if (missingBytesInLastGroup == 1) {
         ch2 = BASE64TOINT (*inpdata); inpdata++;
         if (ch2 < 0)
            return LOG_RTERR (pctxt, RTERR_INVBASE64);
         *pvalue++ = (OSOCTET) ((ch1 << 4) | (ch2 >> 2));
      }
   }
   if (pnocts != 0) *pnocts = (OSUINT32)(pvalue - pvalueStart);
   return 0;
}

