/*
 * Copyright (c) 2014-2023 Objective Systems, Inc.
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

#include <stdlib.h>
#include <string.h>
#include "rtxsrc/rtxArrayList.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/osMacros.h"

/* Default equals function implementation.  This can be overridden. */
static OSBOOL defaultEquals (void* data1, void* data2, OSSIZE dataSize)
{
   if (data1 == data2) return TRUE;
   else if (data1 != 0 && data2 != 0 && dataSize > 0)
      return (OSBOOL)(!memcmp (data1, data2, dataSize));
   else
      return FALSE;
}

EXTRTMETHOD void rtxArrayListInit
(OSRTArrayList* pArrayList, OSSIZE capacity)
{
   if (0 != pArrayList) {
      rtxDListInit (&pArrayList->segments);
      pArrayList->segmentSize =
         (capacity != 0) ? capacity : OSRT_ARRAYLIST_CAPACITY;
      pArrayList->size = 0;
      pArrayList->dataSize = 0;
      pArrayList->equalsFunc = defaultEquals;
   }
}

EXTRTMETHOD OSRTArrayList* rtxNewArrayList (OSCTXT* pctxt, OSSIZE capacity)
{
   OSRTArrayList* pArrayList = rtxMemAllocType (pctxt, OSRTArrayList);
   if (0 != pArrayList) {
      rtxArrayListInit (pArrayList, capacity);
   }
   return pArrayList;
}

EXTRTMETHOD void rtxFreeArrayList (OSCTXT* pctxt, OSRTArrayList* pArrayList)
{
   if (0 != pArrayList) {
      rtxDListFreeAll (pctxt, &pArrayList->segments);
   }
}

EXTRTMETHOD int rtxArrayListAdd
(OSCTXT* pctxt, OSRTArrayList* pArrayList, void* pdata, OSSIZE* pindex)
{
   if (0 != pArrayList) {
      OSSIZE capacity = pArrayList->segmentSize * pArrayList->segments.count;

      if (pArrayList->size >= capacity) {
         OSRTDListNode* seg = (OSRTDListNode *)rtxMemAlloc (pctxt,
            sizeof (void*) * pArrayList->segmentSize + sizeof (OSRTDListNode));
         if (seg == 0) return RTERR_NOMEM;

         rtxDListAppendNode (&pArrayList->segments, seg);

         *(void**)(seg->data = seg + 1) = pdata;
      }
      else {
         ((void**) pArrayList->segments.tail->data)
            [pArrayList->size % pArrayList->segmentSize] = pdata;
      }
      if (0 != pindex) *pindex = (OSSIZE)pArrayList->size;

      pArrayList->size++;
   }
   else return RTERR_INVPARAM;

   return 0;
}

EXTRTMETHOD void rtxArrayListRemove
(OSCTXT* pctxt, OSRTArrayList* pArrayList, void* pdata)
{
   int idx = rtxArrayListIndexOf (pArrayList, pdata);
   if (idx >= 0)
      rtxArrayListRemoveIndexed (pctxt, pArrayList, idx);
}

EXTRTMETHOD void rtxArrayListRemoveIndexed
(OSCTXT* pctxt, OSRTArrayList* pArrayList, OSSIZE idx)
{
   if (0 != pArrayList) {
      if (idx == OSNULLINDEX || idx == pArrayList->size - 1) {
         if (pArrayList->size % pArrayList->segmentSize == 1) {
            /* remove last segment */
            rtxDListFreeNode (pctxt, &pArrayList->segments,
                              pArrayList->segments.tail);
         }

         pArrayList->size--;
      }
      else {
         OSSIZE tailSz = OSNULLINDEX;
         OSSIZE segmentSize = pArrayList->segmentSize;

         OSRTDListNode* seg = rtxDListFindByIndex
            (&pArrayList->segments, idx / segmentSize);

         if (0 == seg) {
            LOG_RTERR (pctxt, RTERR_OUTOFBND);
            return;
         }

         if (pArrayList->size >= (idx + 1)) {
            tailSz = pArrayList->size - (idx + 1);
         }
         idx %= segmentSize;

         if (seg->next != 0 && segmentSize > idx)
            tailSz = segmentSize - 1 - idx;

         if (tailSz != OSNULLINDEX) {
            memmove ((void**) seg->data + idx,
                     (void**) seg->data + idx + 1,
                     tailSz * sizeof (void*));
         }

         if (seg->next)
            ((void**) seg->data) [segmentSize - 1] =
               *((void**) seg->next->data);

         seg = seg->next;

         if (0 != seg) {
            while (seg->next) {
               /* shift full segments */
               memmove (seg->data, (void**) seg->data + 1,
                        (segmentSize - 1) * sizeof (void*));

               ((void**) seg->data) [segmentSize - 1] =
                  *((void**) seg->next->data);

               seg = seg->next;
            }

            /* shift last segment */
            tailSz = pArrayList->size % segmentSize;
            if (tailSz == 1) {
               rtxDListFreeNode (pctxt, &pArrayList->segments,
                                 pArrayList->segments.tail);
            }
            else {
               if (tailSz == 0) tailSz = segmentSize;
               memmove (seg->data, (void**) seg->data + 1,
                        (tailSz - 1) * sizeof (void*));
            }
         }

         pArrayList->size--;
      }
   }
}

