/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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

#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxDiagBitTrace.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxMemBuf.h"
#include "rtxsrc/rtxStream.h"

EXTRTMETHOD int rtxDiagCtxtBitFieldListInit (OSCTXT* pctxt)
{
#ifdef _TRACE
   pctxt->pBitFldList = rtxMemAllocType (pctxt, OSRTDiagBitFieldList);
   if (0 != pctxt->pBitFldList) {
      rtxDiagBitFieldListInit (pctxt, pctxt->pBitFldList);
      return 0;
   }
   else return LOG_RTERR (pctxt, RTERR_NOMEM);
#else
   OS_UNUSED_ARG(pctxt);
   return 0;
#endif
}

EXTRTMETHOD void rtxDiagBitFieldListInit
(OSCTXT* pctxt, OSRTDiagBitFieldList* pBFList)
{
   OSCRTLMEMSET (pBFList, 0, sizeof(OSRTDiagBitFieldList));
   rtxSListInitEx (pctxt, &pBFList->fieldList);
   pBFList->printBuffer.lbm = 0x80;
   pBFList->pCaptureBuf = 0;
#ifndef _NO_STREAM
   /* If context is using an input stream, set up capture buffer */
   if (OSRTISSTREAM(pctxt) && OSRTSTREAM_ID(pctxt) != OSRTSTRMID_MEMORY) {
      pBFList->pCaptureBuf = rtxMemAllocType (pctxt, OSRTMEMBUF);
      if (0 != pBFList->pCaptureBuf) {
         rtxMemBufInit (pctxt, pBFList->pCaptureBuf, 0);
         rtxStreamSetCapture (pctxt, pBFList->pCaptureBuf);
      }
   }
#endif /* _NO_STREAM */
}

EXTRTMETHOD void rtxDiagInsBitFieldLen (OSRTDiagBitFieldList* pBFList)
{
   OSRTDiagBitField* pCurrField;
   OSRTDiagBitField* pNewField;
   OSRTSListNode* pNode;

   if (0 == pBFList || pBFList->disabledCount > 0) return;

   if ((pNode = pBFList->fieldList.tail) != 0) {
      const char* suffix;
      OSCTXT* pctxt = pBFList->fieldList.pctxt;
      pCurrField = (OSRTDiagBitField*) pNode->data;

      /* Change suffix in current field to length */
      suffix = pCurrField->nameSuffix;
      pCurrField->nameSuffix = "length";

      /* Create a new field and set name and suffix to that of current field */
      pNewField = rtxMemAllocType (pctxt, OSRTDiagBitField);
      if (0 == pNewField) return;

      pNewField->elemName = pCurrField->elemName;
      pNewField->nameSuffix = suffix;

      pCurrField->numbits =
         rtxCtxtGetBitOffset(pctxt) - pCurrField->bitOffset;

      /* Append the new field to the field list and initialize the   */
      /* bit offset and count..                                      */
      pNewField->bitOffset = rtxCtxtGetBitOffset (pctxt);
      pNewField->numbits   = 0;
      pNewField->pOpenTypeFieldList = 0;

      rtxSListAppend (&pBFList->fieldList, pNewField);
   }
}

EXTRTMETHOD OSRTDiagBitField* rtxDiagNewBitField
(OSRTDiagBitFieldList* pBFList, const char* nameSuffix)
{
   OSRTDiagBitField* pField = 0;

   if (0 != pBFList && pBFList->disabledCount == 0) {
      OSCTXT* pctxt = pBFList->fieldList.pctxt;

      pField = rtxMemAllocType (pctxt, OSRTDiagBitField);
      if (pField == NULL) return NULL;

      rtxDListToUTF8Str
         (pctxt, &pctxt->elemNameStack, (OSUTF8CHAR**)&pField->elemName, '.');

      pField->nameSuffix = nameSuffix;
      pField->bitOffset = rtxCtxtGetBitOffset (pctxt);
      pField->numbits = 0;
      pField->pOpenTypeFieldList = 0;

      rtxSListAppend (&pBFList->fieldList, pField);
   }

   return pField;
}

EXTRTMETHOD void rtxDiagSetBitFldOffset (OSRTDiagBitFieldList* pBFList)
{
   if (0 != pBFList) {
      OSRTSListNode* pNode = pBFList->fieldList.tail;

      if (pBFList->disabledCount == 0 && 0 != pNode) {
         OSCTXT* pctxt = pBFList->fieldList.pctxt;
         OSRTDiagBitField* pfld = (OSRTDiagBitField*) pNode->data;
         if (0 != pfld && 0 == pfld->numbits) {
            pfld->bitOffset = rtxCtxtGetBitOffset (pctxt);
         }
      }
   }
}

EXTRTMETHOD void rtxDiagSetBitFldCount (OSRTDiagBitFieldList* pBFList)
{
   if (0 != pBFList) {
      OSRTSListNode* pNode = pBFList->fieldList.tail;

      if (pBFList->disabledCount == 0 && 0 != pNode) {
         OSCTXT* pctxt = pBFList->fieldList.pctxt;
         OSRTDiagBitField* pfld = (OSRTDiagBitField*) pNode->data;
         if (0 != pfld) {
            pfld->numbits = rtxCtxtGetBitOffset(pctxt) - pfld->bitOffset;
         }
      }
   }
}

EXTRTMETHOD void rtxDiagSetBitFldNameSuffix
(OSRTDiagBitFieldList* pBFList, const char* nameSuffix)
{
   if (0 != pBFList) {
      OSRTSListNode* pNode = pBFList->fieldList.tail;

      if (pBFList->disabledCount == 0 && 0 != pNode) {
         OSRTDiagBitField* pfld = (OSRTDiagBitField*) pNode->data;
         if (0 != pfld) {
            pfld->nameSuffix = nameSuffix;
         }
      }
   }
}

EXTRTMETHOD OSBOOL rtxDiagSetBitFldDisabled
(OSRTDiagBitFieldList* pBFList, OSBOOL value)
{
   if (0 != pBFList) {
      if (value)
         pBFList->disabledCount++;
      else if (pBFList->disabledCount > 0)
         pBFList->disabledCount--;

      return (OSBOOL)(pBFList->disabledCount > 0);
   }
   else return TRUE;
}

EXTRTMETHOD void rtxDiagBitFldAppendNamePart
(OSRTDiagBitFieldList* pBFList, const char* namePart)
{
   if (0 != pBFList) {
      OSRTSListNode* pNode = pBFList->fieldList.tail;

      if (pBFList->disabledCount == 0 && 0 != pNode) {
         OSCTXT* pctxt = pBFList->fieldList.pctxt;
         OSRTDiagBitField* pfld = (OSRTDiagBitField*) pNode->data;
         if (0 != pfld) {
            if (!OS_ISEMPTY (pfld->elemName)) {
               size_t bufsiz =
                  OSCRTLSTRLEN((const char*)pfld->elemName) +
                  OSCRTLSTRLEN(namePart) + 2;
               char* buf = (char*) rtxMemRealloc
                  (pctxt, (void*)pfld->elemName, bufsiz);
               if (0 != buf) {
                  rtxStrcat (buf, bufsiz, ".");
                  rtxStrcat (buf, bufsiz, namePart);
                  pfld->elemName = buf;
               }
            }
            else {
               rtxDListToUTF8Str (pctxt,
                  &pctxt->elemNameStack, (OSUTF8CHAR**)&pfld->elemName, '.');
            }
         }
      }
   }
}

