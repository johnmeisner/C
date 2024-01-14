/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
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

/* Run-time doubly-linked list functions */

#include <stdlib.h>
#include <string.h>
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxContext.hh"
#include "rtxsrc/rtxDList.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxUTF8.h"

EXTRTMETHOD void rtxDListInit (OSRTDList* pList)
{
   if (0 != pList) {
      pList->count = 0;
      pList->head = (OSRTDListNode*) 0;
      pList->tail = (OSRTDListNode*) 0;
   }
}

EXTRTMETHOD OSRTDListNode* rtxDListAppend
(OSCTXT* pctxt, OSRTDList* pList, void* pData)
{
   OSRTDListNode* pListNode =
      rtxMemAllocType (pctxt, OSRTDListNode);

   if (0 != pListNode) {
      pListNode->data = pData;
      rtxDListAppendNode (pList, pListNode);
   }

   return pListNode;
}

EXTRTMETHOD OSRTDListNode* rtxDListAppendCharArray
(OSCTXT* pctxt, OSRTDList* pList, size_t length, char* pData)
{
   OSRTDListNode* pListNode =
      rtxMemAllocType (pctxt, OSRTDListNode);

   if (0 != pListNode) {
      pListNode->data = (void *)rtxMemAlloc (pctxt, length);
      if (0 == pListNode->data) {
         rtxMemFreePtr (pctxt, pListNode);
         LOG_RTERR (pctxt, RTERR_NOMEM);
         return 0;
      }
      OSCRTLSAFEMEMCPY (pListNode->data, length, pData, length);
      rtxDListAppendNode (pList, pListNode);
   }
   else {
      LOG_RTERR (pctxt, RTERR_NOMEM);
      /* pListNode == 0, will be returned below */
   }

   return pListNode;
}

EXTRTMETHOD OSRTDListNode* rtxDListAppendNode
(OSRTDList* pList, OSRTDListNode* pListNode)
{
   if (0 != pListNode) {
      pListNode->next = (OSRTDListNode*) 0;
      if (0 != pList->tail) {
         pList->tail->next = pListNode;
         pListNode->prev = pList->tail;
      }
      if (0 == pList->head) {
         pList->head = pListNode;
         pListNode->prev = (OSRTDListNode*) 0;
      }
      pList->tail = pListNode;
      pList->count++;
   }

   return pListNode;
}

EXTRTMETHOD OSRTDListNode* rtxDListInsert
(OSCTXT* pctxt, OSRTDList* pList, OSSIZE idx, void* pData)
{
   OSRTDListNode* pListNode;

   LCHECKX2 (pctxt);

   if (idx > pList->count) {
      pListNode = rtxDListAppend (pctxt, pList, pData);
   }
   else {
      pListNode =
         rtxMemAllocType (pctxt, OSRTDListNode);

      if (0 != pListNode) {
         pListNode->data = pData;
         rtxDListInsertNode (pList, idx, pListNode);
      }
   }

   return pListNode;
}

EXTRTMETHOD OSRTDListNode* rtxDListInsertNode
(OSRTDList* pList, OSSIZE idx, OSRTDListNode* pListNode)
{
   if (0 != pListNode) {
      if (idx == 0) { /* index 0 - first element */
         pListNode->next = pList->head;
         pListNode->prev = (OSRTDListNode*) 0;
         if(pList->head != 0) {
            pList->head->prev = pListNode;
         }
         if(pList->tail == 0) {
            pList->tail = pListNode;
         }
         pList->head = pListNode;
      }
      else if (idx == pList->count) { /* last index (tail case) */
         pListNode->next = (OSRTDListNode*) 0;
         if (0 != pList->tail) {
            pList->tail->next = pListNode;
            pListNode->prev = pList->tail;
         }
         if (0 == pList->head) {
            pList->head = pListNode;
            pListNode->prev = (OSRTDListNode*) 0;
         }
         pList->tail = pListNode;
      }
      else {
         OSSIZE i;
         OSRTDListNode* curNode;

         for (i = 1, curNode = pList->head; i < idx; i++) {
            curNode = curNode->next;
         }
         pListNode->next = curNode->next;
         pListNode->prev = curNode;
         curNode->next = pListNode;
         /* here, pListNode->next always should be non-zero,
          * because if pListNode->next is zero - it is tail case (see above)
          */
         pListNode->next->prev = pListNode;
      }
      pList->count++;
   }

   return pListNode;
}

