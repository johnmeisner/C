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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxReal.h"
#include "rtxsrc/rtxUTF8.h"

EXTRTMETHOD int rtxUTF8StrnToDouble
(const OSUTF8CHAR* utf8str, size_t nbytes, OSREAL* pvalue)
{
   size_t i;
   OSBOOL minus = FALSE;
   OSREAL frac = 0.1, result = 0.0;

   if (0 == pvalue || 0 == utf8str) return RTERR_INVPARAM;
   else *pvalue = 0.0;

   /* Consume leading white space */

   for (i = 0; i < nbytes; i++) {
      if (!OS_ISSPACE (utf8str[i])) break;
   }
   if (i == nbytes) return RTERR_INVREAL;

   /* Consume trailing white space */

   for (; nbytes > 0; nbytes--) {
      if (!OS_ISSPACE (utf8str[nbytes - 1])) break;
   }

   /* Check for '-' first character */

   if (utf8str[i] == '-') {
      minus = TRUE; i++;
      if (i >= nbytes) return RTERR_INVREAL;
   }
   else if (utf8str[i] == '+') {
      i++; /* just skip leading plus sign */
      if (i >= nbytes) return RTERR_INVREAL;
   }

   /* Check for special values - INF, -INF, NaN */

   if (i + 3 == nbytes) {
      if (rtxUTF8StrnEqual (utf8str + i, OSUTF8("INF"), 3)) {
         if (minus)
            *pvalue = rtxGetMinusInfinity ();
         else
            *pvalue = rtxGetPlusInfinity ();
         return 0;
      }
      else if (rtxUTF8StrnEqual (utf8str + i, OSUTF8("NaN"), 3)) {
         *pvalue = rtxGetNaN ();
         return 0;
      }
   }

   /* Convert integer part */

   for ( ; i < nbytes && utf8str[i] != '.' &&
           utf8str[i] != 'e' && utf8str[i] != 'E'; i++ )
   {
      if (OS_ISDIGIT (utf8str[i]))
         result = (result * 10) + (utf8str[i] - '0');
      else
         return RTERR_INVREAL;
   }

   /* Convert fractional part */

   if (i < nbytes && utf8str[i] == '.') {
      for (i++ ; i < nbytes &&
         utf8str[i] != 'E' && utf8str[i] != 'e'; i++, frac *= 0.1)
      {
         if (OS_ISDIGIT (utf8str[i]))
            result += (utf8str[i] - '0') * frac;
         else
            return RTERR_INVREAL;
      }
   }

   /* Convert exponent */

   if (i < nbytes && (utf8str[i] == 'E' || utf8str[i] == 'e')) {
      int sign = 1, expo = 0;

      i++;
      if (i >= nbytes) return RTERR_INVREAL;
      if (utf8str[i] == '-') {
         sign = -1;
         i++;
      }
      else if (utf8str[i] == '+')
         i++;

      if (i >= nbytes) return RTERR_INVREAL;

      for ( ; i < nbytes; i++ ) {
         if (OS_ISDIGIT (utf8str[i]))
            expo = (expo * 10) + (utf8str[i] - '0');
         else
            return RTERR_INVREAL;
      }

      result *= pow ((OSREAL)10.0, expo * sign);
   }

   /* If negative, negate number */

   if (minus) {
      if (result != 0)
         result = 0 - result;
      else
         result = rtxGetMinusZero ();
   }

   *pvalue = result;

   return (0);
}

EXTRTMETHOD int rtxUTF8StrToDouble (const OSUTF8CHAR* utf8str, OSREAL* pvalue)
{
   return rtxUTF8StrnToDouble (utf8str, rtxUTF8LenBytes(utf8str), pvalue);
}
