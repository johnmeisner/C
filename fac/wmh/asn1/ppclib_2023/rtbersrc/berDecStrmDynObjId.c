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

#include "rtbersrc/asn1berStream.h"
#include "rtxsrc/rtxContext.hh"

int berDecStrmDynObjId
(OSCTXT *pctxt, ASN1DynOBJID *object_p, ASN1TagType tagging, int length)
{
   OSUINT32 subids[ASN_K_MAXSUBIDS];
   int stat = 0;

   if (0 == pctxt || 0 == object_p) return RTERR_NULLPTR;

   if (tagging == ASN1EXPL) {
      stat = berDecStrmMatchTag (pctxt, ASN_ID_OBJID, &length, TRUE);
      /* RTERR_IDNOTFOU will be logged later, by the generated code,
         or reset by rtxErrReset (for optional seq elements). */
      if (stat != 0)
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   if (length <= 0)
      return LOG_RTERR (pctxt, ASN_E_INVLEN); /* note: indef len not allowed */

   /* Parse OID contents */
   stat = berDecStrmOIDSubIds(pctxt, subids, ASN_K_MAXSUBIDS, length);
   if (stat < 0) return LOG_RTERR(pctxt, stat);
   else object_p->numids = (OSUINT16)stat;

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
