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
#include "rtxsrc/rtxUtil.h"

EXTOERMETHOD int oerEncObjId64 (OSCTXT* pctxt, const ASN1OID64* pvalue)
{
   size_t i, nbytes;
   int stat;

   /* Validate given object ID by applying ASN.1 rules */

   if ((0 == pvalue) ||
       (pvalue->numids < 2) || (pvalue->subid[0] > 2) ||
       (pvalue->subid[0] != 2 && pvalue->subid[1] > 39))
      return LOG_RTERR (pctxt, ASN_E_INVOBJID);

   /* Passed checks, calculate nbytes for object identifier */

   nbytes = rtxGetIdent64ByteCount (pvalue->subid[0] * 40 + pvalue->subid[1]);
   for (i = 2; i < pvalue->numids; i++) {
      nbytes += rtxGetIdent64ByteCount (pvalue->subid[i]);
   }

   /* Encode object identifier */

   stat = oerEncLen (pctxt, nbytes);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* Munge first two sub ID's and encode */

   stat = oerEncIdent (pctxt, ((pvalue->subid[0] * 40) + pvalue->subid[1]));
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* Encode the remainder of the OID value */

   for (i = 2; i < pvalue->numids; i++) {
      stat = oerEncIdent (pctxt, pvalue->subid[i]);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}
