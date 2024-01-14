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
 *  Routine name: xd_boolean
 *
 *  Description:  This routine decodes the boolean value at the current
 *                message pointer location and returns the value.  It
 *                also advances the message pointer to the start of the
 *                the next field.
 *
 *                The routine first checks to see if explicit tagging
 *                is specified.  If yes, the universal tag for this
 *                message type is checked to make sure it is of the
 *                expected value.  If the match is not successful, a
 *                negative value is returned to indicate the parse was
 *                not successful.  Otherwise, the pointer is advanced
 *                to the length field and the length parsed.
 *
 *                The length value is then check to see if it is equal
 *                to one which is the only valid length for boolean.
 *                If it is equal, the boolean data value is parsed;
 *                otherwise, and error is returned.
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
 *  object      bool    Decoded boolean data value.
 *
 ***********************************************************************/
/*  CHANGE LOG */
/*  Date         Init    Description */
/*  09/07/02     ED      Performance improvements */
/*  */
/* ////////////////////////////////////////////////////////////////////////// */
int xd_boolean (OSCTXT *pctxt, OSBOOL *object_p,
                ASN1TagType tagging, int length)
{
   if (tagging == ASN1EXPL) {
      if (!XD_MATCH1 (pctxt, ASN_ID_BOOL)) {
         return berErrUnexpTag (pctxt, ASN_ID_BOOL);
      }
      length = XD_FETCH1 (pctxt);
   }

   if (length == 1) {
      *object_p = XD_FETCH1 (pctxt);
      if (*object_p != 0 && *object_p != 0xFF &&
         (rtxCtxtTestFlag(pctxt, ASN1DER) || rtxCtxtTestFlag(pctxt, ASN1CANON)))
      {
         /* CER/DER require 0xFF for true */
         rtxErrAddStrParm(pctxt, "must use 0xFF for BOOLEAN true");
         LOG_RTERRNEW(pctxt, ASN_E_NOTCANON);
         rtxErrSetNonFatal(pctxt);
      }
   }
   else
      return LOG_RTERR (pctxt, ASN_E_INVLEN);

   LCHECKBER (pctxt);

   return 0;
}

