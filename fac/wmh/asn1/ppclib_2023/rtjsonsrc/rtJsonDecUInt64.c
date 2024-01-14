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

static const OSUINT64 maxUInt64 = OSUI64CONST(0xFFFFFFFFFFFFFFFF);
static const OSUINT64 maxUInt64_10 = OSUI64CONST(0x1999999999999999);

int rtJsonDecUInt64Value (OSCTXT* pctxt, OSUINT64* pvalue)
{
   OSBOOL minus = FALSE;
   register OSUINT64 value = 0;
   int stat = 0;
   OSOCTET ub;

   if (pvalue)
      *pvalue = 0;

   /* Consume leading white space */

   rtJsonDecSkipWhitespace (pctxt);

   /* First character must be a + sign or digit */

   stat = rtxReadBytes (pctxt, &ub, 1);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   if (ub == '+') {
      /* Read next char */
      stat = rtxReadBytes (pctxt, &ub, 1);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   /* Read and process numeric digit characters.  End of input is assumed
      to be first non-numeric character */

   for (;;) {
      if (OS_ISDIGIT (ub)) {
         OSUINT32 tm = ub - '0';

         if (value > maxUInt64_10)
            return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);

         value *= 10;

         if (value > maxUInt64 - tm)
            return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);

            value += tm;
         }
         else if (minus && ub == '0') ; /* -0000... */
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

   if (stat >= 0) {
      if (pvalue)
         *pvalue = value;
   }

   return (0);
}
