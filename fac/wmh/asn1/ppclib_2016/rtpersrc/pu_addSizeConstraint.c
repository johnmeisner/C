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

#include "rtpersrc/pu_common.hh"

/***********************************************************************
 *
 *  Routine name: pu_addSizeConstraint
 *
 *  Description:  This routine adds the given size constraint to
 *                the context structure.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct* ASN.1 context block structure
 *  psize       struct* Pointer to ASN.1 size constraint structure
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  status      int     Status of operation
 *
 **********************************************************************/

EXTPERMETHOD int pu_addSizeConstraint (OSCTXT* pctxt, const Asn1SizeCnst* pSize)
{
   if (0 == pSize) return 0;

   /* If constraint does not already exist, add it */

   if (ACINFO(pctxt)->sizeConstraint.root.upper == 0) {
      ACINFO(pctxt)->sizeConstraint.root = pSize->root;
      ACINFO(pctxt)->sizeConstraint.ext = pSize->ext;
   }

   /* Otherwise, check to make sure given constraint is larger than     */
   /* the existing constraint..                                         */

   else {
      if (ACINFO(pctxt)->sizeConstraint.root.lower < pSize->root.lower ||
          ACINFO(pctxt)->sizeConstraint.root.upper > pSize->root.upper) {
         return LOG_RTERR (pctxt, RTERR_CONSVIO);
      }

      if (ACINFO(pctxt)->sizeConstraint.ext.upper != 0) {
         if (ACINFO(pctxt)->sizeConstraint.ext.lower < pSize->ext.lower ||
             ACINFO(pctxt)->sizeConstraint.ext.upper > pSize->ext.upper) {
            return LOG_RTERR (pctxt, RTERR_CONSVIO);
         }
      }
   }

   return 0;
}
