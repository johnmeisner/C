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

#include "osMacros.h"
#include "rtxBuffer.h"
#include "rtxCtype.h"
#include "rtxDiag.h"
#include "rtxError.h"
#include "rtxStream.h"
#include "rtxUTF8.h"
#include "rtxText.h"


int rtxTxtMatchChar(OSCTXT* pctxt, OSUTF8CHAR ch, OSBOOL skipWs)
{
   OSOCTET ub;
   int stat;

   /* Consume whitespace until character is found or EOF */
   for (;;) {
      stat = rtxReadBytes(pctxt, &ub, 1);
      if (0 != stat) return LOG_RTERR(pctxt, stat);

      if (!skipWs) break;
      else if (ch == ub || !OS_ISSPACE(ub)) break;
   }

   if (ch != ub) {
      char badchar = ub;
      rtxErrAddStrnParm(pctxt, &badchar, 1);
      return LOG_RTERRNEW(pctxt, RTERR_INVCHAR);
   }
   else return 0;
}


EXTRTMETHOD int rtxTxtMatchChars(OSCTXT* pctxt, const OSUTF8CHAR* chars,
                                    OSBOOL skipWs)
{
   OSOCTET ub;
   OSUTF8CHAR ch;
   int stat;

   if ( chars == 0 ) return LOG_RTERR(pctxt, RTERR_INVPARAM);

   ch = *chars;
   if (ch==0) return 0; /* empty string matched */

   /* Consume whitespace until character is found or EOF */
   for (;;) {
      stat = rtxReadBytes(pctxt, &ub, 1);
      if (0 != stat) return LOG_RTERR(pctxt, stat);

      if (!skipWs) break;
      else if (ch == ub || !OS_ISSPACE(ub)) break;
   }

   /* ub is first non-whitespace char.  Now try to match it and subsequent
      characters in chars. */
   for (;;)
   {
      if (ch != ub) {
         char badchar = ub;
         rtxErrAddStrnParm(pctxt, &badchar, 1);
         return LOG_RTERRNEW(pctxt, RTERR_INVCHAR);
      }

      /* Move to next char in chars. */
      chars++;
      ch = *chars;
      if (ch==0) break;

      /* Read next byte and loop around to test it. */
      stat = rtxReadBytes(pctxt, &ub, 1);
      if (0 != stat) return LOG_RTERR(pctxt, stat);
   }

   return 0;
}


EXTRTMETHOD int rtxTxtPeekChar(OSCTXT* pctxt, OSUTF8CHAR* pch, OSBOOL skipWs)
{
   OSOCTET ub;
   int stat;

   if (skipWs) {
      stat = rtxTxtSkipWhitespace(pctxt);
      if (0 != stat) return LOG_RTERR(pctxt, stat);
   }

   stat = rtxPeekByte(pctxt, &ub);
   if (stat < 0) return LOG_RTERR(pctxt, stat);
   else if (stat == 0) return LOG_RTERRNEW(pctxt, RTERR_ENDOFBUF);

   *pch = ub;
   return 0;
}


EXTRTMETHOD char rtxTxtPeekChar2(OSCTXT* pctxt, OSBOOL skipWs)
{
   OSOCTET ub;
   int stat;

   stat = rtxTxtPeekChar(pctxt, &ub, skipWs);
   if (0 != stat) return 0;      /* error already logged */
   else return ub;
}



EXTRTMETHOD int rtxTxtSkipWhitespace(OSCTXT* pctxt)
{
   OSOCTET ub;
   int stat;

   /* Consume whitespace until non-space character is found or EOF */
   for (;;) {
      stat = rtxPeekByte(pctxt, &ub);
      if (stat == 0) break; /* EOF */
      else if (stat == 1) {
         if (OS_ISSPACE(ub)) {
            pctxt->buffer.byteIndex++;
         }
         else break;
      }
      else return LOG_RTERR(pctxt, stat);
   }

   return 0;
}
