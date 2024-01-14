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
#if OS_ASN1RT_VERSION >= 600
#include "rtxsrc/rtxBuffer.h"
#endif

int xerPutCharStr (OSCTXT* pctxt, const char* value, size_t len)
{
   size_t lbufx, nchars, i;
   char lbuf[80];
   const char* sp;
   int  stat;

   /* Loop through all characters and convert special characters into   */
   /* XML escape sequences.                                             */

   for (i = 0, lbufx = 0; i < len; i++) {
      switch (value[i]) {

      case '<' : sp = "&lt;"; nchars = 4; break;
      case '>' : sp = "&gt;"; nchars = 4; break;
      case '&' : sp = "&amp;"; nchars = 5; break;
      case '\'' : sp = "&apos;"; nchars = 6; break;
      case '\"' : sp = "&quot;"; nchars = 6; break;

      default: sp = (const char*)&value[i]; nchars = 1; break;
      }

      if (lbufx + nchars > sizeof (lbuf)) {
         /* flush buffer */
         stat = rtxWriteBytes (pctxt, (OSOCTET*)lbuf, lbufx);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
         lbufx = 0;
      }

      if (1 == nchars) {
         lbuf[lbufx++] = *sp;
      }
      else {
         memcpy (&lbuf[lbufx], sp, nchars);
         lbufx += nchars;
      }
   }

   /* Flush remaining characters from local buffer */

   if (lbufx > 0) {
      stat = rtxWriteBytes (pctxt, (OSOCTET*)lbuf, lbufx);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   return 0;
}

