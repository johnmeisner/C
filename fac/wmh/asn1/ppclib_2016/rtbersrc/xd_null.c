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
 *  Routine name: xd_null
 *
 *  Description:  This routine decodes the null value at the current
 *                message pointer location.  Its main purpose is just to
 *                verify that an expected null placeholder is present,
 *                since the null identifier has no associated contents.
 *
 *                The routine calls xd_match to match the universal
 *                tag for this message type.  If the match is not
 *                successful, a not found status is returned to indicate
 *                the parse was not successful.
 *
 *                If the match is successful, the message pointer is
 *                advance to the next field.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  tagging     bool    Specifies whether element is implicitly or
 *                      explicitly tagged.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  status	int	Status of operation.  Returned as function result.
 *
 ***********************************************************************/
/*  CHANGE LOG */
/*  Date         Init    Description */
/*  09/07/02     ED      Performance improvements */
/*  */
/* ////////////////////////////////////////////////////////////////////////// */
int xd_null (OSCTXT *pctxt, ASN1TagType tagging)
{
   if (tagging == ASN1EXPL) {
      if (!XD_MATCH1 (pctxt, ASN_ID_NULL)) {
         return berErrUnexpTag (pctxt, ASN_ID_NULL);
      }

      /* Check length to make sure it is zero */
      if (XD_FETCH1 (pctxt) != 0) {
         return LOG_RTERR (pctxt, ASN_E_INVLEN);
      }
   }

   return (0);
}

