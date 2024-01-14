/*
 * Copyright (c) 2019-2023 Objective Systems, Inc.
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

#include "rtAvn.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxReal.h"
#include "rtxsrc/rtxText.h"

static int parseDigits
(OSCTXT* pctxt, char* outbuf, OSSIZE* pbufidx, OSSIZE bufsize)
{
   /* Parse digits */
   for (;;) {
      char ch = rtxTxtPeekChar2(pctxt, FALSE);

      if (OS_ISDIGIT(ch)) {
         if (*pbufidx < bufsize) {
            outbuf[(*pbufidx)++] = ch;
            pctxt->buffer.byteIndex++;
         }
         else return LOG_RTERR(pctxt, RTERR_TOOBIG);
      }
      else break;
   }

   return 0;
}

EXTRTMETHOD int rtAvnReadReal(OSCTXT* pctxt, OSREAL* pvalue)
{
   int ret = 0;
   OSUTF8CHAR* pstrValue;
   OSUTF8CHAR ch;
   char* endptr;

   ret = rtxTxtSkipWhitespace(pctxt);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   /* Peek to determine whether we have a special value or a realnumber. */
   ch = rtxTxtPeekChar2(pctxt, FALSE);
   if (ch == 'P') {
      ret = rtxTxtMatchChars(pctxt, OSUTF8("PLUS-INFINITY"), FALSE);
      if (ret != 0) return LOG_RTERR(pctxt, ret);
      *pvalue = rtxGetPlusInfinity();
   }
   else if (ch == 'M') {
      ret = rtxTxtMatchChars(pctxt, OSUTF8("MINUS-INFINITY"), FALSE);
      if (ret != 0) return LOG_RTERR(pctxt, ret);
      *pvalue = rtxGetMinusInfinity();
   }
   else if (ch == 'N') {
      ret = rtxTxtMatchChars(pctxt, OSUTF8("NOT-A-NUMBER"), FALSE);
      if (ret != 0) return LOG_RTERR(pctxt, ret);
      *pvalue = rtxGetNaN();
   }
   else {
      ret = rtAvnReadRealString(pctxt, (char**)&pstrValue);
      if (ret != 0) return LOG_RTERR(pctxt, ret);

      *pvalue = strtod((const char*)pstrValue, &endptr);

      rtxMemFreePtr(pctxt, pstrValue);
   }

   return 0;
}


EXTRTMETHOD int rtAvnReadRealString(OSCTXT* pctxt, char** ppCharStr)
{
   char ch, tmpbuf[1025];
   OSSIZE tmpidx = 0, bufsize = sizeof(tmpbuf) - 1;
   int ret = 0;

   ret = rtxTxtSkipWhitespace(pctxt);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   /* TODO: current implementation has size limit = sizeof(tmpbuf)-1.
      Use MemBuf to allow arbitray sized numbers to be parsed */

   ch = rtxTxtPeekChar2(pctxt, FALSE);
   if (0 == ch) return LOG_RTERR (pctxt, RTERR_ENDOFBUF);

   /* Parse optional minus sign */
   if (ch == '-') {
      tmpbuf[tmpidx++] = ch;
      pctxt->buffer.byteIndex++;

      ch = rtxTxtPeekChar2(pctxt, FALSE);
      if (0 == ch) return LOG_RTERR (pctxt, RTERR_ENDOFBUF);
   }

   /* There must be at least one digit in the integer part. */
   if (!OS_ISDIGIT(ch)) return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);

   /* Parse the first digit and any subsequent digits for integer part. */
   tmpbuf[tmpidx++] = ch;
   pctxt->buffer.byteIndex++;

   /* Parse other digits */
   ret = parseDigits (pctxt, tmpbuf, &tmpidx, bufsize);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   ch = rtxTxtPeekChar2(pctxt, FALSE);


   /* Check for optional floating point. */
   if (ch == '.') {
      if (tmpidx < bufsize) {
         tmpbuf[tmpidx++] = ch;
         pctxt->buffer.byteIndex++;
      }
      else return LOG_RTERR (pctxt, RTERR_TOOBIG);

      /* Parse optional fractional digits. */
      ret = parseDigits (pctxt, tmpbuf, &tmpidx, bufsize);
      if (0 != ret) return LOG_RTERR (pctxt, ret);

      ch = rtxTxtPeekChar2(pctxt, FALSE);
   }

   /* Check for optional exponent. */
   if (ch == 'E' || ch == 'e') {
      if (tmpidx < bufsize) {
         tmpbuf[tmpidx++] = ch;
         pctxt->buffer.byteIndex++;
      }
      else return LOG_RTERR (pctxt, RTERR_TOOBIG);

      /* Check for optional exponent sign. */
      ch = rtxTxtPeekChar2(pctxt, FALSE);
      if (ch == '+' || ch == '-') {
         if (tmpidx < bufsize) {
            tmpbuf[tmpidx++] = ch;
            pctxt->buffer.byteIndex++;
         }
         else return LOG_RTERR (pctxt, RTERR_TOOBIG);

         ch = rtxTxtPeekChar2(pctxt, FALSE);
      }

      /* Parse digits.  There must be at least one digit.  If the first one
         is zero, that must be the end of the string.
      */
      if (!OS_ISDIGIT(ch)) return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);

      /* Parse the first digit and any subsequent digits for integer part. */
      tmpbuf[tmpidx++] = ch;
      pctxt->buffer.byteIndex++;

      if (ch == '0') {
         /* Make sure there is no more input or next character is not
            part of the exponent (it is not a digit) */
         ch = rtxTxtPeekChar2(pctxt, FALSE);
         if (OS_ISDIGIT(ch)) return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);
      }

      /* Parse other digits */
      ret = parseDigits(pctxt, tmpbuf, &tmpidx, bufsize);
      if (0 != ret) return LOG_RTERR(pctxt, ret);
   }

   tmpbuf[tmpidx++] = '\0';

   /* Allocate memory for number string and copy number */
   if (0 != ppCharStr) {
      *ppCharStr = rtxStrdup (pctxt, tmpbuf);
      if (0 == *ppCharStr) return LOG_RTERR (pctxt, RTERR_NOMEM);
   }

   return 0;
}
