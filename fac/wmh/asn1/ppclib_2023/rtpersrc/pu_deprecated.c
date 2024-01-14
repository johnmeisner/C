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

/***********************************************************************
 *
 *  Routine name: pu_getBitOffset
 *
 *  Description:  This routine calculates the relative bit offset
 *                to the current buffer position.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct  PER context structure
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  offset      int     Bit offset (starting at 0) to buffer position
 *
 **********************************************************************/

EXTPERMETHOD int pu_getBitOffset (OSCTXT* pctxt)
{
   /* bit offset = (byte index * 8) + rel bit offset in current byte */
   int relBitOffset = 8 - pctxt->buffer.bitOffset;
   return (int)((pctxt->buffer.byteIndex * 8) + relBitOffset);
}

/* Set the bit offset to the given value within the context */

EXTPERMETHOD void pu_setBitOffset (OSCTXT* pctxt, int bitOffset)
{
   pctxt->buffer.byteIndex = (bitOffset / 8);
   pctxt->buffer.bitOffset = (OSUINT16)(8 - (bitOffset % 8));
}

/***********************************************************************
 *
 *  Routine name: pu_getSizeConstraint
 *
 *  Description:  This routine returns a pointer to a size constraint
 *                matching the given extendable attribute.
 *
 *                This function is now DEPRECATED as of version 6.3.
 *                Root and extension size constraints are now stored
 *                directly in the context.  This function will only
 *                return a pointer to the constraint in the context
 *                which contains root and extension parts.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct* ASN.1 context block structure
 *  extbit      bool    Value of extension bit
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  sizeCnst    struct* Pointer to matching constraint structure
 *
 *
 **********************************************************************/

EXTPERMETHOD
Asn1SizeCnst* pu_getSizeConstraint (OSCTXT* pctxt, OSBOOL extbit)
{
   OS_UNUSED_ARG(extbit);
   return &(ACINFO(pctxt)->sizeConstraint);
}

EXTPERMETHOD int pu_checkSizeConstraint (OSCTXT* pctxt, int size)
{
   OS_UNUSED_ARG(pctxt);
   OS_UNUSED_ARG(size);
   return 0;
}