EXTRTMETHOD OSRTDListNode* rtxDListInsertBefore
(OSCTXT* pctxt, OSRTDList* pList, OSRTDListNode* node, void* pData)
{
   OSRTDListNode* pListNode =
      rtxMemAllocType (pctxt, OSRTDListNode);

   if (0 != pListNode) {
      pListNode->data = pData;

      if (node == 0) { /* insert before end (as last element) */
         pListNode->next = (OSRTDListNode*) 0;
         if (0 != pList->tail) {
            pList->tail->next = pListNode;
            pListNode->prev = pList->tail;
         }
         if (0 == pList->head) {
            pList->head = pListNode;
            pListNode->prev = (OSRTDListNode*) 0;
         }
         pList->tail = pListNode;
      }
      else if(node == pList->head) { /* insert as head (head case) */
         pListNode->next = pList->head;
         pListNode->prev = (OSRTDListNode*) 0;
         if(pList->head != 0) {
            pList->head->prev = pListNode;
         }
         if(pList->tail == 0) {
            pList->tail = pListNode;
         }
         pList->head = pListNode;
      }
      else { /* other cases */
         pListNode->next = node;
         pListNode->prev = node->prev;
         node->prev = pListNode;
         /* here, pListNode->prev always should be non-zero,
          * because if pListNode->prev is zero - it is head case (see above).
          */
         pListNode->prev->next = pListNode;
      }

      pList->count++;
   }

   return pListNode;
}

EXTRTMETHOD OSRTDListNode* rtxDListInsertAfter
(OSCTXT* pctxt, OSRTDList* pList, OSRTDListNode* node, void* pData)
{
   OSRTDListNode* pListNode =
      rtxMemAllocType (pctxt, OSRTDListNode);

   if (0 != pListNode) {
      pListNode->data = pData;

      if(node == 0) { /* insert as head (as first element) */
         pListNode->next = pList->head;
         pListNode->prev = (OSRTDListNode*) 0;
         if(pList->head != 0) {
            pList->head->prev = pListNode;
         }
         if(pList->tail == 0) {
            pList->tail = pListNode;
         }
         pList->head = pListNode;
      }
      else if(node == pList->tail) { /* insert as tail (as last element) */
         pListNode->next = (OSRTDListNode*) 0;
         if (0 != pList->tail) {
            pList->tail->next = pListNode;
            pListNode->prev = pList->tail;
         }
         if (0 == pList->head) {
            pList->head = pListNode;
            pListNode->prev = (OSRTDListNode*) 0;
         }
         pList->tail = pListNode;
      }
      else { /* other cases */
         pListNode->next = node->next;
         pListNode->prev = node;
         node->next = pListNode;
         /* here, pListNode->next always should be non-zero,
          * because if pListNode->next is zero - it is tail case (see above).
          */
         pListNode->next->prev = pListNode;
      }

      pList->count++;
   }

   return pListNode;
}

OSRTDListNode* rtxDListFindByIndex (const OSRTDList* pList, OSSIZE idx)
{
   OSRTDListNode* curNode;
   OSSIZE i;

   if (idx >= pList->count) return 0;

   if (idx < (pList->count / 2)) {
      for (i = 0, curNode = pList->head; i < idx; i++) {
         curNode = curNode->next;
      }
   }
   else {
      for (i = pList->count - 1, curNode = pList->tail; i > idx; i--) {
         curNode = curNode->prev;
      }
   }
   return curNode;
}

OSRTDListNode* rtxDListFindByData (const OSRTDList* pList, void* data)
{
   OSRTDListNode* curNode;
   unsigned int i;

   for(i = 0, curNode = pList->head; i < pList->count; i++) {
      if(curNode->data == data)
         return curNode;
      curNode = curNode->next;
   }
   return 0;
}

