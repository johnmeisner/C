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

int xe_bitstr (OSCTXT* pctxt,
               const OSOCTET* object_p,
               OSSIZE numbits,
               ASN1TagType tagging)
{
   register int	aal, ll;
   OSSIZE       numbytes = numbits/8;
   OSOCTET	b;

   if (0 == numbits) {
      /* special case: to be compliant with DER, must encode 1 zero byte */
      b = 0;
      aal = xe_memcpy (pctxt, &b, 1);
   }
   else {
      if (object_p == NULL) return (0);

      if ((b = (OSOCTET) (numbits % 8)) != 0) {
         b = (OSOCTET) (8 - b);
         numbytes++;
      }

      aal = xe_memcpy (pctxt, object_p, numbytes);

      if (aal > 0)
         aal  = ((ll = xe_memcpy (pctxt, &b, 1)) > 0) ? aal + ll : ll;
   }

   if (tagging == ASN1EXPL && aal > 0)
      aal = xe_tag_len (pctxt, TM_UNIV|TM_PRIM|ASN_ID_BITSTR, aal);

   LCHECKBER (pctxt);

   return (aal);
}

int xe_bitstrExt(OSCTXT* pctxt, const OSOCTET* object_p,
                 OSSIZE numbits, OSSIZE dataSize,
                 const OSOCTET* extdata, ASN1TagType tagging)
{
   register int aal = 0, ll = 0;
   OSOCTET      b;
   OSSIZE       numbytes = numbits/8;

   if (0 == numbits) {
      /* special case: to be compliant with DER, must encode 1 zero byte */
      b = 0;
      aal = xe_memcpy (pctxt, &b, 1);
   }
   else {
      if (object_p == NULL) return (0);

      if ((b = (OSOCTET) (numbits % 8)) != 0) {
         b = (OSOCTET) (8 - b);
         numbytes++;
      }

      /* Encode extention data first, if present */
      if ((NULL != extdata) && (numbytes > dataSize))
      {
         aal = xe_memcpy (pctxt, extdata, numbytes - dataSize);
      }

      /* Now encode root data and length */
      aal = ((ll = xe_memcpy (pctxt, object_p, dataSize)) > 0) ? aal + ll : ll;

      if (aal > 0)
         aal  = ((ll = xe_memcpy (pctxt, &b, 1)) > 0) ? aal + ll : ll;
   }

   if (tagging == ASN1EXPL && aal > 0)
      aal = xe_tag_len (pctxt, TM_UNIV|TM_PRIM|ASN_ID_BITSTR, aal);

   LCHECKBER (pctxt);

   return (aal);
}

