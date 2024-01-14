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

/***********************************************************************
 *
 *  Routine name: xe_bitstr
 *
 *  Description:  This routine encodes the ASN.1 universal bit string
 *                field. It adds the given bit string value to the
 *                data buffer and then, if explicit tagging is specified,
 *                adds the universal tag.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *  object      struct  Bit string to be encoded.
 *  numbits	int	Number of bits in the bit string
 *  tagging     enum    Explicit or implicit tagging specification.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  aal         int     Overall message length.  Returned as function
 *                      result.  Will be negative if encoding fails.
 *
 **********************************************************************/

int derEncBitString
(OSCTXT* pctxt, const OSOCTET* pvalue, OSSIZE numbits, ASN1TagType tagging)
{
   /* Adjust bit count to remove trailing zero bits */
   while (numbits > 0) {
      if (!rtxTestBit (pvalue, numbits, numbits - 1))
         numbits--;
      else
         break;
   }
   return xe_bitstr (pctxt, pvalue, numbits, tagging);
}