OSRTDListNode* rtxDListFindFirstData (const OSRTDList* pList)
{
   OSRTDListNode* curNode;
   unsigned int i;

   for(i = 0, curNode = pList->head; i < pList->count; i++) {
      if(curNode->data != NULL)
         return curNode;
      curNode = curNode->next;
   }
   return 0;
}

EXTRTMETHOD int rtxDListFindIndexByData (const OSRTDList* pList, void* data)
{
   OSRTDListNode* curNode;
   unsigned int i;

   for(i = 0, curNode = pList->head; i < pList->count; i++) {
      if(curNode->data == data)
         return i;
      curNode = curNode->next;
   }
   return -1;
}

/* Remove node from list. Node is not freed */
EXTRTMETHOD void rtxDListRemove (OSRTDList* pList, OSRTDListNode* node)
{
   if (0 == node) return;
   if (node->next != 0) {
      node->next->prev = node->prev;
   }
   else { /* tail */
      pList->tail = node->prev;
   }
   if (node->prev != 0) {
      node->prev->next = node->next;
   }
   else { /* head */
      pList->head = node->next;
   }
   pList->count--;
}

/* Remove and free node from list. Data is not freed */
EXTRTMETHOD void rtxDListFreeNode
   (struct OSCTXT* pctxt, OSRTDList* pList, OSRTDListNode* node)
{
   if (0 != node) {
      rtxDListRemove (pList, node);
      rtxMemFreeType (pctxt, node);
   }
}

/* Free all nodes, but not the data */
EXTRTMETHOD void rtxDListFreeNodes (struct OSCTXT* pctxt, OSRTDList* pList)
{
   if (!rtxMemHeapIsEmpty (pctxt)) {
      OSRTDListNode* pNode, *pNextNode;

      for (pNode = pList->head; pNode != 0; pNode = pNextNode) {
         pNextNode = pNode->next;
         rtxMemFreeType (pctxt, pNode);
      }
   }
   pList->count = 0;
   pList->head = pList->tail = 0;
}

/* Free all nodes and their data */
EXTRTMETHOD void rtxDListFreeAll (struct OSCTXT* pctxt, OSRTDList* pList)
{
   if (!rtxMemHeapIsEmpty (pctxt)) {
      OSRTDListNode* pNode, *pNextNode;

      for (pNode = pList->head; pNode != 0; pNode = pNextNode) {
         pNextNode = pNode->next;

         /* Avoid freeing, if rtxDListAppendNode used */

         if (((const OSOCTET*)pNode->data) - OSRTDLISTNODESIZE
             != (const OSOCTET*)pNode)
            rtxMemFreeArray (pctxt, (void*)pNode->data);

         rtxMemFreeType (pctxt, pNode);
      }
   }
   pList->count = 0;
   pList->head = pList->tail = 0;
}

/* Create and fill (or only fill) the array from doubly-linked list */
EXTRTMETHOD int rtxDListToArray
(struct OSCTXT* pctxt, OSRTDList* pList, void** ppArray,
 OSSIZE* pElemCount, OSSIZE elemSize)
{
   OSRTDListNode* pNode;
   OSSIZE i = 0;
   OSSIZE arrayCount = (0 == pElemCount) ? 0 : *pElemCount;

   if (arrayCount < pList->count) {
      *ppArray = rtxMemAlloc (pctxt, pList->count * elemSize);
      if (*ppArray == NULL) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

      if (0 != pElemCount) *pElemCount = pList->count;
   }

   LCHECKX (pctxt);

   for (pNode = pList->head; pNode != 0; pNode = pNode->next, i += elemSize) {
      OSCRTLSAFEMEMCPY (((OSOCTET*)*ppArray) + i, elemSize, pNode->data, elemSize);
   }

   return (int)pList->count;
}

/* Create a UTF-8 string by concatanating the components of the list.
   The list is assumed to contain null-terminated string components. */
