/*
 * Copyright (c) 2014-2023 Objective Systems, Inc.
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

/* Encode an OER unsigned integer.  This function can be used to encode
   1, 2, 4, and 8 byte signed integers. */

EXTOERMETHOD int oerEncUInt (OSCTXT* pctxt, OSUINT64 value, size_t size)
{
   size_t  tmpidx = size;
   OSUINT64 temp = value;
   OSOCTET lb, tmpbuf[8];
   int stat = 0;

   /* Only 1, 2, 4, and 8 byte sizes allowed */
   if (size != 1 && size != 2 && size != 4 && size != 8)
      return LOG_RTERR (pctxt, RTERR_INVPARAM);

   /* Encode value to temp buffer */
   do {
      lb = (OSOCTET) (temp % 256);
      temp /= 256;
      tmpbuf[--tmpidx] = lb;
   } while (temp != 0 && tmpidx > 0);

   /* Verify value could be encoded in given sized field */
   if (temp != 0) {
#ifndef _COMPACT
      rtxErrAddElemNameParm (pctxt);
      rtxErrAddInt64Parm (pctxt, value);
      return LOG_RTERR (pctxt, RTERR_CONSVIO);
#else
      return RTERR_CONSVIO;
#endif
   }
   /* Add padding */
   while (tmpidx > 0) {
      tmpbuf[--tmpidx] = 0;
   }

   /* Write to stream or memory buffer */
   stat = rtxWriteBytes (pctxt, tmpbuf, size);

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}
