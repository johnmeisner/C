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

#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxMemBuf.h"
#include "osrtjson.h"

/* This file depends on predefined constants defined in corresponding
 * source files:
 *
 * OSJSONUNIDECFUNC (e.g., rtJsonDecUCS2String)
 * OSJSONUNICHAR    (e.g., OSUNICHAR or OS32BITCHAR) */

EXTJSONMETHOD int OSJSONUNIDECFUNC (OSCTXT *pctxt, OSJSONUNICHAR **ppstr,
      OSSIZE *pnchars)
{
   /* for now we assume that the input JSON is encoded in UTF-8, which is the
      only output format we support at the moment. */

   OSRTMEMBUF membuf;
   int stat, curCharOctet = 0, curCharLen = 0, nchars = 0;
   OSOCTET ub;
   OSJSONUNICHAR curUniChar;

   if (ppstr != 0) {
      *ppstr = 0;
   }
   if (pnchars != 0) {
      *pnchars = 0;
   }

   /* Consume whitespace until starting double-quote character (") is found */
   stat = rtJsonDecMatchChar (pctxt, '"');
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   /* If JSON attribute flag is set, match '@' as next character.
      Do not include this character in decoded content. */
   if (rtxCtxtTestFlag (pctxt, OSJSONATTR)) {
      rtxCtxtClearFlag (pctxt, OSJSONATTR);

      stat = rtJsonDecMatchChar (pctxt, '@');
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   rtxMemBufInit (pctxt, &membuf, 128);

   /* Consume characters until ending double-quote character (") is found */
   for (;;) {
      stat = rtxReadBytes (pctxt, &ub, 1);
      if (0 != stat) return LOG_RTERR (pctxt, stat);

      if (ub == '"') {
         break;
      }

      if (curCharOctet == 0) { /* first octet */
         if (ub < 0x80) {
            curCharLen = 1;
            curUniChar = (OSJSONUNICHAR) (ub & 0x7F);
         }
         else if (ub < 0xE0) {
            curCharLen = 2;
            curUniChar = (OSJSONUNICHAR) (ub & 0x1F);
         }
         else if (ub < 0xF0) {
            curCharLen = 3;
            curUniChar = (OSJSONUNICHAR) (ub & 0xF);
         }
         else if (ub < 0xF8) {
            curCharLen = 4;
            curUniChar = (OSJSONUNICHAR) (ub & 0x7);
         }
         else if (ub < 0xFC) {
            curCharLen = 5;
            curUniChar = (OSJSONUNICHAR) (ub & 0x3);
         }
         else if (ub < 0xFE) {
            curCharLen = 6;
            curUniChar = (OSJSONUNICHAR) (ub & 0x1);
         }
         else { /* invalid utf8 character */
            stat = LOG_RTERRNEW (pctxt, RTERR_INVUTF8);
            break;
         }

         if ((curCharLen > 3) && (sizeof (OSJSONUNICHAR) < 4)) {
            /* character not fit to UCS-2 */
            stat = LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
            break;
         }
      }
      else if ((ub >= 0x80) && (ub < 0xC0)) { /* next octet */
         curUniChar = (OSJSONUNICHAR) ((curUniChar << 6) | (ub & 0x3F));
      }
      else { /* invalid utf8 character */
         stat = LOG_RTERRNEW (pctxt, RTERR_INVUTF8);
         break;
      }

      curCharOctet++;

      if (curCharOctet == curCharLen) {
         rtxMemBufAppend (&membuf, (OSOCTET *)(&curUniChar),
               sizeof(OSJSONUNICHAR));
         curCharOctet = 0;
         nchars ++;
      }
   }

   *ppstr = (OSJSONUNICHAR *)rtxMemAlloc(pctxt, nchars*sizeof(OSJSONUNICHAR));
   OSCRTLSAFEMEMCPY (*ppstr, nchars*sizeof(OSJSONUNICHAR), membuf.buffer,
         nchars*sizeof(OSJSONUNICHAR));
   *pnchars = nchars;

   rtxMemBufFree (&membuf);

   return stat;
}