EXTRTMETHOD int rtxDListToUTF8Str
(struct OSCTXT* pctxt, OSRTDList* pList, OSUTF8CHAR** ppstr, char sep)
{
   OSRTDListNode* pNode;
   OSUTF8CHAR* sp;
   OSSIZE slen = 0;

   /* Calculate total length and allocate memory */
   for (pNode = pList->head; pNode != 0; pNode = pNode->next) {
      slen += rtxUTF8LenBytes (OSUTF8(pNode->data)) + 1;
   }
   if (slen > 0) {
      *ppstr = sp = (OSUTF8CHAR*) rtxMemAlloc (pctxt, slen);
      if (0 == *ppstr) return LOG_RTERR (pctxt, RTERR_NOMEM);

      /* Append components to string */
      for (pNode = pList->head; pNode != 0; pNode = pNode->next) {
         slen = rtxUTF8LenBytes (OSUTF8(pNode->data));
         OSCRTLSAFEMEMCPY (sp, slen, pNode->data, slen);
         sp += slen;
         if (pNode != pList->tail) *sp++ = sep; else *sp = '\0';
      }
   }
   else *ppstr = 0;

   return 0;
}

/* Append pointer to each element in array to list */

EXTRTMETHOD int rtxDListAppendArray
(OSCTXT* pctxt, OSRTDList* pList, void* pArray,
 OSSIZE numElements, OSSIZE elemSize)
{
   OSSIZE i;
   void* pData;
   for (i = 0; i < numElements; i++) {
      pData = ((OSOCTET*)pArray) + (i * elemSize);
      if (rtxDListAppend (pctxt, pList, pData) == 0)
         return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
   }
   LCHECKX (pctxt);
   return 0;
}

/* Append copy of each element in array to list */

EXTRTMETHOD int rtxDListAppendArrayCopy
(OSCTXT* pctxt, OSRTDList* pList, const void* pArray,
 OSSIZE numElements, OSSIZE elemSize)
{
   OSSIZE i;
   void* pData;
   for (i = 0; i < numElements; i++) {
      pData = rtxMemAllocArray (pctxt, elemSize, OSOCTET);
      if (pData == NULL) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

      OSCRTLSAFEMEMCPY (pData, elemSize, ((const OSOCTET*)pArray) + (i * elemSize), elemSize);

      if (rtxDListAppend (pctxt, pList, pData) == 0)
         return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
   }
   LCHECKX (pctxt);
   return 0;
}

EXTRTMETHOD void rtxDListBufInit (OSRTDListBuf* pBuf, OSSIZE segSz,
                                  void** ppdata, OSSIZE elemSz)
{
   if (pBuf) {
      pBuf->n = 0;
      pBuf->nMax = 0;
      pBuf->nAll = 0;
      pBuf->firstSegSz = segSz;
      pBuf->elemSize = elemSz;
      pBuf->dataArray = ppdata;
      rtxDListInit (&(pBuf->tmplist));
   }
}

static void rtxDListBufFreeAll (struct OSCTXT* pctxt, OSRTDListBuf* pBuf)
{
   rtxMemFreePtr (pctxt, *(pBuf->dataArray));
   rtxDListFreeAll (pctxt, &(pBuf->tmplist));
   *(pBuf->dataArray) = 0;
   pBuf->nAll = 0;
}

EXTRTMETHOD int rtxDListBufExpand (struct OSCTXT* pctxt, OSRTDListBuf* pBuf)
{
   int stat = 0;

   if (pBuf) {
      if (pBuf->nMax == 0) pBuf->nMax = pBuf->firstSegSz;
      else {
         pBuf->nMax *= 2;

         if (!rtxDListAppend (pctxt, &(pBuf->tmplist), *(pBuf->dataArray))) {
            rtxDListBufFreeAll (pctxt, pBuf);
            return LOG_RTERR (pctxt, RTERR_NOMEM);
         }
      }

      *(pBuf->dataArray) = rtxMemAlloc (pctxt, pBuf->nMax * pBuf->elemSize);
      if (*(pBuf->dataArray) == 0) {
         rtxDListBufFreeAll (pctxt, pBuf);
         return LOG_RTERR (pctxt, RTERR_NOMEM);
      }

      pBuf->n = 0;
   }
   else
      stat = LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   return stat;
}

