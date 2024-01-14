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

#include "osrtjson.h"

int rtJsonEncHexValue (OSCTXT* pctxt, OSSIZE nocts, const OSOCTET* data)
{
   OSSIZE i, lbufx = 0;
   int  stat, ub;
   char lbuf[80];

   /* Verify hex chars will fix in encode buffer. Avoid integer overflow. */

   if (nocts > OSSIZE_MAX/2) return LOG_RTERR (pctxt, RTERR_TOOBIG);

   stat = rtxCheckOutputBuffer (pctxt, nocts*2);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   for (i = 0; i < nocts; i++) {
      if (lbufx >= sizeof(lbuf) - 2) {
         /* flush buffer */
         lbuf[lbufx] = '\0';
         stat = rtxCopyAsciiText (pctxt, lbuf);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
         lbufx = 0;
      }

      ub = (data[i] >> 4) & 0x0f;
      if ((pctxt->flags & OSUPCASE) != 0)
         NIBBLETOUCHEXCHAR (ub, lbuf[lbufx++]);
      else
         NIBBLETOHEXCHAR (ub, lbuf[lbufx++]);

      ub = (data[i] & 0x0f);
      if ((pctxt->flags & OSUPCASE) != 0)
         NIBBLETOUCHEXCHAR (ub, lbuf[lbufx++]);
      else
         NIBBLETOHEXCHAR (ub, lbuf[lbufx++]);
   }
   lbuf[lbufx] = '\0';

   /* flush remaining characters from local buffer */

   if (lbufx > 0) {
      stat = rtxCopyAsciiText (pctxt, lbuf);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}


int rtJsonEncHexStr (OSCTXT* pctxt, OSSIZE nocts, const OSOCTET* data)
{
   int  stat;

   OSRTSAFEPUTCHAR (pctxt, '"');

   stat = rtJsonEncHexValue(pctxt, nocts, data);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   OSRTSAFEPUTCHAR (pctxt, '"');

   return 0;
}
