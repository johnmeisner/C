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

#include "rtpersrc/asn1per.h"
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxStream.h"

/***********************************************************************
 *
 *  Routine name: pu_setFldBitCount
 *
 *  Description:
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct* PER context block structure
 *
 *  Outputs:
 *
 *  None
 *
 *
 **********************************************************************/

EXTPERMETHOD void pu_setFldBitCount (OSCTXT* pctxt)
{
   PERField* pfld;
   OSRTSListNode* pNode = ACINFO(pctxt)->fieldList.tail;

   if ((pctxt->flags & ASN1TRACE) != 0) {
      if (pNode) {
         pfld = (PERField*) pNode->data;
         if (pfld) {
            size_t pos = PU_GETCTXTBITOFFSET (pctxt);

#ifndef _NO_STREAM
            if (OSRTISSTREAM (pctxt))
               pos += pctxt->pStream->bytesProcessed * 8;
#endif /* _NO_STREAM */

            pfld->numbits = pos - pfld->bitOffset;
            RTDIAGSTRM4 (pctxt,"field '%s' bit count set to %d\n",
                    pfld->name, pfld->numbits);
         }
      }
   }
}