EXTRTMETHOD int rtxDListBufToArray (struct OSCTXT* pctxt, OSRTDListBuf* pBuf)
{
   int stat = 0;

   if (pBuf) {
      if (pBuf->tmplist.count) {
         OSRTDListNode* pnode;

         if (*(pBuf->dataArray)) {
            if (!rtxDListAppend (pctxt, &(pBuf->tmplist),
                *(pBuf->dataArray)))
            {
               rtxDListBufFreeAll (pctxt, pBuf);
               return LOG_RTERR (pctxt, RTERR_NOMEM);
            }
         }

         *(pBuf->dataArray) = rtxMemAlloc (pctxt, pBuf->nAll * pBuf->elemSize);
         if (*(pBuf->dataArray) == 0) {
            rtxDListBufFreeAll (pctxt, pBuf);
            return LOG_RTERR (pctxt, RTERR_NOMEM);
         }

         pnode = pBuf->tmplist.head;
         pBuf->n = 0;
         pBuf->nMax = pBuf->firstSegSz;

         while (pnode) {
            OSSIZE rest = pBuf->nAll - pBuf->n;
            if (rest > pBuf->nMax) rest = pBuf->nMax;

            OSCRTLSAFEMEMCPY ((char*)*(pBuf->dataArray) + pBuf->n * pBuf->elemSize,
              rest*pBuf->elemSize, pnode->data, pBuf->elemSize * rest);

            pBuf->n += pBuf->nMax;
            pBuf->nMax *= 2;
            pnode = pnode->next;
         }

         rtxDListFreeAll (pctxt, &(pBuf->tmplist));
      }
      else
         *(pBuf->dataArray) = rtxMemRealloc (pctxt, *(pBuf->dataArray),
            pBuf->nMax * pBuf->elemSize);
   }
   else
      stat = LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   LCHECKX (pctxt);
   return stat;
}

EXTRTMETHOD OSRTDListNode* rtxDListInsertSorted (struct OSCTXT* pctxt,
   OSRTDList* pList, void* pData, PEqualsFunc equalsFunc, void* sortCtxt)
{
   OSRTDListNode* pListNode;

   pListNode = rtxMemAllocType (pctxt, OSRTDListNode);

   if (0 != pListNode) {
      pListNode->data = pData;
      rtxDListInsertNodeSorted (pList, pListNode, equalsFunc, sortCtxt);
   }

   return pListNode;
}

EXTRTMETHOD OSRTDListNode* rtxDListInsertNodeSorted (OSRTDList* pList,
   OSRTDListNode* pListNode, PEqualsFunc equalsFunc, void* sortCtxt)
{
   void* pData = pListNode->data;

   if (!pList->head) {
      /* insert to empty list */
      pListNode->next = (OSRTDListNode*) 0;
      pListNode->prev = (OSRTDListNode*) 0;
      pList->head = pListNode;
      pList->tail = pListNode;
   }
   else if (equalsFunc (pData, pList->tail->data, sortCtxt) >= 0) {
      /* insert to tail of list */
      /* tail first checked to optimize inserting partially sorted data
         that has ascending order */
      (pListNode->prev = pList->tail)->next = pListNode;
      pListNode->next = (OSRTDListNode*) 0;
      pList->tail = pListNode;
   }
   else if (pList->count == 1 ||
            equalsFunc (pData, pList->head->data, sortCtxt) <= 0)
   {
      /* insert to head of list */
      (pListNode->next = pList->head)->prev = pListNode;
      pListNode->prev = (OSRTDListNode*) 0;
      pList->head = pListNode;
   }
   else {
      /* find proper place in sorted list starting from second node */
      /* in this branch node will be always inserted between head and tail */
      OSRTDListNode* p = pList->head->next;
      while (equalsFunc (pData, p->data, sortCtxt) > 0)
         p = p->next;

      (pListNode->prev = p->prev)->next = pListNode;
      (pListNode->next = p)->prev = pListNode;
   }

   pList->count++;
   return pListNode;
}

