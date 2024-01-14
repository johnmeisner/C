/*
 * Copyright (c) 1997-2023 Objective Systems, Inc.
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

#include "rtbersrc/asn1ber.h"
#include "rtxsrc/rtxContext.hh"

int xd_dynObjId
(OSCTXT *pctxt, ASN1DynOBJID *object_p, ASN1TagType tagging, int length)
{
   int status;
   OSUINT32 subids[ASN_K_MAXSUBIDS];

   if (tagging == ASN1EXPL) {
      if (!XD_MATCH1 (pctxt, ASN_ID_OBJID)) {
         return berErrUnexpTag (pctxt, ASN_ID_OBJID);
      }

      status = XD_LEN (pctxt, &length);
      if (status != 0) return LOG_RTERR (pctxt, status);
   }

   /* Check length */

   if (length > 0) {
      status = XD_CHKDEFLEN (pctxt, length);
      if (status != 0) return LOG_RTERR (pctxt, status);
   }
   else
      return LOG_RTERR (pctxt, ASN_E_INVLEN);

   /* Parse OID contents */

   status = rtDecOIDSubIds(pctxt, subids, ASN_K_MAXSUBIDS, length);
   if (status < 0) return LOG_RTERR(pctxt, status);
   else object_p->numids = (OSUINT16)status;

   /* Allocate dynamic array for subidentifiers */

   object_p->pSubIds = rtxMemAllocArray(pctxt, object_p->numids, OSUINT32);
   if (0 == object_p->pSubIds) return RTERR_NOMEM;
   object_p->memAllocated = TRUE;

   /* Copy subids from temp array to target variable */
   {
      OSUINT16 i;
      for (i = 0; i < object_p->numids; i++) {
         object_p->pSubIds[i] = subids[i];
      }
   }
   LCHECKBER (pctxt);

   return 0;
}
