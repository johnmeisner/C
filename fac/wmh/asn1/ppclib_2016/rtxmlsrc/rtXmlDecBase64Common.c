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

#include "rtxsrc/rtxCtype.h"
#include "rtxmlsrc/osrtxml.hh"

/*
 * This array is a lookup table that translates characters
 * drawn from the "Base64 Alphabet" (as specified in Table 1 of RFC 2045)
 * into their 6-bit positive integer equivalents. Characters that
 * are not in the Base64 alphabet but fall within the bounds of the
 * array are translated to -1. Note, first 40 values are omitted, because
 * all of them are -1. Use offset -40 to fetch values from this table.
 */
const signed char decodeTable[] = {
   /*
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1, */
   -1, -1, -1, 62, -1, -1, -1, 63,
   52, 53, 54, 55, 56, 57, 58, 59,
   60, 61, -1, -1, -1,  0, -1, -1,
   -1,  0,  1,  2,  3,  4,  5,  6,
   7,   8,  9, 10, 11, 12, 13, 14,
   15, 16, 17, 18, 19, 20, 21, 22,
   23, 24, 25, -1, -1, -1, -1, -1,
   -1, 26, 27, 28, 29, 30, 31, 32,
   33, 34, 35, 36, 37, 38, 39, 40,
   41, 42, 43, 44, 45, 46, 47, 48,
   49, 50, 51, -1, -1, -1, -1, -1
};

EXTXMLMETHOD int rtXmlGetBase64StrDecodedLen
   (const OSUTF8CHAR* inpdata, size_t srcDataSize,
    size_t* pNumOcts, size_t* pSrcDataLen)
{
   size_t numGroups = srcDataSize/4;
   size_t encLen, i;
   int missingBytesInLastGroup = 0;

   for (encLen = 0, i = 0; i < srcDataSize; i++) {
      if (!OS_ISSPACE (inpdata[i])) encLen++;
      else continue;

      if (inpdata[i] == '=') {
         if (++missingBytesInLastGroup > 2)
            return RTERR_INVBASE64;
         continue;
      }

      if (missingBytesInLastGroup)
         return RTERR_INVBASE64;

      if (BASE64TOINT(inpdata[i]) < 0)
         return RTERR_INVBASE64;
   }

   numGroups = encLen/4;

   if (pSrcDataLen != 0)
      *pSrcDataLen = encLen;

   if (pNumOcts != 0)
      *pNumOcts = 3*numGroups - missingBytesInLastGroup;

   return 0;
}


