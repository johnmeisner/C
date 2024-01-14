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

#include "rtbersrc/asn1ber.h"
#include "rtxsrc/rtxContext.hh"

/***********************************************************************
 *
 *  Routine name: xe_uint16
 *
 *  Description:  This routine encodes a 16-bit ASN.1 universal integer field
 *                from an unsigned integer input value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *  object      ushort* Pointer to unsigned integer value to be encoded.
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

int xe_uint16 (OSCTXT* pctxt, OSUINT16 *object_p, ASN1TagType tagging)
{
   register OSOCTET lb;
   register int aal = 1;
   register OSUINT32 temp;

   if (0 == object_p) return LOG_RTERR(pctxt, RTERR_BADVALUE);

   temp = *object_p;

   lb = (OSOCTET) (temp % 256);
   XE_SAFEPUT1 (pctxt, lb);

   temp /= 256;
   if (temp != 0) {
      lb = (OSOCTET) temp;
      XE_SAFEPUT1 (pctxt, lb);
      aal++;
   }

   /* If bit 8 of the leading byte is set, copy a zero byte to the      */
   /* contents to signal a positive number..                            */

   if (lb & 0x80) {
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
