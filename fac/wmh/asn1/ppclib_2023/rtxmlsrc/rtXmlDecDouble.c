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
#include "rtxsrc/rtxCtype.h"
#include "rtxmlsrc/osrtxml.hh"

EXTXMLMETHOD int rtXmlDecDouble (OSCTXT* pctxt, OSREAL* pvalue)
{
   const OSUTF8CHAR* const inpdata = (const OSUTF8CHAR*) OSRTBUFPTR (pctxt);
   size_t i;
   size_t nbytes = OSRTBUFSIZE(pctxt);
   OSBOOL minus = FALSE;
   OSREAL frac = 0.1, result = 0.0;

   /* Consume leading white space */

   for (i = 0; i < nbytes; i++) {
      if (!OS_ISSPACE (inpdata[i])) break;
   }
   if (i == nbytes) return LOG_RTERRNEW (pctxt, RTERR_INVREAL);

   /* Consume trailing white space */

   for (; nbytes > 0; nbytes--) {
      if (!OS_ISSPACE (inpdata[nbytes - 1])) break;
   }

   /* Check for '-' first character */

   if (inpdata[i] == '-') {
      minus = TRUE; i++;
      if (i >= nbytes) return LOG_RTERRNEW (pctxt, RTERR_INVREAL);
   }
   else if (inpdata[i] == '+') {
      i++; /* just skip leading plus sign */
      if (i >= nbytes) return LOG_RTERRNEW (pctxt, RTERR_INVREAL);
   }

   /* Check for special values - INF, -INF, NaN */

   if (i + 3 == nbytes) {
      if (rtXmlStrnCmpAsc (inpdata + i, "INF", 3)) {
         if (minus)
            *pvalue = rtxGetMinusInfinity ();
         else
            *pvalue = rtxGetPlusInfinity ();
         return 0;
      }
      else if (rtXmlStrnCmpAsc (inpdata, "NaN", 3)) {
         *pvalue = rtxGetNaN ();
         return 0;
      }
   }

   /* Convert integer part */

   for ( ; i < nbytes && inpdata[i] != '.' &&
           inpdata[i] != 'e' && inpdata[i] != 'E'; i++ )
   {
      if (OS_ISDIGIT (inpdata[i]))
         result = (result * 10) + (inpdata[i] - '0');
      else
         return LOG_RTERRNEW (pctxt, RTERR_INVREAL);
   }

   /* Convert fractional part */

   if (i < nbytes && inpdata[i] == '.') {
      for (i++ ; i < nbytes &&
         inpdata[i] != 'E' && inpdata[i] != 'e'; i++, frac *= 0.1)
      {
         if (OS_ISDIGIT (inpdata[i]))
            result += (inpdata[i] - '0') * frac;
         else
            return LOG_RTERRNEW (pctxt, RTERR_INVREAL);
      }
   }

   /* Convert exponent */

   if (i < nbytes && (inpdata[i] == 'E' || inpdata[i] == 'e')) {
      int sign = 1, exponent = 0;

      i++;
      if (i >= nbytes) return LOG_RTERRNEW (pctxt, RTERR_INVREAL);
      if (inpdata[i] == '-') {
         sign = -1;
         i++;
      }
      else if (inpdata[i] == '+')
         i++;

      if (i >= nbytes) return LOG_RTERRNEW (pctxt, RTERR_INVREAL);

      for ( ; i < nbytes; i++ ) {
         if (OS_ISDIGIT (inpdata[i]))
            exponent = (exponent * 10) + (inpdata[i] - '0');
         else
            return LOG_RTERRNEW (pctxt, RTERR_INVREAL);
      }

      result *= pow ((OSREAL)10.0, exponent * sign);
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


