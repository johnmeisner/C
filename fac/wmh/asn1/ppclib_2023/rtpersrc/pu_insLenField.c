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

#include "rtpersrc/asn1per.h"
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxStream.h"
#include "rtxsrc/rtxCharStr.h"

/***********************************************************************
 *
 *  Routine name: pu_insLenField
 *
 *  Description:  This routine inserts a special length field
 *                before the current record.
 *
 *  Inputs:
 *
 *  Name        Type     Description
 *  ----        ----     -----------
 *  pctxt      struct*  Pointer to PER context block
 *
 *  Outputs:
 *
 *  Name        Type     Description
 *  ----        ----     -----------
 *
 *
 **********************************************************************/

EXTPERMETHOD void pu_insLenField (OSCTXT* pctxt)
{
   PERField* pCurrField;
   PERField* pNewField;
   OSRTSListNode* pNode;

   if (((pctxt->flags & ASN1TRACE) != 0) && (0 != pctxt->pBitFldList)) {
      if ((pNode = pctxt->pBitFldList->fieldList.tail) != 0) {
         char*  tmpstr;
         size_t tmpstrSize;

         pCurrField = (PERField*) pNode->data;

         /* Create a new field and set name to that of current field    */

         pNewField = rtxMemAllocType (pctxt, PERField);
         if (pNewField == NULL) return;

         pNewField->elemName = pCurrField->elemName;

         /* Change name of current field to include 'length' suffix     */
         /* and set bit count..                                         */

         tmpstrSize = OSCRTLSTRLEN(pCurrField->elemName) + 8;
         tmpstr = (char*) rtxMemAlloc (pctxt, tmpstrSize);

         if (tmpstr == NULL) {
            rtxMemFreePtr (pctxt, pNewField);
            return;
         }

         rtxStrcpy (tmpstr, tmpstrSize, pNewField->elemName);
         rtxStrcat (tmpstr, tmpstrSize, " length");

         pCurrField->elemName = tmpstr;
         pCurrField->numbits = PU_GETCTXTBITOFFSET(pctxt);

#ifndef _NO_STREAM
         if (OSRTISSTREAM (pctxt))
            pCurrField->numbits += pctxt->pStream->bytesProcessed * 8;
#endif /* _NO_STREAM */

         pCurrField->numbits -= pCurrField->bitOffset;

         /* Append the new field to the field list and initialize the   */
         /* bit offset and count..                                      */

         pNewField->bitOffset = PU_GETCTXTBITOFFSET (pctxt);

#ifndef _NO_STREAM
         if (OSRTISSTREAM (pctxt))
            pNewField->bitOffset += pctxt->pStream->bytesProcessed * 8;
#endif /* _NO_STREAM */

         pNewField->numbits   = 0;
         pNewField->pOpenTypeFieldList = 0;

         rtxSListAppend (&pctxt->pBitFldList->fieldList, pNewField);
      }
   }
}
