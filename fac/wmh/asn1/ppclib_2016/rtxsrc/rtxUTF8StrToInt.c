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

#include <stdlib.h>
#include <string.h>
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxUTF8.h"

#ifndef XXSTRNFUNC
#define XXSTRNFUNC rtxUTF8StrnToInt
#define XXSTRFUNC  rtxUTF8StrToInt
#define XXTYPE     OSINT32
#endif

/***********************************************************************
 *
 *  Routine name: rtxUTF8StrToInt
 *
 *  Description:  This routine converts the given UTF-8 string to
 *  an integer value.  It is assumed the string contains only numeric
 *  digits and whitespace.  It is similar to the C atoi function
 *  except that the result is returned as a separate argument and
 *  an error status value is returned if the conversion cannot be
 *  performed successfully.
 *
 *  Inputs:
 *
 *  Name        Type           Description
 *  ----        ----           -----------
 *  utf8str     OSUTF8CHAR*    Numeric UTF-8 string to convert
 *  pvalue      OSINT32*       Pointer to integer to receive result
 *
 *  Outputs:
 *
 *  Name        Type           Description
 *  ----        ----           -----------
 *  stat        int            Status: 0 = OK, negative value = error
 *
 ***********************************************************************/

int XXSTRNFUNC (const OSUTF8CHAR* utf8str, size_t nbytes, XXTYPE* pvalue)
{
   size_t i;
#ifndef XXUNSIGNED
   OSBOOL minus = FALSE;
#endif
   register XXTYPE value = 0, prevVal = 0;

   *pvalue = 0;

   /* Consume leading white space */

   for (i = 0; i < nbytes; i++) {
      if (!OS_ISSPACE (utf8str[i])) break;
   }
   if (i == nbytes) return RTERR_INVFORMAT;

   /* Consume trailing white space */

   for (; nbytes > 0; nbytes--) {
      if (!OS_ISSPACE (utf8str[nbytes - 1])) break;
   }

   /* Check for '+' or '-' first character */

   if (utf8str[i] == '+') {
      i++;
      if (i >= nbytes) return RTERR_INVFORMAT;
   }
   else if (utf8str[i] == '-') {
#ifndef XXUNSIGNED
      minus = TRUE; i++;
      if (i >= nbytes)
#endif
      return RTERR_INVFORMAT;
   }

   /* Convert Unicode characters to an integer value */

   for ( ; i < nbytes; i++, prevVal = value) {
      if (OS_ISDIGIT (utf8str[i]))
         value = (value * 10) + (utf8str[i] - '0');
      else
         return RTERR_INVCHAR;

      if (value/10 != prevVal)  /* test for int overflow */
         return RTERR_TOOBIG;
   }

   /* If negative, negate number */
#ifndef XXUNSIGNED
   if (minus) *pvalue = 0 - value; else
#endif
   *pvalue = value;

   return 0;
}

int XXSTRFUNC (const OSUTF8CHAR* utf8str, XXTYPE* pvalue)
{
   return XXSTRNFUNC (utf8str, OSCRTLSTRLEN ((const char*)utf8str), pvalue);
}
