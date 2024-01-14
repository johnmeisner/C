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

#include <math.h>
#include "xed_common.hh"

int xmlDecReal (OSCTXT* pctxt, OSREAL* pvalue)
{
   XMLCHAR* inpdata = (XMLCHAR*) ASN1BUFPTR (pctxt);
   OSBOOL minus = FALSE;
   OSREAL frac = 0.1, result = 0.0;

   /* Consume leading white space */

   for (; *inpdata != 0 && OS_ISSPACE (*inpdata); *inpdata++)
      ;

   /* Check for '-' first character */

   if (*inpdata == '-') {
      minus = TRUE; inpdata++;
      if (*inpdata == 0)
         return LOG_RTERR (pctxt, RTERR_INVREAL);
   }
   else if (*inpdata == '+') {
      inpdata++; /* just skip leading plus sign */
   }

   if (*inpdata == 0)
      return LOG_RTERR (pctxt, RTERR_INVREAL);

   /* Check for special values - INF, -INF, NaN */

   if (xerCmpText (inpdata, "INF")) {
      if (minus)
         *pvalue = rtxGetMinusInfinity ();
      else
         *pvalue = rtxGetPlusInfinity ();
      return 0;
   }
   else if (xerCmpText (inpdata, "NaN")) {
      *pvalue = rtxGetNaN ();
      return 0;
   }

   /* Convert integer part */

   for ( ; *inpdata != 0 && *inpdata != '.'; inpdata++ ) {
      if (OS_ISDIGIT (*inpdata))
         result = (result * 10) + (*inpdata - '0');
      else
         return LOG_RTERR (pctxt, RTERR_INVREAL);
   }

   /* Convert fractional part */

   if (*inpdata == '.') {
      for (inpdata++ ; *inpdata != 0 &&
         *inpdata != 'E' && *inpdata != 'e'; inpdata++, frac *= 0.1)
      {
         if (OS_ISDIGIT (*inpdata))
            result += (*inpdata - '0') * frac;
         else
            return LOG_RTERR (pctxt, RTERR_INVREAL);
      }
   }

   /* Convert exponent */

   if (*inpdata == 'E' || *inpdata == 'e') {
      /* To be clear, only 'E' should be accepted in XER,
         but who knows?.. (AB) */
      int sign = 1, exp = 0;

      inpdata++;
      if (*inpdata == '-') {
         sign = -1;
         inpdata++;
      }
      else if (*inpdata == '+')
         inpdata++;

      if (*inpdata == 0)
         return LOG_RTERR (pctxt, RTERR_INVREAL);

      for ( ; *inpdata != 0; inpdata++ ) {
         if (OS_ISDIGIT (*inpdata))
            exp = (exp * 10) + (*inpdata - '0');
         else
            return LOG_RTERR (pctxt, RTERR_INVREAL);
      }

      result *= pow ((OSREAL)10.0, exp * sign);
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

