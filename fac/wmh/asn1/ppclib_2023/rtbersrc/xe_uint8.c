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
 *  Routine name: xe_uint8
 *
 *  Description:  This routine encodes an 8-bit ASN.1 universal integer field
 *                from an unsigned integer input value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *  object      uchar*  Pointer to unsigned integer value to be encoded.
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

int xe_uint8 (OSCTXT* pctxt, OSUINT8 *object_p, ASN1TagType tagging)
{
   int aal = 1;

   if (0 == object_p) return LOG_RTERR(pctxt, RTERR_BADVALUE);

   XE_SAFEPUT1 (pctxt, *object_p);

   /* add leading zero byte if necessary */

   if (*object_p & 0x80) {
      XE_SAFEPUT1 (pctxt, 0L);
      aal++;
   }

   if (tagging == ASN1EXPL && aal > 0) {
      XE_CHKBUF (pctxt, 2);
      XE_PUT2 (pctxt, ASN_ID_INT, (OSOCTET)aal);
      aal += 2;
   }

   LCHECKBER (pctxt);

   return (aal);
}

