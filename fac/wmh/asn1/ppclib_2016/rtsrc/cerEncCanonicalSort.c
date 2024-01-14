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

#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxStreamMemory.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxContext.hh"


OSBOOL rtxCanonicalSort (OSOCTET* refPoint, OSRTSList* pList, OSBOOL normal)
{
   OSBOOL swap = TRUE, swapped = FALSE;
   OSRTSListNode *pNode1, *pNode2;
   OSRTBufLocDescr *pComponent1, *pComponent2;
   OSOCTET *pelem1, *pelem2;
   OSSIZE cmplen;
   int result;

   /* Sort elements */

   while (swap) {
      swap = FALSE;
      pNode1 = 0;
      pNode2 = pList->head;

      while (pNode2) {
         if (pNode1) {
            pComponent1 = (OSRTBufLocDescr*) pNode1->data;
            pComponent2 = (OSRTBufLocDescr*) pNode2->data;

            cmplen = OSRTMIN (pComponent1->numocts, pComponent2->numocts);

            /* Calculate encode buffer location pointers */
            if ( normal ) {
               pelem1 = refPoint + pComponent1->offset;
               pelem2 = refPoint + pComponent2->offset;
            }
            else {
               pelem1 = refPoint - pComponent1->offset;
               pelem2 = refPoint - pComponent2->offset;
            }

            result = memcmp (pelem1, pelem2, cmplen);
            if (result == 0)
               result = (int)(pComponent1->numocts - pComponent2->numocts);
            if ((normal && result > 0) || ( !normal && result < 0 ))
            {
               /* swap the data, not node */
               void* ptmp = pNode1->data;
               pNode1->data = pNode2->data;
               pNode2->data = ptmp;
               swap = swapped = TRUE;
            }
         }
         pNode1 = pNode2;
         pNode2 = pNode2->next;
      }
   }

   return swapped;
}


/***********************************************************************
 *
 *  CER ENCODE FUNCTIONS
 *
 **********************************************************************/

int rtxEncCanonicalSort (OSCTXT* pctxt, OSCTXT* pMemCtxt, OSRTSList* pList)
{
   OSBOOL swapped = FALSE;
   OSRTSListNode *pNode1, *pNode2;
   OSRTBufLocDescr *pComponent1;
   OSOCTET *pBuf;
   OSSIZE totalSize;
   int stat = 0;

   pBuf = rtxStreamMemoryGetBuffer (pMemCtxt, &totalSize);

   /* Sort the list */
   swapped = rtxCanonicalSort(pBuf, pList, TRUE /* normal */);

   /* If at least one element was swapped.. */

   if (swapped) {

      /* Write sorted elements */

      pNode1 = pList->head;
      while (pNode1) {
         pComponent1 = (OSRTBufLocDescr*) pNode1->data;

         if (stat == 0) {
            stat = rtxWriteBytes (pctxt,
               pBuf + pComponent1->offset, pComponent1->numocts);
         }

         rtxMemFreePtr (pMemCtxt, pComponent1);
         pNode2 = pNode1;
         pNode1 = pNode2->next;
         rtxMemFreePtr (pMemCtxt, pNode2);
      }
   }
   else {
      /* Write buffer "as is" */
      stat = rtxWriteBytes (pctxt, pBuf, totalSize);

      /* Clean up memory */
      pNode1 = pList->head;
      while (pNode1) {
         pComponent1 = (OSRTBufLocDescr*) pNode1->data;
         rtxMemFreePtr (pMemCtxt, pComponent1);
         pNode2 = pNode1;
         pNode1 = pNode2->next;
         rtxMemFreePtr (pMemCtxt, pNode2);
      }
   }

   LCHECKBER (pctxt);

   if (stat != 0) return LOG_RTERR (pctxt, stat);
   return 0;
}


void rtxGetBufLocDescr (OSCTXT *pctxt, OSRTBufLocDescr* pDescr)
{
   size_t totalSize;
   /* here we don't need flush stream, since the call to this function should be
      either with empty buffer or after rtxAddBufLocDescr" */
   rtxStreamMemoryGetBuffer (pctxt, &totalSize);
   pDescr->offset = totalSize;
}

void rtxAddBufLocDescr (OSCTXT *pctxt, OSRTSList* pElemList,
                        OSRTBufLocDescr* pDescr)
{
   OSRTBufLocDescr* pbuf;
   OSSIZE totalSize;
   rtxStreamFlush (pctxt); /* flush the internal buffer to memory stream's
                              buffer */
   rtxStreamMemoryGetBuffer (pctxt, &totalSize); /* we only need size */

   pbuf = (OSRTBufLocDescr*) rtxMemAlloc (pctxt, sizeof(OSRTBufLocDescr));
   if (pbuf == 0) return;
   memcpy (pbuf, pDescr, sizeof(OSRTBufLocDescr));
   pbuf->numocts = totalSize - pbuf->offset;
   rtxSListAppend (pElemList, pbuf);
}

