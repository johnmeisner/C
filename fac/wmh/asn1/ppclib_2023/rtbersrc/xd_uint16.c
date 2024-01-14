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
 * This function parses an unsigned variant of ASN.1 INTEGER
 * tag/length/value at the current message pointer location and advances
 * the pointer to the next field.
 *
 * This function is similar to ::xd_unsigned but it is used to parse 16-bit
 * unsigned integer values.
 *
 *  @param pctxt     Pointer to context block structure.
 *  @param tagging    Specifies whether element is implicitly or
 *                      explicitly tagged.
 *  @param length     Length of data to retrieve.  Valid for implicit
 *                      case only.
 *  @param object_p   Decoded integer data value.
 *  @return           Completion status of operation:
 *                      - 0 (0) = success,
 *                      - negative return value is error.
 *
 **********************************************************************/

int xd_uint16 (OSCTXT *pctxt, OSUINT16 *object_p, ASN1TagType tagging,
   int length)
{
   register int status = 0;
   OSOCTET      ub; /* unsigned */
   OSBOOL       negative;

   if (tagging == ASN1EXPL) {
      if ((status = XD_CHKDEFLEN (pctxt, 3)) != 0) /* tag + len + 1b val */
         return LOG_RTERR (pctxt, status);

      if (!XD_MATCH1 (pctxt, ASN_ID_INT)) {
         return berErrUnexpTag (pctxt, ASN_ID_INT);
      }
      length = (int) XD_FETCH1 (pctxt);
   }

   negative = ((ASN1BUFCUR(pctxt) & 0x80) != 0);

   /* Make sure integer will fit in target variable */
   if (length > (int)(sizeof(OSUINT16) + 1))
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   else if (length == (int)(sizeof(OSUINT16) + 1)) {
      /* first byte must be zero */
      if (0 != ASN1BUFCUR(pctxt))
         return LOG_RTERR (pctxt, RTERR_TOOBIG);

      XD_BUMPIDX (pctxt, 1); /* skip it */

      if ((status = XD_CHKDEFLEN (pctxt, 1)) != 0)
         return LOG_RTERR (pctxt, status);

      length--;
   }

   /* Use unsigned bytes to decode unsigned integer.. */

   ub = XD_FETCH1 (pctxt);

   if (length > 1) {
      *object_p = (OSUINT16) ((ub * 256) + XD_FETCH1 (pctxt));
   }
   else
      *object_p = ub;

   if (negative) {
      OSINT32 signedValue = (OSINT32) *object_p;
      if (signedValue >= 0) signedValue = -signedValue;
      rtxErrAddStrParm (pctxt, "_uint16");
      rtxErrAddIntParm (pctxt, signedValue);
      return LOG_RTERRNEW (pctxt, RTERR_CONSVIO);
   }

   LCHECKBER (pctxt);

   return 0;
}
