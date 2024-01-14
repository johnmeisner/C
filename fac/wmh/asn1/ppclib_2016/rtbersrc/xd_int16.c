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
 * This function parses an ASN.1 INTEGER tag/length/value at the
 * current message pointer location and advances the pointer to
 * the next field.
 *
 * This function is similar to ::xd_integer but it is used to parse 16-bit
 * integer values.
 *
 *  @param pctxt     Pointer to context block structure.
 *  @param tagging    Specifies whether element is implicitly or
 *                      explicitly tagged.
 *  @param length     Length of data to retrieve.  Valid for implicit
 *                      case only.
 *  @param object_p   Decoded boolean data value.
 *  @return           Completion status of operation:
 *                      - 0 (0) = success,
 *                      - negative return value is error.
 *
 **********************************************************************/

int xd_int16 (OSCTXT *pctxt, OSINT16 *object_p,
              ASN1TagType tagging, int length)
{
   register int status = 0;
   signed char	b;	/* signed   */
   OSOCTET	ub;	/* unsigned */

   if (tagging == ASN1EXPL) {
      if ((status = XD_CHKDEFLEN (pctxt, 2)) != 0) /* tag + len */
         return LOG_RTERR (pctxt, status);

      if (!XD_MATCH1 (pctxt, ASN_ID_INT)) {
         return berErrUnexpTag (pctxt, ASN_ID_INT);
      }
      length = (int) XD_FETCH1 (pctxt);
   }

   /* Make sure integer will fit in target variable */

   if (length > (int)(sizeof(OSINT16))) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }
   else if (length <= 0) {
      return LOG_RTERR (pctxt, ASN_E_INVLEN);  /* note: indef len not allowed */
   }
   status = XD_CHKDEFLEN (pctxt, length);
   if (status != 0) return LOG_RTERR (pctxt, status);

   /* Copy first byte into a signed char variable and assign it to	*/
   /* object.  This should handle sign extension in the case of a	*/
   /* negative number..                                                 */

   b = XD_FETCH1 (pctxt);

   if (length > 1) {
      ub = XD_FETCH1 (pctxt);
      *object_p = (((OSINT16)b) * 256) + ub;
   }
   else
      *object_p = b;

   return 0;
}
