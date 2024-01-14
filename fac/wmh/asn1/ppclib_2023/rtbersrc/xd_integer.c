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
#include "rtxsrc/rtxContext.hh"

/***********************************************************************
 *
 *  Routine name: xd_integer
 *
 *  Description:  This routine decodes the integer value at the current
 *                message pointer location and returns the value.  It
 *                also advances the message pointer to the start of the
 *                the next field.
 *
 *                The routine first checks to see if explicit tagging
 *                is specified.  If yes, it calls xd_match to match
 *                the universal tag for this message type.  If the
 *                match is not successful, a negative value is returned
 *                to indicate the parse was not successful.
 *
 *                If the match is successful or implicit tagging is
 *                specified, xd_memcpy is called to fetch the
 *                data value.  This routine will take into account
 *                the overall message length to ensure the data is
 *                properly retrieved.
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
 *  object      int     Decoded integer data value.
 *
 **********************************************************************/

int xd_integer (OSCTXT *pctxt, OSINT32 *object_p,
                ASN1TagType tagging, int length)
{
   register int status = 0;
   OSOCTET first;
   OSOCTET	ub;	/* unsigned */

   if (tagging == ASN1EXPL) {
      if (!XD_MATCH1 (pctxt, ASN_ID_INT)) {
         return berErrUnexpTag (pctxt, ASN_ID_INT);
      }
      status = XD_LEN (pctxt, &length);
      if (status != 0) return LOG_RTERR (pctxt, status);
   }

   /* Make sure integer will fit in target variable */

   if (length > (int)(sizeof(OSINT32))) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }
   else if (length > 0) {
      status = XD_CHKDEFLEN (pctxt, length);
      if (status != 0) return LOG_RTERR (pctxt, status);
   }
   else
      return LOG_RTERR (pctxt, ASN_E_INVLEN);

   /* Copy first byte into a signed char variable and assign it to */
   /* object.  This should handle sign extension in the case of a  */
   /* negative number..                                            */

   if (length > 0) {
      first = XD_FETCH1 (pctxt);
      *object_p = (signed char) first;
      length--;

      /* Now use unsigned bytes to add in the rest of the integer */
      if (length > 0) {
         OSBOOL nonMinimal;
         ub = XD_FETCH1(pctxt);
         *object_p = (*object_p * 256) + ub;
         length--;

         nonMinimal = (first == 0xFF && (ub & 0x80) != 0) ||
                           (first == 0 && (ub & 0x80) == 0);
         if (nonMinimal && (pctxt->flags & (ASN1CANON | ASN1DER)) != 0)
         {
            rtxErrAddStrParm(pctxt, "integer does not use minimal octets");
            LOG_RTERRNEW(pctxt, ASN_E_NOTCANON);
            rtxErrSetNonFatal(pctxt);
         }
      }

      while (length > 0) {
         ub = XD_FETCH1 (pctxt);
         *object_p = (*object_p * 256) + ub;
         length--;
      }
   }

   LCHECKBER (pctxt);

   return 0;
}
