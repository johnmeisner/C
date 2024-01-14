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

/***********************************************************************
 *
 *  Routine name: xe_enum
 *
 *  Description:  This routine encodes the ASN.1 universal enumerated field.
 *                It adds the enumerated integer value by calling the
 *                integer encode function with implicit tagging specified.
 *                If explicit tagging is specified, it then adds the
 *                universal tag.  The overall length of the encoded field
 *                is returned.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *  object	enum	Enumerated value to be encoded.
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

int xe_enum (OSCTXT* pctxt, OSINT32 *object_p, ASN1TagType tagging)
{
   int aal;

   if (0 == object_p) return LOG_RTERR(pctxt, RTERR_BADVALUE);

   aal = xe_integer (pctxt, object_p, ASN1IMPL);

   if (tagging == ASN1EXPL && aal >= 0) {
      XE_CHKBUF (pctxt, 2);
      XE_PUT2 (pctxt, ASN_ID_ENUM, (OSOCTET)aal);
      aal += 2;
   }

   return (aal);
}



/***********************************************************************
 *
 *  Routine name: xe_enumUnsigned
 *
 *  Description:  This routine encodes the ASN.1 universal enumerated field.
 *                It adds the enumerated integer value by calling the
 *                integer encode function with implicit tagging specified.
 *                If explicit tagging is specified, it then adds the
 *                universal tag.  The overall length of the encoded field
 *                is returned.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *  object	enum	Enumerated value to be encoded.
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

int xe_enumUnsigned (OSCTXT* pctxt, OSUINT32 *object_p, ASN1TagType tagging)
{
   int aal;

   if (0 == object_p) return LOG_RTERR(pctxt, RTERR_BADVALUE);

   aal = xe_unsigned (pctxt, object_p, ASN1IMPL);

   if (tagging == ASN1EXPL && aal >= 0) {
      XE_CHKBUF (pctxt, 2);
      XE_PUT2 (pctxt, ASN_ID_ENUM, (OSOCTET)aal);
      aal += 2;
   }

   return (aal);
}
