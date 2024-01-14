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

#ifndef RTXLOOKUPENUMFUNC
#define RTXLOOKUPENUMFUNC rtxLookupBigEnum
#define OSENUMITEM OSBigEnumItem
#define OSINTTYPE OSINT32
#define NOTINCBYVALUEFUNC
#endif

#include "rtxLookupEnum.c"

/*
 * Lookup enum by stringified version of value.  Required for ASN.1 because
 * enumerated values do not need to be sequential.
 */
EXTRTMETHOD OSINT32 rtxLookupBigEnumByValue
(const char* value, const OSBigEnumItem enumTable[], size_t enumTableSize)
{
   size_t i;
   int    cmpRes;
   size_t strValueSize = rtxUTF8LenBytes (OSUTF8(value));

   for (i = 0; i < enumTableSize; i++) {
      cmpRes = rtxUTF8Strncmp (enumTable[i].name, OSUTF8(value), strValueSize);

      if (cmpRes == 0)
         cmpRes = (int)enumTable[i].namelen - (int)strValueSize;

      if (cmpRes == 0) { /* equal */
         return (int)i;
      }
   }
   return RTERR_INVENUM;
}
