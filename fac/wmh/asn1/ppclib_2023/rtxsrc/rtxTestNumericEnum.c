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

/* These defs allow this function to be used as a template for other
   Numeric Enum test functions (int64, unsigned int, unsigned int64) */
#ifndef RTXTESTNUMERICENUMFUNC
#define RTXTESTNUMERICENUMFUNC rtxTestNumericEnum
#define OSNUMERICENUMITEM OSNumericEnumItem
#define OSINTTYPE OSINT32
#endif

#include "rtxsrc/rtxEnum.h"
#include "rtxsrc/rtxErrCodes.h"

int RTXTESTNUMERICENUMFUNC (OSINTTYPE ivalue,
                        const OSNUMERICENUMITEM enumTable[],
                        OSUINT16 enumTableSize)
{
   size_t lower = 0;
   size_t upper = enumTableSize - 1;

   while (lower < upper && upper != (size_t)-1) {
      size_t middle = (lower + upper)/2;
      int cmpRes;

      if (ivalue < enumTable[middle].start)
         cmpRes = 1;
      else if (ivalue > enumTable[middle].end)
         cmpRes = -1;
      else
         cmpRes = 0;

      if (cmpRes == 0) { /* equal */
         return 0;
      }
      if (cmpRes < 0)
         lower = middle+1;
      else
         upper = middle-1;
   }

   if (lower == upper && ivalue >= enumTable[lower].start &&
                         ivalue <= enumTable[lower].end)
      return 0;

   return RTERR_INVENUM;
}
