/*
 * Copyright (c) 2018-2018 Objective Systems, Inc.
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

#include "rtjsonsrc/osrtjson.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCtype.h"

static int parseDigits
(OSCTXT* pctxt, char* outbuf, OSSIZE* pbufidx, OSSIZE bufsize)
{
   /* Parse digits */
   for (;;) {
      char ch = rtJsonDecPeekChar2 (pctxt);

      if (OS_ISDIGIT(ch)) {
         if (*pbufidx < bufsize) {
            outbuf[(*pbufidx)++] = ch;
            pctxt->buffer.byteIndex++;
         }
         else return LOG_RTERR (pctxt, RTERR_TOOBIG);
      }
      else break;
   }

   return 0;
}

int rtJsonDecNumberString (OSCTXT* pctxt, char** ppCharStr)
{
   char ch, tmpbuf[1025];
   OSSIZE tmpidx = 0, bufsize = sizeof(tmpbuf) - 1;
   OSBOOL needDigit = FALSE;
   int ret = 0;

   /* TODO: current implementation has size limit = sizeof(tmpbuf)-1.
      Use MemBuf to allow arbitray sized numbers to be parsed */

   ch = rtJsonDecPeekChar2 (pctxt);
   if (0 == ch) return LOG_RTERR (pctxt, RTERR_ENDOFBUF);

   /* Parse optional minus sign */
   if (ch == '-') {
      tmpbuf[tmpidx++] = ch;
      pctxt->buffer.byteIndex++;

      ch = rtJsonDecPeekChar2 (pctxt);
      if (0 == ch) return LOG_RTERR (pctxt, RTERR_ENDOFBUF);

      needDigit = TRUE;
   }

   /* Parse leading digit */
   if (ch == '0') {
      needDigit = FALSE;
      tmpbuf[tmpidx++] = ch;
      pctxt->buffer.byteIndex++;

      ch = rtJsonDecPeekChar2 (pctxt);
   }
   else if (ch >= '1' && ch <= '9') {
      needDigit = FALSE;
      tmpbuf[tmpidx++] = ch;
      pctxt->buffer.byteIndex++;

      /* Parse other digits */
      ret = parseDigits (pctxt, tmpbuf, &tmpidx, bufsize);
      if (0 != ret) return LOG_RTERR (pctxt, ret);
   }

   /* Check for floating point number */
   if (ch == '.') {
      if (tmpidx < bufsize) {
         needDigit = TRUE;
         tmpbuf[tmpidx++] = ch;
         pctxt->buffer.byteIndex++;
      }
      else return LOG_RTERR (pctxt, RTERR_TOOBIG);

      /* Parse digits */
      ret = parseDigits (pctxt, tmpbuf, &tmpidx, bufsize);
      if (0 != ret) return LOG_RTERR (pctxt, ret);

      if (OS_ISDIGIT(tmpbuf[tmpidx])) needDigit = FALSE;
   }

   /* Check for exponent */
   if ((ch == 'E' || ch == 'e') && !needDigit) {
      if (tmpidx < bufsize) {
         needDigit = TRUE;
         tmpbuf[tmpidx++] = ch;
         pctxt->buffer.byteIndex++;
      }
      else return LOG_RTERR (pctxt, RTERR_TOOBIG);

      ch = rtJsonDecPeekChar2 (pctxt);
      if (ch == '+' || ch == '-') {
         if (tmpidx < bufsize) {
            tmpbuf[tmpidx++] = ch;
            pctxt->buffer.byteIndex++;
         }
         else return LOG_RTERR (pctxt, RTERR_TOOBIG);
      }

      /* Parse digits */
      ret = parseDigits (pctxt, tmpbuf, &tmpidx, bufsize);
      if (0 != ret) return LOG_RTERR (pctxt, ret);

      if (OS_ISDIGIT(tmpbuf[tmpidx])) needDigit = FALSE;
   }

   tmpbuf[tmpidx++] = '\0';

   /* Allocate memory for number string and copy number */
   if (0 != ppCharStr) {
      *ppCharStr = rtxStrdup (pctxt, tmpbuf);
      if (0 == *ppCharStr) return LOG_RTERR (pctxt, RTERR_NOMEM);
   }

   return needDigit ? LOG_RTERR (pctxt, RTERR_INVFORMAT) : 0;
}
