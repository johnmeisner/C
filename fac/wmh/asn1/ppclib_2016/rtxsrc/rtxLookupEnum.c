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

/* These defs allow this function to be used as a template for other
   Numeric Enum test functions (int64, unsigned int, unsigned int64) */
#ifndef RTXLOOKUPENUMFUNC
#define RTXLOOKUPENUMFUNC rtxLookupEnum
#define OSENUMITEM OSEnumItem
#define OSINTTYPE OSINT32
#endif

#ifndef NOTINCBYVALUEFUNC
#ifndef RTXLOOKUPENUMBYVALUEFUNC
#define RTXLOOKUPENUMBYVALUEFUNC rtxLookupEnumByValue
#define OSENUMITEM OSEnumItem
#define OSINTTYPE OSINT32
#endif
#endif

#include "rtxsrc/rtxEnum.h"
#include "rtxsrc/rtxUTF8.h"
#include "rtxsrc/rtxErrCodes.h"

EXTRTMETHOD OSINT32 RTXLOOKUPENUMFUNC (const OSUTF8CHAR* strValue,
                       size_t strValueSize,
                       const OSENUMITEM enumTable[],
                       OSUINT16 enumTableSize)
{
   size_t lower = 0;
   size_t upper = enumTableSize - 1;
   size_t middle;
   int    cmpRes;

   if (strValueSize == (size_t)-1) {
      strValueSize = rtxUTF8LenBytes (strValue);
   }

   while (lower < upper && upper != (size_t)-1) {
      middle = (lower + upper)/2;

      cmpRes = rtxUTF8Strncmp (enumTable[middle].name, strValue, strValueSize);

      if (cmpRes == 0)
         cmpRes = (int)enumTable[middle].namelen - (int)strValueSize;

      if (cmpRes == 0) { /* equal */
         return (int)middle;
      }
      if (cmpRes < 0)
         lower = middle+1;
      else
         upper = middle-1;
   }

   if (lower == upper && (size_t)enumTable[lower].namelen == strValueSize &&
       rtxUTF8Strncmp (enumTable[lower].name, strValue, strValueSize) == 0) {
      return (int)lower;
   }

   return RTERR_INVENUM;
}

#ifndef NOTINCBYVALUEFUNC
/*
 * Lookup enum by integer value.  Required for ASN.1 because enumerated
 * values do not need to be sequential.
 */
EXTRTMETHOD OSINT32 RTXLOOKUPENUMBYVALUEFUNC
(OSINTTYPE value, const OSENUMITEM enumTable[], size_t enumTableSize)
{
   size_t i;
   for (i = 0; i < enumTableSize; i++) {
      if (enumTable[i].value == value) return (int)i;
   }
   return RTERR_INVENUM;
}
#endif
