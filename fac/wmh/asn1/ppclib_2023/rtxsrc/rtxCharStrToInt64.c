/*
 * Copyright (c) 2016-2023 Objective Systems, Inc.
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

#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxErrCodes.h"

static const OSINT64 maxInt64 = OSI64CONST(0x7FFFFFFFFFFFFFFF);
static const OSINT64 maxInt64_10 = OSI64CONST(0xCCCCCCCCCCCCCCC);

EXTRTMETHOD int rtxCharStrToInt64 (const char* cstr, OSINT64* pvalue)
{
   OSBOOL minus = FALSE;
   OSINT64 value = 0;
   OSINT64 maxval = maxInt64;
   OSSIZE i = 0;

   if (0 == cstr || 0 == pvalue) return RTERR_NULLPTR;

   *pvalue = 0;

   /* Consume leading white space */

   while (OS_ISSPACE (cstr[i])) i++;

   /* First character must be a +, -, or digit */
   if (cstr[i] == '+' || cstr[i] == '-') {
      if (cstr[i] == '-') {
         minus = TRUE;
         maxval++; /* support for 0x80000000 */
      }
      i++;
   }

   /* First character after +/- must be a numeric digit */
   if (!OS_ISDIGIT (cstr[i])) return RTERR_INVCHAR;

   /* Read and process numeric digit characters.  End of input is assumed
      to be first non-numeric character */

   for (;;) {
      if (OS_ISDIGIT (cstr[i])) {
         OSINT32 tm = cstr[i] - '0';

         if (value > maxInt64_10)
            return RTERR_TOOBIG;

         value *= 10;

         if ((OSUINT64)value > (OSUINT64)(maxval - tm))
            return RTERR_TOOBIG;

         value += tm;
         i++;
      }
      else {
         break;
      }
   }

   /* If negative, negate number */

   *pvalue = (minus) ? 0 - value : value;

   return (0);
}

