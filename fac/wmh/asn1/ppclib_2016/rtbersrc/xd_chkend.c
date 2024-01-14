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
 *  Routine name: xd_chkend
 *
 *  Description:  This routine checks for the end of the context defined
 *                by the length variables that were last pushed on the
 *                stack using xd_push.
 *
 *  Inputs:
 *
 *  None
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  eoc		bool	True if end-of-context encountered.  Returned as
 *                      function result.
 *
 **********************************************************************/

OSBOOL xd_chkend64 (OSCTXT* pctxt, const OSOCTET* consptr,
                    OSSIZE conslen, OSBOOL indef)
{
   OSSIZE  len, usedBytes;
   ASN1TAG tag;
   OSBOOL  eoc, indefLen;
   int     stat;

   if (indef)
   {
      stat = xd_tag_len_64 (pctxt, &tag, &len, &indefLen, 0);

      if (stat == RTERR_ENDOFBUF) {
         rtxErrReset (pctxt);
         eoc = TRUE;
      }
      else if (tag == 0 && len == 0)
      {
         /* Advance cursor past EOC only if enveloping context  */
         /* parsed the indefinite length marker..               */
         eoc = TRUE;
      }
      else
         eoc = FALSE;
   }
   else
   {
      usedBytes = (OSSIZE)(OSRTBUFPTR(pctxt) - consptr);
      eoc = (OSBOOL)((usedBytes >= conslen) ||
         (pctxt->buffer.byteIndex >= pctxt->buffer.size));
   }

   return (eoc);
}

OSBOOL xd_chkend (OSCTXT* pctxt, const ASN1CCB* ccb_p)
{
   OSSIZE conslen;
   OSBOOL indefLen;

   if (ccb_p->len == ASN_K_INDEFLEN) {
      conslen = OSSIZE_MAX;
      indefLen = TRUE;
   }
   else {
      conslen = (OSSIZE)ccb_p->len;
      indefLen = FALSE;
   }

   return xd_chkend64 (pctxt, ccb_p->ptr, conslen, indefLen);
}
