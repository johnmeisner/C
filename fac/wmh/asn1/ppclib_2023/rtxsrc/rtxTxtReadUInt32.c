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
#include "rtxBuffer.h"
#include "rtxCtype.h"
#include "rtxError.h"
#include "rtxText.h"

static const OSUINT32 maxUInt32 = 0xFFFFFFFFu;
static const OSUINT32 maxUInt32_10 = 0x19999999u; /* 0x100000000/10 */

int rtxTxtReadUInt32 (OSCTXT* pctxt, OSUINT32* pvalue)
{
   register OSUINT32 value = 0;
   OSOCTET ub;
   int stat;

   *pvalue = 0;

   /* Consume leading white space */

   rtxTxtSkipWhitespace (pctxt);

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

         if (value > maxUInt32_10)
            return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);

         value *= 10;

         if (value > maxUInt32 - tm)
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

   *pvalue = value;

   return (0);
}
