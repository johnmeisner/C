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

int rtJsonAsn1DecReal(OSCTXT* pctxt, OSREAL* pvalue)
{
   int ret = 0;
   OSUTF8CHAR* pstrValue;
   OSUTF8CHAR nextChar;
   char* endptr;

   /* Need to peek ahead and determine if we have a JSON string, a JSON object,
   or a JSON number next. */

   ret = rtJsonDecPeekChar(pctxt, &nextChar);
   if (ret != 0) return LOG_RTERR (pctxt, ret);

   if ( nextChar == '"' )
   {
      /* JSON string. The value should be a special real, excl. 0 */
      ret = rtJsonDecStringValue(pctxt, &pstrValue);
      if (ret != 0) return LOG_RTERR (pctxt, ret);

      if ( rtxUTF8StrEqual(OSUTF8("-0"), pstrValue) )
         *pvalue = -0;
      else if ( rtxUTF8StrEqual(OSUTF8("-INF"), pstrValue) )
         *pvalue = rtxGetMinusInfinity();
      else if ( rtxUTF8StrEqual (OSUTF8("INF"), pstrValue) )
         *pvalue = rtxGetPlusInfinity();
      else if ( rtxUTF8StrEqual (OSUTF8("NaN"), pstrValue) )
         *pvalue = rtxGetNaN();
      else {
         rtxErrAddStrParm(pctxt, (const char*)pstrValue);
         ret = LOG_RTERRNEW(pctxt, RTERR_BADVALUE);
      }

      rtxMemFreePtr(pctxt, pstrValue);
      return ret;
   }
   else if ( nextChar == '{' )
   {
      /* JSON object.  There should be a single entry named base10Value. */
      OSUTF8NameAndLen nameArray[1];

      /* Match expected name token */
      nameArray[0].name = OSUTF8("base10Value");
      nameArray[0].len = 11;

      ret = rtJsonDecMatchObjectStart (pctxt, nameArray, 1);
      if (ret != 0) return LOG_RTERR (pctxt, ret);

      ret = rtJsonDecNumberString(pctxt, (char**)&pstrValue);
      if (ret != 0) return LOG_RTERR (pctxt, ret);

      ret = rtJsonDecMatchChar(pctxt, '}');
      if (ret != 0) {
         rtxMemFreePtr( pctxt, pstrValue );
         return LOG_RTERR (pctxt, ret);
      }

      /* we'll convert pstrValue below */
   }
   else {
      /* We expect a JSON number */
      ret = rtJsonDecNumberString(pctxt, (char**)&pstrValue);
      if (ret != 0) return LOG_RTERR (pctxt, ret);
   }

   /* pstrValue is a JSON number; convert it to double */
   *pvalue = strtod((const char*)pstrValue, &endptr);

   rtxMemFreePtr(pctxt, pstrValue);

   return 0;
}



int rtJsonAsn1DecReal10(OSCTXT* pctxt, OSUTF8CHAR** ppvalue)
{
   int ret = 0;
   OSUTF8CHAR* pstrValue;
   OSUTF8CHAR nextChar;

   /* Need to peek ahead and determine if we have a JSON string, a JSON object,
   or a JSON number next. */

   ret = rtJsonDecPeekChar(pctxt, &nextChar);
   if (ret != 0) return LOG_RTERR (pctxt, ret);

   if ( nextChar == '"' )
   {
      /* JSON string. The value should be a special real, excl. 0. */
      ret = rtJsonDecStringValue(pctxt, &pstrValue);
      if (ret != 0) return LOG_RTERR (pctxt, ret);

      if ( rtxUTF8StrEqual(OSUTF8("-0"), pstrValue) ||
           rtxUTF8StrEqual(OSUTF8("-INF"), pstrValue) ||
           rtxUTF8StrEqual (OSUTF8("INF"), pstrValue) ||
           rtxUTF8StrEqual (OSUTF8("NaN"), pstrValue) ) {
         *ppvalue = pstrValue;
      }
      else {
         rtxErrAddStrParm(pctxt, (const char*)pstrValue);
         ret = LOG_RTERRNEW(pctxt, RTERR_BADVALUE);
      }

      if ( ret != 0 ) rtxMemFreePtr(pctxt, pstrValue);
      return ret;
   }
   else if ( nextChar == '{' )
   {
      /* JSON object.  There should be a single entry named base10Value. */
      OSUTF8NameAndLen nameArray[1];

      /* Match expected name token */
      nameArray[0].name = OSUTF8("base10Value");
      nameArray[0].len = 11;

      ret = rtJsonDecMatchObjectStart (pctxt, nameArray, 1);
      if (ret != 0) return LOG_RTERR (pctxt, ret);

      ret = rtJsonDecNumberString(pctxt, (char**)&pstrValue);
      if (ret != 0) return LOG_RTERR (pctxt, ret);

      ret = rtJsonDecMatchChar(pctxt, '}');
      if (ret != 0) {
         rtxMemFreePtr( pctxt, pstrValue );
         return LOG_RTERR (pctxt, ret);
      }

      /* we'll return pstrValue below */
   }
   else {
      /* We expect a JSON number */
      ret = rtJsonDecNumberString(pctxt, (char**)&pstrValue);
      if (ret != 0) return LOG_RTERR (pctxt, ret);
   }

   /* pstrValue is a JSON number; convert it to double */
   *ppvalue = pstrValue;

   return 0;
}

