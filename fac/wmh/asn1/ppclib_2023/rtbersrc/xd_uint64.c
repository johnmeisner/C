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
 *  Routine name: xd_uint64
 *
 *  Description:  This routine decodes the unsigned int value at the current
 *                message pointer location and returns the value.  It
 *                also advances the message pointer to the start of the
 *                the next field.  This version of the function stores
 *                the parsed integer value in a 64-bit unsigned int variable.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct  Pointer to ASN.1 context block structure
 *  tagging     enum    Specifies whether element is implicitly or
 *                      explicitly tagged.
 *  length      int     Length of data to retrieve.  Valid for implicit
 *                      case only.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     Status of operation.  Returned as function result.
 *  object_p    uint64* Pointer to 64-bit integer to receive result.
 *
 **********************************************************************/

int xd_uint64 (OSCTXT *pctxt, OSUINT64 *object_p,
               ASN1TagType tagging, int length)
{
   OSOCTET      ub;     /* unsigned */
   int          stat;
   OSBOOL       negative;

   if (tagging == ASN1EXPL) {
      if (!XD_PEEKTAG (pctxt, ASN_ID_INT))
         return berErrUnexpTag (pctxt, ASN_ID_INT);

      stat = xd_Tag1AndLen (pctxt, &length);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   /* if the length is zero, return 0 */
   if (length == 0) {
      *object_p = 0;
      return 0;
   }

   /* Verify that encoded value is not negative */

   negative = ((ASN1BUFCUR(pctxt) & 0x80) != 0);

   /* Make sure integer will fit in target variable */

   if (length > (int)(sizeof(OSUINT64) + 1))
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   else if (length == (int)(sizeof(OSUINT64) + 1)) {

      /* first byte must be zero */

      if (0 != ASN1BUFCUR(pctxt))
         return LOG_RTERR (pctxt, RTERR_TOOBIG);

      XD_BUMPIDX (pctxt, 1); /* skip it */

      if ((stat = XD_CHKDEFLEN (pctxt, 1)) != 0)
         return LOG_RTERR (pctxt, stat);

      length--;
   }

   /* Use unsigned bytes to decode unsigned integer.. */

   *object_p = 0;

   while (length > 0) {
      ub = XD_FETCH1 (pctxt);
      *object_p = (*object_p * 256) + ub;
      length--;
   }

   if (negative) {
      OSINT64 signedValue = (OSINT64) *object_p;
      if (signedValue >= 0) signedValue = -signedValue;
      rtxErrAddStrParm (pctxt, "_uint64");
      rtxErrAddInt64Parm (pctxt, signedValue);
      return LOG_RTERRNEW (pctxt, RTERR_CONSVIO);
   }

   return 0;
}
