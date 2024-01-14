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

#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxError.h"
#include "rtjsonsrc/osrtjson.h"

static const OSINT32 maxInt32 = 0x7FFFFFFF;
static const OSINT32 maxInt32_10 = 0xCCCCCCC; /* 0x80000000/10 */

int rtJsonDecInt32Value (OSCTXT* pctxt, OSINT32* pvalue)
{
   int stat;
   register OSINT32 value = 0;
   OSINT32 maxval = maxInt32;
   OSBOOL  minus = FALSE;
   OSOCTET ub;

   rtJsonDecSkipWhitespace (pctxt);

   /* First character must be a +, -, or digit */
   stat = rtxReadBytes (pctxt, &ub, 1);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   if (ub == '+' || ub == '-') {
      if (ub == '-') {
         minus = TRUE;
         maxval++; /* support for 0x80000000 */
      }
      /* Read next char */
      stat = rtxReadBytes (pctxt, &ub, 1);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   /* First character after +/- must be a numeric digit */
   if (!OS_ISDIGIT (ub)) return LOG_RTERR (pctxt, RTERR_INVCHAR);

   /* Read and process numeric digit characters.  End of input is assumed
      to be first non-numeric character */
   for (;;) {
      if (OS_ISDIGIT (ub)) {
         OSINT32 tm = ub - '0';

         if (value > maxInt32_10)
            return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);

         value *= 10;

         if ((OSUINT32)value > (OSUINT32)(maxval - tm))
            return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);

         value += tm;
      }
      else {
         /* Push character back to be reread again */
         OSRTASSERT (pctxt->buffer.byteIndex > 0);
         pctxt->buffer.byteIndex--;
         break;
      }

      stat = rtxReadBytes (pctxt, &ub, 1);
      if (stat == RTERR_ENDOFBUF) break;
      else if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   /* If negative, negate number */

   if (minus) *pvalue = 0 - value;
   else *pvalue = value;

   return (0);
}
