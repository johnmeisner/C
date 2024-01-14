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
 *  Routine name: xe_octstr
 *
 *  Description:  This routine encodes the ASN.1 universal octet (byte)
 *                string field. It adds the given string value to the
 *                data buffer and then, if explicit tagging is specified,
 *                adds the universal tag.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *  object      char*   Octet string to be encoded.
 *  numocts	int	Number of octets in the octet string.
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

int xe_octstr (OSCTXT* pctxt,
               const OSOCTET* object_p,
               OSSIZE numocts,
               ASN1TagType tagging)
{
   int aal = (0 == object_p) ? 0 : xe_memcpy (pctxt, object_p, numocts);

   if (tagging == ASN1EXPL && aal >= 0)
      aal = xe_tag_len (pctxt, TM_UNIV|TM_PRIM|ASN_ID_OCTSTR, aal);

   LCHECKBER (pctxt);

   return (aal);
}

