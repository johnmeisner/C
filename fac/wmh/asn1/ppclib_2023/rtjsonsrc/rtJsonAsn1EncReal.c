/*
 * Copyright (c) 1997-2023 Objective Systems, Inc.
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

#include "asn1json.h"
#include "rtxsrc/rtxReal.h"

int rtJsonAsn1EncReal(OSCTXT* pctxt, OSREAL value)
{
   int ret = 0;

   if (rtxIsPlusInfinity (value)) {
      ret = rtxCopyAsciiText (pctxt, "\"INF\"");
      if (ret != 0) return LOG_RTERR (pctxt, ret);
      return 0;
   }
   else if (rtxIsMinusInfinity (value)) {
      ret = rtxCopyAsciiText (pctxt, "\"-INF\"");
      if (ret != 0) return LOG_RTERR (pctxt, ret);
      return 0;
   }
   else if (rtxIsNaN (value)) {
      ret = rtxCopyAsciiText (pctxt, "\"NaN\"");
      if (ret != 0) return LOG_RTERR (pctxt, ret);
      return 0;
   }
   else if ( rtxIsMinusZero(value) )
   {
      ret = rtxCopyAsciiText (pctxt, "\"-0\"");
      if (ret != 0) return LOG_RTERR (pctxt, ret);
      return 0;
   }
   else if ( value == 0 )
   {
      ret = rtxCopyAsciiText (pctxt, "0");
      if (ret != 0) return LOG_RTERR (pctxt, ret);
      return 0;
   }
   else
   {
      ret = rtJsonEncDoubleValue(pctxt, value, 0);
      if (ret != 0) return LOG_RTERR (pctxt, ret);
      return 0;
   }

   return ret;
}


int rtJsonAsn1EncReal10(OSCTXT* pctxt, const OSUTF8CHAR* pvalue)
{
   OSBOOL bNeg = FALSE;    /* TRUE if '-' sign appeared */
   const char* p = (const char*)pvalue;    /* pointer into pvalue */
   int ret = 0;
   size_t len = OSUTF8LEN(pvalue);

   if ( *pvalue == '+' || *pvalue == '-' )
   {
      bNeg = (*pvalue == '-' );
      p++;      /* move past sign */
   }

   if ( 0 == rtxStricmp(p, "INF") || 0 == rtxStricmp(p, "INFINITY") )
   {
      const char* negInfinity = "-INF";
      const char* penc = bNeg ? negInfinity : negInfinity + 1;

      ret = rtJsonEncCharStr (pctxt, penc);
      if (ret != 0) return LOG_RTERR (pctxt, ret);
   }
   else if ( 0 == rtxStrnicmp((const char*)p, "NAN", 3) &&
               ( p[3] == 0 || ( p[3] == '(' && pvalue[len-1] == ')' ) ) )
   {
      /* String is just NAN or NAN(n-char-sequence).  This isn't exact true,
         as technically we should make sure n-char-sequence does not have
         a ')' in it.
      */
      ret = rtJsonEncChars (pctxt, "NaN", 3);
      if (ret != 0) return LOG_RTERR (pctxt, ret);
   }
   else {
      /* Verify the string matches a JSON number format & determine whether
         the string's value is zero or not.  If zero, encode as a string,
         otherwise encode the number as-is.
      */
      OSBOOL bZero = TRUE;                      /* true if value is zero */
      const char* pterm = (const char*)pvalue + len; /* terminating char */
      if ( *p >= '1' && *p <= '9' )
      {
         bZero = FALSE;
         /* accept all digits */
         while ( p < pterm && *p >= '0' && *p <= '9' ) p++;
      }
      else if ( *p == '0' ) p++;
      else {
         rtxErrAddStrParm(pctxt, (const char*)pvalue);
         return LOG_RTERRNEW(pctxt, RTERR_BADVALUE);
      }

      if ( *p == '.' )
      {
         /* accept all digits for the fraction */
         p++;
         while ( p < pterm && *p >= '0' && *p <= '9' )
         {
            bZero = bZero && *p == '0';
            p++;
         }
      }

      if ( *p == 'e' || *p == 'E' )
      {
         /* accept optional sign for the exponent */
         p++;
         if ( *p == '-' || *p == '+' ) p++;

         /* accept all digits for the exponent */
         while ( p < pterm && *p >= '0' && *p <= '9' ) p++;
      }

      if ( p != pterm )
      {
         /* string did not end as expected */
         rtxErrAddStrParm(pctxt, (const char*)pvalue);
         return LOG_RTERRNEW(pctxt, RTERR_BADVALUE);
      }

      if ( bZero && bNeg ) rtJsonEncCharStr (pctxt, "-0");
      else if ( bZero ) ret = rtJsonEncStringRaw (pctxt, OSUTF8("0"));
      else ret = rtJsonEncStringRaw(pctxt, pvalue);

      if (ret != 0) return LOG_RTERR (pctxt, ret);
   }

   return ret;
}
