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
 *  Routine name: xd_int64
 *
 *  Description:  This routine decodes the integer value at the current
 *                message pointer location and returns the value.  It
 *                also advances the message pointer to the start of the
 *                the next field.  This version of the function stores
 *                the parsed integer value in a 64-bit integer variable.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *  tagging     enum    Specifies whether element is implicitly or
 *                      explicitly tagged.
 *  length      int     Length of data to retrieve.  Valid for implicit
 *                      case only.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat	int	Status of operation.  Returned as function result.
 *  object_p    int64*  Pointer to 64-bit integer to receive result.
 *
 **********************************************************************/

int xd_int64 (OSCTXT *pctxt, OSINT64 *object_p,
	      ASN1TagType tagging, int length)
{
   OSOCTET	ub;	/* unsigned */

   if (tagging == ASN1EXPL) {
      int stat;

      if (!XD_PEEKTAG (pctxt, ASN_ID_INT))
         return berErrUnexpTag (pctxt, ASN_ID_INT);

      stat = xd_Tag1AndLen (pctxt, &length);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   /* Make sure integer will fit in target variable */

   if (length > (int)(sizeof(OSINT64))) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }
   else if (length > 0) {
      int stat = XD_CHKDEFLEN (pctxt, length);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else
      return LOG_RTERR (pctxt, ASN_E_INVLEN);  /* note: indef len not allowed */

   /* Copy first byte into a signed char variable and assign it to	*/
   /* object.  This should handle sign extension in the case of a	*/
   /* negative number..							*/

   if (length > 0) {
      *object_p = (signed char)XD_FETCH1 (pctxt);;
      length--;
   }

   /* Now use unsigned bytes to add in the rest of the integer..	*/

   while (length > 0) {
      ub = XD_FETCH1 (pctxt);
      *object_p = (*object_p * 256) + ub;
      length--;
   }

   return (0);
}
