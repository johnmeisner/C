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

#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxMemBuf.h"
#include "rtxsrc/rtxText.h"
#include "rtAvn.h"



EXTRTMETHOD int rtAvnReadString(OSCTXT* pctxt, OSUTF8CHAR** ppCharStr)
{
   /* Assume input is UTF-8, which is the only output format we support. */
   OSRTMEMBUF membuf;
   int stat, curCharOctet = 0, curCharLen = 0;
   OSSIZE nchars;
   OSOCTET ub;

   /* Skip whitespace and match opening double-quote. */
   stat = rtxTxtMatchChar(pctxt, '"', TRUE);
   if (0 != stat) return LOG_RTERR(pctxt, stat);

   rtxMemBufInit(pctxt, &membuf, 128);

   /* Consume characters until ending double-quote character (") is found. */

   for (;;) {
      stat = rtxReadBytes(pctxt, &ub, 1);
      if (0 != stat) return LOG_RTERR(pctxt, stat);

      if (curCharOctet == 0) { /* first octet */
         if (ub == '"') {
            if ('"' == rtxTxtPeekChar2(pctxt, FALSE))
            {
               /* This is an escaping double-quote.  Read the next char (2nd
                  double-quote) and then let it be added as a normal char. */
               stat = rtxReadBytes(pctxt, &ub, 1);
               if (0 != stat) return LOG_RTERR(pctxt, stat);
            }
            else {
               /* End of string. */
               break;
            }
         }
         else if (9 <= ub && ub <= 13)
         {
            /* New-line.  Discard prior whitespace, skip subsequent whitespace,
               and then continue to next byte. */
            OSOCTET* end;
            OSOCTET* start = OSMEMBUFPTR(&membuf);
            nchars = OSMEMBUFUSEDSIZE(&membuf);
            end = start + nchars;
            if (nchars > 0 && OS_ISSPACE(*end)) {
               /* There is some w/s to trim off. */
               OSSIZE cutbytes = 1; /* w/s to cut */
               while (end > start && OS_ISSPACE(*(end-1))) {
                  end--;
                  cutbytes++;
               }
               rtxMemBufCut(&membuf, end - start, cutbytes);
            }
            stat = rtxTxtSkipWhitespace(pctxt);
            if (0 != stat) return LOG_RTERR(pctxt, stat);

            continue;      /* Move on to next byte. */
         }

         if (ub < 0x80) {
            curCharLen = 1;
         }
         else if (ub < 0xE0) {
            curCharLen = 2;
         }
         else if (ub < 0xF0) {
            curCharLen = 3;
         }
         else if (ub < 0xF8) {
            curCharLen = 4;
         }
         else if (ub < 0xFC) {
            curCharLen = 5;
         }
         else if (ub < 0xFE) {
            curCharLen = 6;
         }
         else { /* invalid utf8 character */
            stat = LOG_RTERRNEW(pctxt, RTERR_INVUTF8);
            break;
         }
      }
      else if ((ub < 0x80) || (ub >= 0xC0)) {
         /* invalid utf8 character */
         stat = LOG_RTERRNEW(pctxt, RTERR_INVUTF8);
         break;
      }

      curCharOctet++;
      rtxMemBufAppend(&membuf, &ub, 1);
      if (curCharOctet == curCharLen) {
         curCharOctet = 0;
      }
   }

   nchars = OSMEMBUFUSEDSIZE(&membuf);
   *ppCharStr = rtxMemAlloc(pctxt, nchars + 1);
   OSCRTLSAFEMEMCPY(*ppCharStr, nchars, membuf.buffer, nchars);
   *(*ppCharStr + nchars) = 0;
   rtxMemBufFree(&membuf);

   return stat;
}


