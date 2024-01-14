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

/* Decode an OER 16-bit signed integer */

EXTOERMETHOD int oerDecInt16 (OSCTXT* pctxt, OSINT16* pvalue)
{
   OSOCTET tmpbuf[2];

   int stat = rtxReadBytes (pctxt, tmpbuf, 2);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   if (0 != pvalue) {
      *pvalue = (signed char) tmpbuf[0];
      *pvalue = (*pvalue * 256) + tmpbuf[1];
   }

   return 0;
}

/* Decode an OER 32-bit signed integer */

EXTOERMETHOD int oerDecInt32 (OSCTXT* pctxt, OSINT32* pvalue)
{
   OSOCTET tmpbuf[4];

   int stat = rtxReadBytes (pctxt, tmpbuf, 4);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   if (0 != pvalue) {
      int i;
      *pvalue = (signed char) tmpbuf[0];
      for (i = 1; i < 4; i++) {
         *pvalue = (*pvalue * 256) + tmpbuf[i];
      }
   }

   return 0;
}

