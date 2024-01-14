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

#include "xed_common.hh"

int xerDecDynBitStr (OSCTXT* pctxt, ASN1DynBitStr* pvalue)
{
   XMLCHAR* inpdata = (XMLCHAR*) ASN1BUFPTR (pctxt);
   int i, nbits = 0, stat;
   OSUINT32 bufsize;
   OSOCTET* data;

   /* binary case */

   /* Count bits */
   for (i = 0; inpdata[i] != 0; i++) {
      char c = (char)inpdata[i];
      if (!OS_ISSPACE (c)) {
         if (c != '0' && c != '1')
            return LOG_RTERR (pctxt, ASN_E_INVBINS);
         nbits++;
      }
   }

   bufsize = (nbits + 7) / 8;
   data = (OSOCTET*) rtxMemAlloc (pctxt, bufsize);
   if (data != 0) {

      /* Convert Unicode characters to a bit string value */
      stat = xerDecBinStrValue (pctxt, data, 0, bufsize, TRUE);
      if (stat != 0) return LOG_RTERR_AND_FREE_MEM (pctxt, stat, data);

      pvalue->data = data;
      pvalue->numbits = nbits;
   }
   else
      return LOG_RTERR (pctxt, RTERR_NOMEM);

   return 0;
}

