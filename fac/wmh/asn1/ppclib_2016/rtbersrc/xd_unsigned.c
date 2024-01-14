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
 *  Routine name: xd_unsigned
 *
 *  Description:  This routine decodes an integer ASN.1 value into
 *                a C unsigned integer data field.
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
 *  object      u_int   Decoded unsigned integer data value.
 *
 **********************************************************************/

int xd_unsigned
(OSCTXT *pctxt, OSUINT32 *object_p, ASN1TagType tagging, int length)
{
   register int	status = 0;
   OSOCTET      ub = 0;  /* unsigned */
   OSBOOL       negative = FALSE;

   if (tagging == ASN1EXPL) {
      if (!XD_MATCH1 (pctxt, ASN_ID_INT)) {
         return berErrUnexpTag (pctxt, ASN_ID_INT);
      }
      status = XD_LEN (pctxt, &length);
      if (status != 0) return LOG_RTERR (pctxt, status);
   }

   if (status == 0) {
      negative = ((ASN1BUFCUR(pctxt) & 0x80) != 0);

      /* Make sure integer will fit in target variable */
      if (length > (int)(sizeof(OSUINT32) + 1))
         return LOG_RTERR (pctxt, RTERR_TOOBIG);
      else if (length == (int)(sizeof(OSUINT32) + 1)) {
         /* first byte must be zero */
         if (0 != ASN1BUFCUR(pctxt))
            return LOG_RTERR (pctxt, RTERR_TOOBIG);
      }

      /* Use unsigned bytes to decode unsigned integer.. */

      *object_p = 0;

      while (length > 0 && status == 0) {
         if ((status = XD_MEMCPY1 (pctxt, &ub)) == 0) {
            *object_p = (*object_p * 256) + ub;
            length--;
         }
      }
   }

   if (status != 0) return LOG_RTERR (pctxt, status);

   if (negative) {
      OSINT32 signedValue = (OSINT32) *object_p;
      if (signedValue >= 0) signedValue = -signedValue;
      rtxErrAddStrParm (pctxt, "_uint32");
      rtxErrAddIntParm (pctxt, signedValue);
      return LOG_RTERRNEW (pctxt, RTERR_CONSVIO);
   }

   LCHECKBER (pctxt);

   return 0;
}
