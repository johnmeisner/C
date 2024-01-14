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
#include "rtsrc/asn1intl.h"

/***********************************************************************
 *
 *  Routine name: xd_OpenTypeAppend
 *
 *  Description:  This routine decodes an ASN.1 open type field.
 *                An open type is assumed to be a previously encoded
 *                value which is to be represented in its native
 *                form in the returned C structure.  The user can then
 *                apply subsequent decode routine calls to decode the
 *                value.
 *
 *                This routine stores the current decode pointer in
 *                the C structure and moves on to the next field.
 *
 *  Inputs:
 *
 *  None
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  status      int	Status of operation.  Returned as function result.
 *  object	struct* Structure containing tag, length, and data pointer.
 *
 **********************************************************************/

int xd_OpenTypeAppend (OSCTXT *pctxt, OSRTDList *pElemList)
{
   int stat;

   if (0 != pElemList) {
      ASN1OpenType* pOpenType = rtxMemAllocType (pctxt, ASN1OpenType);
      if (pOpenType == NULL) return LOG_RTERR (pctxt, RTERR_NOMEM);

      stat = xd_OpenType (pctxt, &pOpenType->data, &pOpenType->numocts);

      if (stat != 0)
         return LOG_RTERR_AND_FREE_MEM (pctxt, stat, pOpenType);
      else
         rtxDListAppend (pctxt, pElemList, pOpenType);
   }
   else {
      stat = xd_NextElement (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}
