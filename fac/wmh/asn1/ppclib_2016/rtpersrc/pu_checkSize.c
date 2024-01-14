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
 *  Routine name: pu_checkSize
 *
 *  Description:  This routine checks the given size against the size
 *                constraint list.  It will return the constraint the
 *                value falls within.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct* ASN.1 context block structure
 *  value       int     Size value
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  sizeCnst    struct* Pointer to matching constraint structure
 *  extendable  bool*   Indicates size is extendable
 *
 **********************************************************************/

EXTPERMETHOD OSBOOL pu_isFixedSize (const Asn1SizeCnst* pSizeCnst)
{
   if (0 != pSizeCnst && 0 == pSizeCnst->ext.upper) {
      return (OSBOOL) (pSizeCnst->root.lower == pSizeCnst->root.upper);
   }
   else return FALSE;
}

EXTPERMETHOD Asn1SizeCnst* pu_checkSize
(Asn1SizeCnst* pSizeCnst, OSUINT32 value, OSBOOL* pExtendable)
{
   if (0 != pSizeCnst) {
      if (0 != pExtendable) {
         *pExtendable = (OSBOOL)(pSizeCnst->ext.upper > 0);
      }
      /* note: does not distinguish whether size falls within root
         or extension area */
      if (value >= pSizeCnst->root.lower && value <= pSizeCnst->root.upper) {
         return (pSizeCnst);
      }
      else if (value >= pSizeCnst->ext.lower &&
               value <= pSizeCnst->ext.upper) {
         return (pSizeCnst);
      }
   }
   return 0;
}

EXTPERMETHOD int pu_checkSizeExt
(Asn1SizeCnst* pSizeCnst, OSSIZE value, OSBOOL* pExtendable,
 Asn1SizeValueRange* pSizeRange, OSBOOL* pExtSize)
{
   if (0 == pSizeCnst) {
      return RTERR_INVPARAM;
   }
   else {
      OSBOOL extendable = (OSBOOL)(pSizeCnst->ext.upper > 0);
      if (0 != pExtendable) { *pExtendable = extendable; }

      if (pSizeCnst->root.upper > 0) { /* has root constraint */
         if (value >= pSizeCnst->root.lower &&
             value <= pSizeCnst->root.upper) {
            if (0 != pSizeRange) {
               pSizeRange->lower = pSizeCnst->root.lower;
               pSizeRange->upper = pSizeCnst->root.upper;
            }
            if (0 != pExtSize) *pExtSize = FALSE;
         }
         else if (extendable) {
            if (value >= pSizeCnst->ext.lower &&
                value <= pSizeCnst->ext.upper) {
               if (0 != pSizeRange) {
                  pSizeRange->lower = pSizeCnst->ext.lower;
                  pSizeRange->upper = pSizeCnst->ext.upper;
               }
               if (0 != pExtSize) *pExtSize = TRUE;
            }
            else {
               return RTERR_CONSVIO;
            }
         }
         else {
            return RTERR_CONSVIO;
         }
      }
      else if (extendable) { /* extendable size with no root */
         if (value >= pSizeCnst->ext.lower &&
             value <= pSizeCnst->ext.upper) {
            if (0 != pSizeRange) {
               pSizeRange->lower = pSizeCnst->ext.lower;
               pSizeRange->upper = pSizeCnst->ext.upper;
            }
            if (0 != pExtSize) *pExtSize = TRUE;
         }
         else {
            return RTERR_CONSVIO;
         }
      }
      else if (0 != pSizeRange) {
         pSizeRange->lower = 0;
         pSizeRange->upper = OSSIZE_MAX;
      }
   }

   return 0;
}
