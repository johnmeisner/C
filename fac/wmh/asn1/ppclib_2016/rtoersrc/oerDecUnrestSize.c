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

int oerDecUnrestSize (OSCTXT* pctxt, OSSIZE* pvalue)
{
   register int stat = 0, retval = 0;
   OSOCTET  tmpbuf[16];
   OSUINT32 bufidx = 0;
   OSOCTET  ub;     /* unsigned */
   OSOCTET  len;

   /* Length will be short form (a single byte) */

   stat = rtxReadBytes (pctxt, &len, 1);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   /* Make sure integer will fit in target variable */

   if (len > (sizeof(OSSIZE))) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }

   /* Read encoded integer contents into memory */

   stat = rtxReadBytes (pctxt, tmpbuf, len);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   /* Decode integer contents */

   *pvalue = 0;

   while (len > 0) {
      if ( bufidx == 0 && tmpbuf[0] == 0 && len > 1 )
      {
         /* This is the first byte, it is zero, and more bytes are to
            follow.  This is not in canonical form.
         */
         retval = ASN_E_NOTCANON;
      }

      ub = tmpbuf[bufidx++];
      *pvalue = (*pvalue * 256) + ub;
      len--;
   }

   return retval;
}
