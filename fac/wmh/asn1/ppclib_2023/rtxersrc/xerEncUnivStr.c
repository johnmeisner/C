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

int xerEncUnivStr (OSCTXT* pctxt,
                   const ASN1UniversalString* pvalue,
                   const char* elemName)
{
   int  stat;
   OSSIZE inbufx, inlen = pvalue->nchars;
   OS32BITCHAR wc;

   if (0 == elemName) elemName = "UniversalString";

   /* Encode start element */

   stat = xerEncStartElement (pctxt, elemName, 0);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   pctxt->state = XERDATA;

   for (inbufx = 0; inbufx < inlen; inbufx++) {
      wc = pvalue->data[inbufx];

      if (wc >> 31) { /* equivavlent of 'wc < 0 || wc > 0x7fffffff' */
         /* This is not a correct ISO 10646 character */
#ifndef _COMPACT
         rtxErrAddSizeParm (pctxt, inbufx);
#endif
         return LOG_RTERR (pctxt, RTERR_INVUTF8);/* invalid encoding error */
      }

      stat = xerEncChar (pctxt, wc);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   /* Encode end element */

   stat = xerEncEndElement (pctxt, elemName);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}
