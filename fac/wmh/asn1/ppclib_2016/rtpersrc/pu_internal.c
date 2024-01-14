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
 *  Routine name: pu_BitAndOctetStringAligmentTest
 *
 *  Description:  This routine test various conditions to determine
 *                if a bit or octet string should be aligned on a
 *                byte boundary.  It is assumed that aligned encoding
 *                is being used if this routine is invoked.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pSizeList   struct* Pointer to ASN.1 size constraint structure
 *                      from context.
 *  itemCount   int     Numbers of items (bits or octets) encoded or
 *                      parsed from message.
 *  bitStrFlag  bool    Flag indicating call is being made for a bit
 *                      string (false for octet string).
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  status      int     Status of operation.  Returned as function result.
 *  pAlignFlag  bool*   Flag indicating byte alignment should be
 *                      performed.
 *
 *
 **********************************************************************/

OSBOOL pu_BitAndOctetStringAlignmentTest
(const Asn1SizeCnst* pSizeCnst, OSSIZE itemCount, OSBOOL bitStrFlag)
{
   /* If type contains a size constraint */
   if (0 != pSizeCnst) {
      /* If item count > threshold, do alignment in all cases (X.691 16.10) */
      OSUINT32 threshold = (bitStrFlag) ? 16 : 2;
      if (itemCount > threshold) { return TRUE; }

      /* If constraint is extensible (X.691 16.6) */
      if (pSizeCnst->ext.upper > 0) {
         /* If item count is not within root, do alignment */
         if (itemCount < pSizeCnst->root.lower ||
             itemCount > pSizeCnst->root.upper) {
            return TRUE;
         }
         /* Otherwise, ignore extension */
      }
      /* Non-extensible size case: do not do alignment if item count
         is within threshold and upper bound == lower bound (16.9) */
      return (pSizeCnst->root.upper != pSizeCnst->root.lower);
   }
   else { /* Type does not contain a size constraint */
      return TRUE;
   }
}
