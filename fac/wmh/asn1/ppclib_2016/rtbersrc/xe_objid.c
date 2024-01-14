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
#include "rtsrc/asn1ErrCodes.h"
#include "rtxsrc/rtxContext.hh"

/***********************************************************************
 *
 *  Routine name: xe_objid
 *
 *  Description:  This routine encodes the ASN.1 universal object ID
 *                field.
 *
 *                The "ASNOBJID" structure is used to pass the object
 *                identifier to this routine.  This structure allows the
 *                number of subidentifiers to be specified along with a
 *                dynamic array of integers which hold the actual sub-
 *                identifier values.
 *
 *                The routine copies all subidentifiers above the first
 *                two to the encode buffer as is.  The first two are then
 *                combined using the formula:
 *
 *                      (X * 40) + Y
 *
 *                where for X = 0 or 1, Y cannot be greater than 39.
 *                This combined identifer is then copied to the encode
 *                buffer.
 *
 *                If explicit tagging is specified, the routine then
 *                adds the universal tag.  The overall length of the
 *                encoded field is returned.
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

int xe_objid (OSCTXT* pctxt, ASN1OBJID *object_p, ASN1TagType tagging)
{
   OSUINT32 subid;
   register int	aal, ll, i;

   /* Validate given object ID by applying ASN.1 rules */

   if (0 == object_p) return LOG_RTERR (pctxt, ASN_E_INVOBJID);
   if (object_p->numids < 2) return LOG_RTERR (pctxt, ASN_E_INVOBJID);
   if (object_p->subid[0] > 2) return LOG_RTERR (pctxt, ASN_E_INVOBJID);
   if (object_p->subid[0] != 2 && object_p->subid[1] > 39)
      return LOG_RTERR (pctxt, ASN_E_INVOBJID);

   /* Passed checks, encode object identifier */

   for (aal = 0, i = object_p->numids - 1; i >= 1 && aal >= 0; i--)
   {
      subid = (i == 1) ?
	 (object_p->subid[0] * 40) + object_p->subid[1] : object_p->subid[i];

      aal = ((ll = xe_identifier (pctxt, subid)) >= 0) ? aal + ll : ll;
   }

   if (tagging == ASN1EXPL && aal > 0)
      aal = xe_tag_len (pctxt, TM_UNIV|TM_PRIM|ASN_ID_OBJID, aal);

   LCHECKBER (pctxt);

   return (aal);
}
