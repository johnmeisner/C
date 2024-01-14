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

EXTOERMETHOD int oerDecObjId (OSCTXT* pctxt, ASN1OBJID* pvalue)
{
   size_t length;
   int stat;

   stat = oerDecLen (pctxt, &length);
   if (0 != stat) {
      LOG_RTERR (pctxt, stat);
      /* If "not canonical" error, continue decoding */
      if (stat != ASN_E_NOTCANON) return stat;
   }

   if (length == 0)
      return LOG_RTERR (pctxt, ASN_E_INVLEN);

   /* Parse OID contents */

   stat = rtDecOIDSubIds(pctxt, pvalue->subid, ASN_K_MAXSUBIDS, (int)length);
   if (stat < 0) return LOG_RTERR(pctxt, stat);
   else pvalue->numids = (OSUINT32)stat;

   return 0;
}

EXTOERMETHOD int oerDecDynObjId (OSCTXT* pctxt, ASN1DynOBJID* pvalue)
{
   ASN1OBJID oidval;
   int stat = oerDecObjId(pctxt, &oidval);
   if (0 != stat) return LOG_RTERR(pctxt, stat);

   /* Allocate dynamic array for subidentifiers */
   pvalue->numids = (OSUINT16)oidval.numids;
   pvalue->pSubIds = rtxMemAllocArray(pctxt, pvalue->numids, OSUINT32);
   if (0 == pvalue->pSubIds) return RTERR_NOMEM;
   pvalue->memAllocated = TRUE;

   /* Copy subids from temp array to target variable */
   {
      OSUINT16 i;
      for (i = 0; i < pvalue->numids; i++) {
         pvalue->pSubIds[i] = oidval.subid[i];
      }
   }

   return 0;
}

