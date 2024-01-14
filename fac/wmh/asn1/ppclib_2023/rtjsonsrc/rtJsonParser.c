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

#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxStream.h"
#include "rtxsrc/rtxUTF8.h"
#include "rtjsonsrc/osrtjson.h"

size_t rtJsonGetElemIdx
(OSCTXT* pctxt, const OSUTF8NameAndLen nameArray[], size_t nrows)
{
   OSUTF8CHAR* pToken = 0;
   OSSIZE i, parsedStrLen, retidx = OSNULLINDEX;
   int stat = 0;

   /* Parse token from input buffer or stream */
   stat = rtJsonDecStringValue (pctxt, &pToken);
   if (0 != stat) {
      LOG_RTERR (pctxt, stat);
      return OSNULLINDEX;
   }
   parsedStrLen = rtxUTF8LenBytes (pToken);

   rtxCtxtPushElemName (pctxt, pToken);

   /* Match : */
   stat = rtJsonDecMatchChar (pctxt, ':');
   if (0 != stat) { LOG_RTERR (pctxt, stat); }

   /* Loop through table rows looking for match */
   for (i = 0; i < nrows; i++) {
      if (parsedStrLen == nameArray[i].len &&
          rtxUTF8StrnEqual (pToken, nameArray[i].name, nameArray[i].len)) {
         retidx = i;
         break;
      }
   }

   return retidx;
}

int rtJsonDecMatchChar (OSCTXT* pctxt, OSUTF8CHAR ch)
{
   OSOCTET ub;
   int stat;

   /* Consume whitespace until character is found or EOF */
   for (;;) {
      stat = rtxReadBytes (pctxt, &ub, 1);
      /*
      RTDIAG4 (pctxt, "char '%c'(0x%x) read, buffer.byteIndex = %d\n",
               ub, ub, pctxt->buffer.byteIndex);
      */
      if (0 != stat) return LOG_RTERR (pctxt, stat);
      else if (ch == ub || !OS_ISSPACE (ub)) break;
   }

   if (ch != ub) {
      /* Push character back for reparse */
      pctxt->buffer.byteIndex--;
      /*
      RTDIAG4 (pctxt, "char '%c'(0x%x) at offset %d match failed!\n",
               ub, ub, pctxt->buffer.byteIndex);
      */
      /* Note: error should not be logged in this case because trial
         parse may be being done.  Caller can decide if error should
         be logged.. */
      return RTERR_INVCHAR;
   }
   else return 0;
}


int rtJsonDecMatchToken2
(OSCTXT* pctxt, const OSUTF8CHAR* token, size_t tokenLen)
{
   OSUTF8CHAR* pToken = 0;
   OSSIZE savedByteIndex = OSNULLINDEX;
   int stat;

   if (0 == token || tokenLen == 0)
      return LOG_RTERR (pctxt, RTERR_INVPARAM);

   if (tokenLen > pctxt->buffer.size) {
      rtxErrAddStrParm
         (pctxt, "Match on token larger than context buffer size");
      return LOG_RTERR (pctxt, RTERR_NOTSUPP);
   }

   /* Mark current position */
#ifndef _NO_STREAM
   if (OSRTISSTREAM(pctxt)) {
      stat = rtxStreamMark (pctxt, pctxt->buffer.size);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }
   else
#endif
      savedByteIndex = pctxt->buffer.byteIndex;

   /* Parse token from input buffer or stream */
   stat = rtJsonDecStringValue (pctxt, &pToken);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   /* Compare token to parsed string value */
   stat = (tokenLen == rtxUTF8LenBytes (pToken) &&
           rtxUTF8StrnEqual (pToken, token, tokenLen)) ? 0 : RTERR_IDNOTFOU;

   /* If not matched, reset buffer or stream to starting position */
   if (stat == RTERR_IDNOTFOU) {
#ifndef _NO_STREAM
      if (OSRTISSTREAM(pctxt)) {
         int stat2 = rtxStreamReset (pctxt);
         if (0 != stat2) {
            rtxMemFreePtr (pctxt, pToken);
            return LOG_RTERR (pctxt, stat2);
         }
      }
      else
#endif
         pctxt->buffer.byteIndex = savedByteIndex;
   }

   rtxMemFreePtr (pctxt, pToken);

   return stat;
}

int rtJsonDecMatchToken (OSCTXT* pctxt, const OSUTF8CHAR* token)
{
   return rtJsonDecMatchToken2 (pctxt, token, OSUTF8LEN(token));
}

int rtJsonDecMatchCharStr (OSCTXT* pctxt, const char* token)
{
   return rtJsonDecMatchToken (pctxt, (const OSUTF8CHAR*)token);
}

int rtJsonDecMatchObjectStart
(OSCTXT* pctxt, const OSUTF8NameAndLen* nameArray, size_t numNames)
{
   int stat;

   /* Match starting { char */
   stat = rtJsonDecMatchChar (pctxt, '{');

   /* Attempt to match name with name in nameArray */
   if (0 == stat) {
      size_t i = rtJsonGetElemIdx (pctxt, nameArray, numNames);
      if (i == OSNULLINDEX) stat = RTERR_IDNOTFOU;
   }

   return stat;
}


int rtJsonDecSkipWhitespaceAndComma (OSCTXT* pctxt)
{
   int stat;
   OSUTF8CHAR c;

   stat = rtJsonDecSkipWhitespace (pctxt);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   c = pctxt->buffer.data[pctxt->buffer.byteIndex];

   if (c == ',') {
      pctxt->buffer.byteIndex++;

      stat = rtJsonDecSkipWhitespace (pctxt);
      if (stat < 0) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}
