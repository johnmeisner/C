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
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxContext.hh"

/***********************************************************************
 *
 *  Routine name: xd_bigint
 *
 *  Description:  This routine decodes an integer ASN.1 value into
 *                a hexadecimal character string.
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
 *  object      char*   Decoded big integer data value.
 *
 **********************************************************************/

int xd_bigint (OSCTXT *pctxt, const char** object_p,
               ASN1TagType tagging, int length)
{
   int bufsiz, i, ub, off = 2;
   char* tmpstr;
   OSBOOL leadingZeros = FALSE;

   if (tagging == ASN1EXPL) {
      int status;

      if (!XD_MATCH1 (pctxt, ASN_ID_INT)) {
         return berErrUnexpTag (pctxt, ASN_ID_INT);
      }
      status = XD_LEN (pctxt, &length);
      if (status != 0) return LOG_RTERR (pctxt, status);
   }

   if ((pctxt->buffer.byteIndex + length) > pctxt->buffer.size)
      return LOG_RTERR (pctxt, RTERR_ENDOFBUF);

   /* check is the first byte zero */

   if (length > 0 && ASN1BUFCUR(pctxt) == 0) {
      leadingZeros = TRUE;
      XD_BUMPIDX(pctxt, 1);         /* skip.., */
      length--;
   }

   /* skip all remained leading zeros */

   while (length > 0 && ASN1BUFCUR(pctxt) == 0) {
      XD_BUMPIDX(pctxt, 1);
      length--;
   }

   bufsiz = (length * 2) + 4;
   tmpstr = (char*) rtxMemAlloc (pctxt, bufsiz);
   if (tmpstr != 0) {
      tmpstr[0] = '0';
      tmpstr[1] = 'x';

      if (length == 0 || (leadingZeros && (ASN1BUFCUR(pctxt) & 0x80)))
         tmpstr [off++] = '0';

      for (i = 0; i < length; i++) {
         OSOCTET oct = ASN1BUFCUR(pctxt);

         ub = (oct >> 4) & 0x0f;
         NIBBLETOHEXCHAR (ub, tmpstr[off++]);

         ub = (oct & 0x0f);
         NIBBLETOHEXCHAR (ub, tmpstr[off++]);

         XD_BUMPIDX(pctxt, 1);
      }

      tmpstr[off] = '\0';
      *object_p = tmpstr;
   }
   else
      return LOG_RTERR (pctxt, RTERR_NOMEM);

   LCHECKBER (pctxt);

   return 0;
}
