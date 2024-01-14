/*
 * Copyright (c) 2014-2018 Objective Systems, Inc.
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

#include "rtoersrc/asn1oer.h"
#include "rtxsrc/rtxBuffer.h"

/* Encode an OER unrestricted size type value */

EXTOERMETHOD int oerEncUnrestSize (OSCTXT* pctxt, OSSIZE value)
{
   OSOCTET tmpbuf[sizeof(OSSIZE) + 4], lb;
   OSSIZE  i = sizeof(OSSIZE) + 4;
   int     stat = 0;
   OSOCTET len = 0;

   do {
      lb = (OSOCTET) (value % 256);
      value /= 256;
      tmpbuf[--i] = lb;
      len++;
   } while (value != 0 && i > 0);

   tmpbuf[--i] = len;

   stat = rtxWriteBytes (pctxt, (OSOCTET*)&tmpbuf[i], len + 1);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}
