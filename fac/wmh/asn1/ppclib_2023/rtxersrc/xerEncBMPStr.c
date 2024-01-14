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

int xerEncBMPStr (OSCTXT* pctxt,
                  const ASN1BMPString* pvalue,
                  const char* elemName)
{
   int  stat;
   OSSIZE inbufx, inlen = pvalue->nchars;
   OSUNICHAR wc;

   if (0 == elemName) elemName = "BMPString";

   /* Encode start element */

   stat = xerEncStartElement (pctxt, elemName, 0);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   pctxt->state = XERDATA;

   for (inbufx = 0; inbufx < inlen; inbufx++) {
      wc = pvalue->data[inbufx];

      stat = xerEncChar (pctxt, (OS32BITCHAR)wc);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   /* Encode end element */

   stat = xerEncEndElement (pctxt, elemName);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}
