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
 *  Routine name: xd_Tag1AndLen
 *
 *  Description:  This routine is an optimized version of xd_tag_len.
 *                It assumes the tag value is a single byte and sets
 *                the ASN1CONSTAG flag in the context based on the
 *                form value.  It then skips the byte and decodes
 *                the length value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat	int	Status of operation.  Returned as function result.
 *  len_p       int     Length of message component.  Returned as follows:
 *			>= 0 		component is fixed length
 *			ASN_K_INDEFLEN	component is indefinite length
 *
 ***********************************************************************/

int xd_Tag1AndLen (OSCTXT *pctxt, int *len_p)
{
   int stat;
   OSUINT16 mask = ASN1INDEFLEN | ASN1LASTEOC;
   OSOCTET tagbyte;

   /* Check for attempt to read past EOB */

   if ((pctxt->flags & mask) == mask)
      return LOG_RTERR(pctxt, RTERR_ENDOFBUF);

   /* Set constructed tag bit in context flags based on tag value */

   tagbyte = XD_FETCH1 (pctxt);

   SET_ASN1CONSTAG_BYTE(pctxt, tagbyte);

   /* Decode length */

   stat = XD_LEN (pctxt, len_p);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}
