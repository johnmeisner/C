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

int xerDecReal10 (OSCTXT* pctxt, const OSUTF8CHAR** pvalue)
{
   /* The input buffer will contain the input data. */
   /* Just validate the input and return a copy of it. */

   OSUTF8CHAR* str;     /* allocated string to hold copy of input */
   XMLCHAR* startInput; /* start of actual input string, ignoring whitespace */
   XMLCHAR* endInput;   /* end of actual input string, ignoring whitespace/null */
   OSSIZE i, nbytes;    /* length of actual input, ignoring whitespace/null */
   XMLCHAR* inpdata = (XMLCHAR*) ASN1BUFPTR (pctxt);

   /* Consume leading white space */

   //for (; *inpdata != 0 && OS_ISSPACE (*inpdata); *inpdata++) ;

   startInput = inpdata;    /* save pointer to start of actual data */

   /* Note: infinities and NaN are not supported.  The encoder does not handle */
   /* them and NOTE 2 on X.680 21.5 suggests they would be excluded. That */
   /* same note suggests +/- zero would be excluded, but we'll allow them */
   /* to be lax. */

   if (*inpdata == 0)
      return LOG_RTERR (pctxt, RTERR_INVREAL);

   /* Check for '-' first character */

   if (*inpdata == '-') {
      inpdata++;
      if (*inpdata == 0)
         return LOG_RTERR (pctxt, RTERR_INVREAL);
   }

   /* parse integer part */

   for ( ; *inpdata != 0 && *inpdata != '.'; inpdata++ ) {
      if (!OS_ISDIGIT (*inpdata))
         return LOG_RTERR (pctxt, RTERR_INVREAL);
   }

   /* parse fractional part */

   if (*inpdata == '.') {
      for (inpdata++ ; *inpdata != 0 &&
         *inpdata != 'E' && *inpdata != 'e'; inpdata++)
      {
         if (!OS_ISDIGIT (*inpdata))
            return LOG_RTERR (pctxt, RTERR_INVREAL);
      }
   }

   /* Convert exponent */

   if (*inpdata == 'E' || *inpdata == 'e') {
      inpdata++;

      if (*inpdata == '+' || *inpdata == '-')
         inpdata++;

      if (*inpdata == 0)
         return LOG_RTERR (pctxt, RTERR_INVREAL);

      for ( ; OS_ISDIGIT (*inpdata); inpdata++ ) {
      }

   }

   /* inpdata now points to first char beyond actual input.  This */
   /* *should* be either whitespace or terminating null char, and if whitepapce, */
   /* then all chars to the end of string should be whitespace. */
   endInput = inpdata - 1;

   /* skip over any trailing whitespace */
   //for (; *inpdata != 0 && OS_ISSPACE (*inpdata); *inpdata++) ;

   /* if inpdata is not now the null terminator, then the input had some */
   /* bad characters */
   if (*inpdata != 0)
      return LOG_RTERR (pctxt, RTERR_INVREAL);


   /* Allocate memory for output data */
   nbytes = (OSSIZE)(endInput - startInput + 1);
   str = (OSUTF8CHAR*) rtxMemAlloc (pctxt, nbytes + 1);
   if (str != 0) {
      /* Copy source chars to destination */
      for (i = 0; i < nbytes; i++) {
         str[i] = startInput[i];
      }
   }
   else
      return LOG_RTERR (pctxt, RTERR_NOMEM);

   str[nbytes] = 0;
   *pvalue = str;
   return (0);
}

