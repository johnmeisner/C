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
 *  Routine name: xe_oid64
 *
 *  Description:  This routine encodes the ASN.1 universal object ID
 *                field.  This object identifier uses the C ASN1OID64
 *                type that can hold 64-bit arc values.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *  object      struct  Object ID to be encoded.
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

static int xe_ident64 (OSCTXT *pctxt, OSUINT64 ident);

int xe_oid64 (OSCTXT* pctxt, ASN1OID64 *object_p, ASN1TagType tagging)
{
   OSUINT64	subid;
   register int	aal, ll, i;

   /* Validate given object ID by applying ASN.1 rules */

   if (0 == object_p) return LOG_RTERR(pctxt, ASN_E_INVOBJID);
   if (object_p->numids < 2) return LOG_RTERR(pctxt, ASN_E_INVOBJID);
   if (object_p->subid[0] > 2) return LOG_RTERR(pctxt, ASN_E_INVOBJID);
   if (object_p->subid[0] != 2 && object_p->subid[1] > 39)
      return LOG_RTERR(pctxt, ASN_E_INVOBJID);

   /* Passed checks, encode object identifier */

   for (aal = 0, i = object_p->numids - 1; i >= 1 && aal >= 0; i--)
   {
      subid = (i == 1) ?
	 (object_p->subid[0] * 40) + object_p->subid[1] : object_p->subid[i];

      aal = ((ll = xe_ident64 (pctxt, subid)) >= 0) ? aal + ll : ll;
   }

   if (tagging == ASN1EXPL && aal > 0)
      aal = xe_tag_len (pctxt, TM_UNIV|TM_PRIM|ASN_ID_OBJID, aal);

   return (aal);
}

static int xe_ident64 (OSCTXT *pctxt, OSUINT64 ident)
{
   register int aal = 0, ll;
   OSOCTET b, first_byte = TRUE;

   do {
      b = (OSOCTET)
         ((first_byte) ? (ident % 128) : ((ident % 128) | 0x80));

      aal = ((ll = xe_memcpy (pctxt, &b, 1)) >= 0) ? aal + ll : ll;
      if (aal < 0) break;

      ident /= 128; first_byte = FALSE;
   }
   while (ident > 0);

   return (aal);
}
