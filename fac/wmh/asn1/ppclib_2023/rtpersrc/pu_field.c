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
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxDiagBitTrace.h"
#include "rtxsrc/rtxStream.h"

/***********************************************************************
 *
 *  Routine name: pu_newField
 *
 *  Description:  This routine creates a new PER primitive field
 *                record and links it to the PER field list.
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

EXTPERMETHOD PERField* pu_newField (OSCTXT* pctxt, const char* nameSuffix)
{
#if 0
   PERField* pField = 0;

   if ((pctxt->flags & ASN1TRACE) != 0) {
#ifndef _NO_STREAM
      /* If context is using an input stream, set up capture buffer */
      if (OSRTISSTREAM (pctxt) && pctxt->pStream->pCaptureBuf == 0) {
         OSRTMEMBUF* pCaptureBuf = rtxMemAllocType (pctxt, OSRTMEMBUF);
         if (0 != pCaptureBuf) {
            rtxMemBufInit (pctxt, pCaptureBuf, 0);
            rtxStreamSetCapture (pctxt, pCaptureBuf);
         }
      }
#endif /* _NO_STREAM */

      pField = rtxMemAllocType (pctxt, PERField);
      if (pField == NULL) return NULL;

      pField->name = pu_getFullName (pctxt, nameSuffix);
      pField->bitOffset = PU_GETCTXTBITOFFSET (pctxt);
      pField->numbits = 0;
      pField->openTypeFields = 0;

#ifndef _NO_STREAM
      if (OSRTISSTREAM (pctxt))
         pField->bitOffset += pctxt->pStream->bytesProcessed * 8;
#endif /* _NO_STREAM */

      RTDIAGSTRM5 (pctxt,"pu_newField: name '%s', bitOffset %d, pField 0x%x\n",
                   pField->name, pField->bitOffset, pField);

      rtxSListAppend (&pctxt->pBitFldList->fieldList, pField);
   }

   return pField;
#else
   return rtxDiagNewBitField (pctxt->pBitFldList, nameSuffix);
#endif
}

EXTPERMETHOD void pu_initRtxDiagBitFieldList (OSCTXT* pctxt, OSINT16 bitOffset)
{
   rtxDiagCtxtBitFieldListInit (pctxt);

   /* If bit offset argument is not on a byte boundary, add a dummy
      record to the list in order to keep bit alignment right */
   if (bitOffset > 0 && bitOffset < 8 && !pctxt->buffer.aligned) {
      OSRTDiagBitField* pField = rtxMemAllocTypeZ (pctxt, OSRTDiagBitField);
      if (0 != pField) {
         pField->bitOffset = 8L - bitOffset;
         rtxSListAppend (&pctxt->pBitFldList->fieldList, pField);
      }
   }
}

EXTPERMETHOD void pu_popName (OSCTXT* pctxt)
{
   OS_UNUSED_ARG(pctxt);
   /*
   if ((pctxt->flags & ASN1TRACE) != 0)
      rtxStackPop (&(ACINFO(pctxt)->nameStack));
   */
}

EXTPERMETHOD void pu_pushName (OSCTXT* pctxt, const char* name)
{
   OS_UNUSED_ARG(pctxt);
   OS_UNUSED_ARG(name);
   /*
   if ((pctxt->flags & ASN1TRACE) != 0) {
      rtxStackPush (&(ACINFO(pctxt)->nameStack), (void*)name);
   }
   */
}

EXTPERMETHOD void pu_pushElemName (OSCTXT* pctxt, int idx)
{
   OS_UNUSED_ARG(pctxt);
   OS_UNUSED_ARG(idx);
   /*
   if ((pctxt->flags & ASN1TRACE) != 0) {
      char lbuf[32], *pName;
      if (rtxIntToCharStr (idx, lbuf, sizeof(lbuf), 0) < 0) {
         lbuf[0] = '?'; lbuf[1] = '\0';
      }
      pName = (char*) rtxStrDynJoin (pctxt, "elem[", lbuf, "]", 0, 0);
      if (pName != 0) {
         rtxStackPush (&(ACINFO(pctxt)->nameStack), pName);
      }
   }
   */
}

/*
EXTPERMETHOD void pu_setOpenTypeFldList (OSCTXT* pctxt, OSRTSList* plist)
{
   if ((pctxt->flags & ASN1TRACE) != 0) {
      OSRTSListNode* pNode = pctxt->pBitFldList->fieldList.tail;
      if (0 != pNode) {
         PERField* pfld = (PERField*) pNode->data;
         if (0 != pfld) {
            pfld->openTypeFields = plist;
         }
      }
   }
}
*/

EXTPERMETHOD void pu_setRtxDiagOpenTypeFldList
(OSRTDiagBitFieldList* pMainBFList, OSRTDiagBitFieldList* pOpenTypeBFList)
{
   if (0 != pMainBFList) {
      OSRTSListNode* pNode = pMainBFList->fieldList.tail;
      if (pMainBFList->disabledCount == 0 && 0 != pNode) {
         OSRTDiagBitField* pfld = (OSRTDiagBitField*) pNode->data;
         if (0 != pfld) {
            pfld->pOpenTypeFieldList = pOpenTypeBFList;
         }
      }
   }
}

EXTPERMETHOD void pu_deleteFieldList (OSCTXT* pctxt)
{
   OSRTSListNode* pFieldNode;

   if (0 == pctxt->pBitFldList) return;

   pFieldNode = pctxt->pBitFldList->fieldList.head;
   while (0 != pFieldNode) {
      PERField* pField = (PERField*) pFieldNode->data;
      rtxMemFreePtr (pctxt, (void*)pField->elemName);
      pFieldNode = pFieldNode->next;
   }

   rtxSListFreeAll (&pctxt->pBitFldList->fieldList);
   pctxt->pBitFldList->pLastPrinted = 0;
}
