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
 *  Routine name: xe_boolean
 *
 *  Description:  This routine encodes the ASN.1 universal boolean field.
 *                It adds the given boolean data byte to the data buffer
 *                and then, if explicit tagging is specified, adds
 *                the universal tag.  The overall length of the encoded
 *                field is returned.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *  object      bool    Boolean value to be encoded.
 *  tagging     bool    Explicit or implicit tagging specification.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  aal         int     Overall message length.  Returned as function
 *                      result.  Will be negative if encoding fails.
 *
 **********************************************************************/

int xe_boolean (OSCTXT* pctxt, OSBOOL *object_p, ASN1TagType tagging)
{
   OSOCTET ub;
   int aal = 1;

   if (0 == object_p) return LOG_RTERR(pctxt, RTERR_BADVALUE);

   ub = (*object_p == 0) ? 0 : 0xff;
   if (pctxt->flags & ASN1BOOLTRUE1 && ub != 0) ub = 0x01;

   XE_CHKBUF (pctxt, 1);
   XE_PUT1 (pctxt, ub);

   if (tagging == ASN1EXPL) {
      XE_CHKBUF (pctxt, 2);
      XE_PUT2 (pctxt, ASN_ID_BOOL, 1L);
      aal += 2;
   }

   LCHECKBER (pctxt);

   return aal;
}

