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

#include "xee_common.hh"
#include "rtxsrc/rtxBuffer.h"

#define NIBBLETOHEXCHAR_UC(b,ch) \
if (b >= 0 && b <= 9) ch = (char)(b + '0'); \
else if (b >= 0x0a && b <= 0x0f) ch = (char)((b - 10)+ 'A'); \
else ch = '?';

int xerEncHexStrValue (OSCTXT* pctxt, OSSIZE nocts, const OSOCTET* data)
{
   OSSIZE i, lbufx = 0;
   int  stat, ub;
   char lbuf[80];

   pctxt->state = XERDATA;

   for (i = 0; i < nocts; i++) {
      if (lbufx >= sizeof(lbuf)) {
         /* flush buffer */
         stat = rtxWriteBytes (pctxt, (OSOCTET*)lbuf, sizeof(lbuf));
         if (stat != 0) return LOG_RTERR (pctxt, stat);
         lbufx = 0;
      }

      ub = (data[i] >> 4) & 0x0f;
      NIBBLETOHEXCHAR_UC (ub, lbuf[lbufx++]);

      ub = (data[i] & 0x0f);
      NIBBLETOHEXCHAR_UC (ub, lbuf[lbufx++]);
   }

   /* flush remaining characters from local buffer */

   if (lbufx > 0) {
      stat = rtxWriteBytes (pctxt, (OSOCTET*)lbuf, lbufx);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}
