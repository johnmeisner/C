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

#include "rtpersrc/asn1per.h"
#include "rtxsrc/rtxContext.hh"

/***********************************************************************
 *
 *  Routine name: pd_ObjectIdentifier
 *
 *  Description:  The following function decodes an object identifier
 *                value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pdata       objid*  pointer to OID value to receive decoded data
 *  stat        int     completion status of encode operation
 *
 **********************************************************************/

EXTPERMETHOD int pd_DynObjId (OSCTXT* pctxt, ASN1DynOBJID* pvalue)
{
   OSUINT32 len;
   int      stat;
   OSUINT32 subids[ASN_K_MAXSUBIDS];

   /* Decode unconstrained length */

   if ((stat = pd_Length (pctxt, &len)) < 0) {
      return LOG_RTERR (pctxt, stat);
   }

   PU_NEWFIELD (pctxt, "ObjectID");

   if (pvalue) {
      /* Decode subidentifiers into local array */
      stat = rtDecOIDSubIds(pctxt, subids, ASN_K_MAXSUBIDS, len);
      if (stat < 0) return LOG_RTERR(pctxt, stat);
      else pvalue->numids = (OSUINT16)stat;

      /* Allocate dynamic array for subidentifiers */
      pvalue->pSubIds = rtxMemAllocArray(pctxt, pvalue->numids, OSUINT32);
      if (0 == pvalue->pSubIds) return RTERR_NOMEM;
      pvalue->memAllocated = TRUE;

      /* Copy subids from temp array to target variable */
      {
         OSUINT16 i;
         for (i = 0; i < pvalue->numids; i++) {
            pvalue->pSubIds[i] = subids[i];
         }
      }
   }
   else {
      stat = rtxSkipBytes(pctxt, len);
   }

   PU_SETBITCOUNT(pctxt);

   LCHECKPER (pctxt);

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}
