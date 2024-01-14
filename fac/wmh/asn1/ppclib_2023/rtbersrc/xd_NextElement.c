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
 *  Routine name: xd_NextElement
 *
 *  Description:  This routine will advance the decode pointer
 *                to the next element within a constructed type.
 *
 *  Inputs:
 *
 *  Name        Type                Description
 *  ----        ---------           -----------
 *  pctxt      OSCTXT*           Context block pointer
 *
 *  Outputs:
 *
 *  Name        Type                Description
 *  ----        ---------           -----------
 *  status      int	            Status of operation.
 *                                  Returned as function result.
 *
 **********************************************************************/

int xd_NextElement (OSCTXT* pctxt)
{
   ASN1TAG tag;
   int len, stat;

   stat = xd_tag_len (pctxt, &tag, &len, XM_ADVANCE);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   if (len == ASN_K_INDEFLEN) {
      stat = xd_MovePastEOC (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else {
      pctxt->buffer.byteIndex += len;
   }

   return 0;
}