EXTRTMETHOD int rtxArrayListInsert
(OSCTXT* pctxt, OSRTArrayList* pArrayList, void* pdata, OSSIZE idx)
{
   if (0 != pArrayList) {
      if (idx > pArrayList->size)
         return RTERR_OUTOFBND;
      else if (idx == pArrayList->size) /* insert to end */
         return rtxArrayListAdd (pctxt, pArrayList, pdata, 0);
      else {
         OSRTDListNode* seg;
         void *lastData;

         OSSIZE segmentSize = pArrayList->segmentSize;
         OSSIZE size = pArrayList->size;
         OSSIZE capacity = segmentSize * pArrayList->segments.count;

         seg = pArrayList->segments.tail;

         if (size == capacity) { /* add new segment */
            int stat;

            lastData = ((void**) seg->data) [segmentSize - 1];
            stat = rtxArrayListAdd (pctxt, pArrayList, lastData, 0);
            if (stat != RT_OK)
               return stat;

            /* capacity is index of current segment begin */
            capacity -= segmentSize;
         }
         else if ((capacity -= segmentSize) > idx)
         {
            /* move tail in last segment */
            memmove ((void**) seg->data + 1, seg->data,
               (size % segmentSize) * sizeof (void*));

            /* seg->prev is not null always */
            lastData =
               ((void**) seg->prev->data) [segmentSize - 1];
            *((void**) seg->data) = lastData;

            capacity -= segmentSize;
            seg = seg->prev;
         }

         /* shift full segments */
         while (capacity > idx) {
            memmove ((void**) seg->data + 1, seg->data,
                     (segmentSize - 1) * sizeof (void*));

            /* seg->prev is not null always */
            lastData =
               ((void**) seg->prev->data) [segmentSize - 1];
            *((void**) seg->data) = lastData;

            capacity -= segmentSize;
            seg = seg->prev;
         }

         /* shift insertion segment tail */
         idx -= (OSSIZE)capacity;

         if (idx < segmentSize - 1) {
            OSSIZE tailSz = segmentSize - 1 - idx;
            if (size < capacity + segmentSize)
               tailSz = size - capacity - 1 - idx;

            memmove ((void**) seg->data + idx + 1,
                     (void**) seg->data + idx,
                     tailSz * sizeof (void*));
         }

         ((void**) seg->data) [idx] = pdata;
         pArrayList->size++;

         return RT_OK;
      }
   }

   return RTERR_INVPARAM;
}

EXTRTMETHOD int rtxArrayListReplace
(OSRTArrayList* pArrayList, void* pdata, OSSIZE idx)
{
   if (0 != pArrayList) {
      if (idx >= pArrayList->size)
         return RTERR_OUTOFBND;
      else {
         OSRTDListNode* seg = rtxDListFindByIndex
            (&pArrayList->segments,
             idx / (OSSIZE)pArrayList->segmentSize);

         if (0 != seg) {
            ((void**) seg->data) [idx % pArrayList->segmentSize] = pdata;
         }

         return RT_OK;
      }
   }

   return RTERR_INVPARAM;
}

EXTRTMETHOD void* rtxArrayListGetIndexed
(const OSRTArrayList* pArrayList, OSSIZE idx)
{
   if (0 != pArrayList && idx < pArrayList->size) {
      OSRTDListNode* seg = rtxDListFindByIndex
         (&pArrayList->segments,
          idx / pArrayList->segmentSize);

      if (0 == seg) {
         return 0;
      }

      return ((void**) seg->data) [idx % pArrayList->segmentSize];
   }

   return 0;
}

EXTRTMETHOD int rtxArrayListIndexOf (OSRTArrayList* pArrayList, void* pdata)
{
   OSSIZE idx = 0;

   if (0 != pArrayList) {
      OSSIZE dataSize = pArrayList->dataSize;
      OSRTDListNode* seg = pArrayList->segments.head;

      while (seg) {
         void** p = (void**) seg->data;
         void** pEnd = p + pArrayList->segmentSize;

         if (seg->next == 0) {
            OSSIZE tailSz =
               pArrayList->size % pArrayList->segmentSize;
            if (tailSz)
               pEnd = p + tailSz;
         }

         for (;p != pEnd; p++, idx++) {
            if (pArrayList->equalsFunc (*p, pdata, dataSize))
               return (int) idx;
         }

         seg = seg->next;
      }

      return -1;
   }

   return RTERR_INVPARAM;
}

EXTRTMETHOD int rtxArrayListInitIter
(OSRTArrayListIter* piter, const OSRTArrayList* pArrayList, OSSIZE startIndex)
{
   if (0 == pArrayList || 0 == piter)
      return RTERR_INVPARAM;

   piter->pArrayList = pArrayList;

   if (startIndex == 0)
      piter->pSegment = pArrayList->segments.head;
   else if (startIndex < pArrayList->size)
      piter->pSegment = rtxDListFindByIndex
         (&pArrayList->segments,
          startIndex / pArrayList->segmentSize);

   piter->index = startIndex;

   return RT_OK;
}

EXTRTMETHOD OSBOOL rtxArrayListHasNextItem (OSRTArrayListIter* piter)
{
   return (OSBOOL)(0 != piter && piter->index < piter->pArrayList->size);
}

EXTRTMETHOD void* rtxArrayListNextItem (OSRTArrayListIter* piter)
{
   void* pdata = 0;

   if (piter && piter->pSegment) {
      /* get current */
      OSSIZE segPos = piter->index % piter->pArrayList->segmentSize;
      pdata = ((void**) piter->pSegment->data) [segPos];

      piter->index++;

      if (segPos + 1 == piter->pArrayList->segmentSize)
         piter->pSegment = piter->pSegment->next;
   }

   return pdata;
}

