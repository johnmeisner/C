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

/* Decode an OER BMP string into a dynamic buffer */

EXTOERMETHOD int oerDecBMPStr
(OSCTXT* pctxt, ASN1BMPString* pvalue)
{
   int stat = 0, retval = 0, len = 0, i;
   OSUNICHAR* data = 0;

   stat = oerDecLen (pctxt, &len);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   if (len == 0) {
      if (0 != pvalue) {
         pvalue->nchars = 0;
         pvalue->data = 0;
      }
      return retval;
   }
   
   /* If the decoded length is not a multiple of the appropriate size,
      return an error.  */
      
   if ((len % sizeof (OSUNICHAR)) != 0) return ASN_E_INVLEN;

   /* Decode contents */
   data = rtxMemAlloc (pctxt, len);
   if (0 == data) return LOG_RTERR (pctxt, RTERR_NOMEM);
   for (i = 0; i < (len / sizeof (OSUNICHAR)) ; i++) {
      stat = oerDecInt16 (pctxt, &data[i]);
      if (stat < 0) return LOG_RTERR_AND_FREE_MEM (pctxt, stat, data);
   }
   pvalue->nchars = (len / sizeof (OSUNICHAR));
   pvalue->data = data;
   
   return retval;
}
