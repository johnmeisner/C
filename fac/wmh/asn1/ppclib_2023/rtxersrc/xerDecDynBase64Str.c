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

int xerDecDynBase64Str (OSCTXT* pctxt, ASN1DynOctStr* pvalue)
{
   XMLCHAR* inpdata = (XMLCHAR*) ASN1BUFPTR (pctxt);
   int stat, nocts = 0, encLen;
   OSOCTET* data;

   /* Count octets */
   encLen = xerTextLength (inpdata);
   nocts = xerGetBase64StrDecodedLen (pctxt, encLen);
   if (nocts < 0) return LOG_RTERR (pctxt, nocts);

   pvalue->numocts = nocts;
   data = (OSOCTET*) rtxMemAlloc (pctxt, nocts);
   if (data != 0) {
      pvalue->data = data;

      stat = xerDecBase64StrValue (pctxt, data, 0, nocts, encLen);
      if (stat != 0) return LOG_RTERR_AND_FREE_MEM (pctxt, stat, data);
   }
   else
      return LOG_RTERR (pctxt, RTERR_NOMEM);

   return 0;
}

