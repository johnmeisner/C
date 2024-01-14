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

/***********************************************************************
 *
 *  Routine name: xe_int8
 *
 *  Description:  This routine encodes an 8-bit ASN.1 universal integer field.
 *                It adds the given integer value to the data buffer
 *                and then, if explicit tagging is specified, adds
 *                the universal tag.  The overall length of the encoded
 *                field is returned.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct  Pointer to ASN.1 context block structure
 *  object      char*   Pointer to integer value to be encoded.
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

int xe_int8 (OSCTXT* pctxt, OSINT8 *object_p, ASN1TagType tagging)
{
   int aal = 1;
   OSOCTET lb;

   if (0 == object_p) return LOG_RTERR(pctxt, RTERR_BADVALUE);

   lb = (OSOCTET) *object_p;
   XE_SAFEPUT1 (pctxt, lb);

   if (tagging == ASN1EXPL && aal > 0) {
      XE_CHKBUF (pctxt, 2);
      XE_PUT2 (pctxt, ASN_ID_INT, 1L);
      aal += 2;
   }

   LCHECKBER (pctxt);

   return (aal);
}

