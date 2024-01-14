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

int xerEncBinStrValue (OSCTXT* pctxt, OSSIZE nbits, const OSOCTET* data)
{
   OSSIZE i;
   unsigned char mask = 0x80;
   OSSIZE lbufx = 0;
   int  stat;
   char lbuf[40];

   pctxt->state = XERDATA;

   for (i = 0; i < nbits; i++) {
      if (lbufx >= sizeof(lbuf)) {
         /* flush buffer */
         stat = rtxWriteBytes (pctxt, (OSOCTET*)lbuf, sizeof(lbuf));
         if (stat != 0) return LOG_RTERR (pctxt, stat);
         lbufx = 0;
      }
      lbuf[lbufx++] = ((char)(((data[i/8] & mask) != 0) ? '1' : '0'));
      mask >>= 1;
      if (0 == mask) mask = 0x80;
   }

   /* flush remaining characters from local buffer */

   if (lbufx > 0) {
      stat = rtxWriteBytes (pctxt, (OSOCTET*)lbuf, lbufx);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}

