/*
 * Copyright (c) 2017-2023 Objective Systems, Inc.
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

int rtJsonDecHexStr
(OSCTXT* pctxt, OSOCTET* pvalue, OSUINT32* pnocts, size_t bufsize)
{
   OSSIZE nocts64;
   int stat = 0;

   stat = rtJsonDecHexStr64(pctxt, pvalue, &nocts64, bufsize);
   if (stat >= 0)
   {
      if ((sizeof(nocts64) > 4) && (nocts64 > OSUINT32_MAX))
      {
         return LOG_RTERR(pctxt, RTERR_TOOBIG);
      }
      else
      {
         if (pnocts)
         {
            *pnocts = (OSUINT32)nocts64;
         }
      }
   }

   return stat;
}

int rtJsonDecHexData64
(OSCTXT* pctxt, OSOCTET* pvalue, OSSIZE* pnocts, size_t bufsize)
{
   OSSIZE oldsize = 0;     /* number of hex digits (nibbles) read */
   int stat = 0;
   OSUTF8CHAR c;
   OSSIZE idx = 0;         /* index into pvalue for current nibble */

   if (pnocts) *pnocts = 0;

   if ( pvalue && bufsize == 0 ) return 0;   /* array already full */

   for (;;) {
      stat = rtxReadBytes (pctxt, &c, 1);
      if (stat == RTERR_ENDOFBUF) break;
      else if (0 != stat) return LOG_RTERR (pctxt, stat);

      if (c >= '0' && c <= '9')
         c -= '0';
      else if (c >= 'a' && c <= 'f')
         c -= 'a' - 10;
      else if (c >= 'A' && c <= 'F')
         c -= 'A' - 10;
      else {
         /* Push character back to be reread again */
         OSRTASSERT (pctxt->buffer.byteIndex > 0);
         pctxt->buffer.byteIndex--;
         break;
      }

      if (pvalue) {
         idx = oldsize / 2;
         if ((oldsize & 1) == 0)    /* even # of nibbles */
            pvalue[idx] = (OSOCTET) (c << 4);
         else {
            /* odd # of nibbles */
            pvalue[idx] |= c;
         }
      }

      if (stat < 0)
         return LOG_RTERR (pctxt, stat);

      oldsize++;

      if ( pvalue && idx == bufsize - 1 && oldsize % 2 == 0 )
      {
         /* We've filled the array */
         break;
      }
   }

   if (stat >= 0) {
      stat = 0;

      if (pvalue) {
         if ((oldsize & 1) != 0) {/* shift hex str (211 -> 0211) */
            OSUTF8CHAR* p = pvalue + oldsize / 2;

            for (;p != pvalue; p--) {
               *p = (OSOCTET) ((p[-1] << 4) | (*p >> 4));
            }

            *p >>= 4;
         }
      }

      if (pnocts) *pnocts = (oldsize + 1) / 2;
   }

   return stat;
}


int rtJsonDecHexStr64
(OSCTXT* pctxt, OSOCTET* pvalue, OSSIZE* pnocts, size_t bufsize)
{
   int stat = 0;
   OSSIZE nocts;

   rtJsonDecSkipWhitespace (pctxt);
   stat = rtJsonDecMatchChar (pctxt, '"');
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   if ( pnocts == 0 ) pnocts = &nocts;

   stat = rtJsonDecHexData64(pctxt, pvalue, pnocts, bufsize);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   if ( *pnocts == bufsize )
   {
      /* filled the buffer.  If next char is not ", we overflowed the buffer */
      OSUTF8CHAR c;
      stat = rtJsonDecPeekChar(pctxt, &c);
      if (0 != stat) return LOG_RTERR (pctxt, stat);

      if ( c != '"' ) return LOG_RTERRNEW (pctxt, RTERR_STROVFLW);
   }
   /* else: did not fill the buffer */

   stat = rtJsonDecMatchChar(pctxt, '"');
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   return stat;
}
