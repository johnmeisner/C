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

/* Run-time doubly-linked list functions for scalar types */

#include <stdlib.h>
#include <string.h>
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxContext.h"
#include "rtxsrc/rtxScalarDList.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxUTF8.h"

EXTRTMETHOD void rtxScalarDListInit (OSRTScalarDList* pList)
{
   if (0 != pList) {
      pList->count = 0;
      pList->head = (OSRTScalarDListNode*) 0;
      pList->tail = (OSRTScalarDListNode*) 0;
   }
}

EXTRTMETHOD OSRTScalarDListNode* rtxScalarDListAppendInt32
(OSCTXT* pctxt, OSRTScalarDList* pList, OSINT32 value)
{
   OSRTScalarDListNode* pListNode =
      rtxMemAllocType (pctxt, OSRTScalarDListNode);

   if (0 != pListNode) {
      pListNode->ident = ScalarDList_int32;
      pListNode->u.i32val = value;
      rtxScalarDListAppendNode (pList, pListNode);
   }

   return pListNode;
}

EXTRTMETHOD OSRTScalarDListNode* rtxScalarDListAppendUInt32
(OSCTXT* pctxt, OSRTScalarDList* pList, OSUINT32 value)
{
   OSRTScalarDListNode* pListNode =
      rtxMemAllocType (pctxt, OSRTScalarDListNode);

   if (0 != pListNode) {
      pListNode->ident = ScalarDList_uint32;
      pListNode->u.ui32val = value;
      rtxScalarDListAppendNode (pList, pListNode);
   }

   return pListNode;
}

#if !defined(_NO_INT64_SUPPORT)
EXTRTMETHOD OSRTScalarDListNode* rtxScalarDListAppendInt64
(OSCTXT* pctxt, OSRTScalarDList* pList, OSINT64 value)
{
   OSRTScalarDListNode* pListNode =
      rtxMemAllocType (pctxt, OSRTScalarDListNode);

   if (0 != pListNode) {
      pListNode->ident = ScalarDList_int64;
      pListNode->u.i64val = value;
      rtxScalarDListAppendNode (pList, pListNode);
   }

   return pListNode;
}

EXTRTMETHOD OSRTScalarDListNode* rtxScalarDListAppendUInt64
(OSCTXT* pctxt, OSRTScalarDList* pList, OSUINT64 value)
{
   OSRTScalarDListNode* pListNode =
      rtxMemAllocType (pctxt, OSRTScalarDListNode);

   if (0 != pListNode) {
      pListNode->ident = ScalarDList_uint64;
      pListNode->u.ui64val = value;
      rtxScalarDListAppendNode (pList, pListNode);
   }

   return pListNode;
}
#endif

EXTRTMETHOD OSRTScalarDListNode* rtxScalarDListAppendDouble
(OSCTXT* pctxt, OSRTScalarDList* pList, OSDOUBLE value)
{
   OSRTScalarDListNode* pListNode =
      rtxMemAllocType (pctxt, OSRTScalarDListNode);

   if (0 != pListNode) {
      pListNode->ident = ScalarDList_double;
      pListNode->u.dfltval = value;
      rtxScalarDListAppendNode (pList, pListNode);
   }

   return pListNode;
}

EXTRTMETHOD OSRTScalarDListNode* rtxScalarDListAppendFloat
(OSCTXT* pctxt, OSRTScalarDList* pList, OSFLOAT value)
{
   OSRTScalarDListNode* pListNode =
      rtxMemAllocType (pctxt, OSRTScalarDListNode);

   if (0 != pListNode) {
      pListNode->ident = ScalarDList_float;
      pListNode->u.fltval = value;
      rtxScalarDListAppendNode (pList, pListNode);
   }

   return pListNode;
}

EXTRTMETHOD OSRTScalarDListNode* rtxScalarDListAppendNode
(OSRTScalarDList* pList, OSRTScalarDListNode* pListNode)
{
   if (0 != pListNode) {
      pListNode->next = (OSRTScalarDListNode*) 0;
      if (0 != pList->tail) {
         pList->tail->next = pListNode;
         pListNode->prev = pList->tail;
      }
      if (0 == pList->head) {
         pList->head = pListNode;
         pListNode->prev = (OSRTScalarDListNode*) 0;
      }
      pList->tail = pListNode;
      pList->count++;
   }

   return pListNode;
}

EXTRTMETHOD OSRTScalarDListNode* rtxScalarDListInsertNode
(OSRTScalarDList* pList, OSUINT32 idx, OSRTScalarDListNode* pListNode)
{
   if (0 != pListNode) {
      if (idx == 0) { /* index 0 - first element */
         pListNode->next = pList->head;
         pListNode->prev = (OSRTScalarDListNode*) 0;
         if(pList->head != 0) {
            pList->head->prev = pListNode;
         }
         if(pList->tail == 0) {
            pList->tail = pListNode;
         }
         pList->head = pListNode;
      }
      else if (idx == pList->count) { /* last index (tail case) */
         pListNode->next = (OSRTScalarDListNode*) 0;
         if (0 != pList->tail) {
            pList->tail->next = pListNode;
            pListNode->prev = pList->tail;
         }
         if (0 == pList->head) {
            pList->head = pListNode;
            pListNode->prev = (OSRTScalarDListNode*) 0;
         }
         pList->tail = pListNode;
      }
      else {
         OSUINT32 i;
         OSRTScalarDListNode* curNode;

         for (i = 1, curNode = pList->head; i < idx; i++) {
            curNode = curNode->next;
         }
         pListNode->next = curNode->next;
         pListNode->prev = curNode;
         curNode->next = pListNode;
         pListNode->next->prev = pListNode;
      }
      pList->count++;
   }

   return pListNode;
}

OSRTScalarDListNode* rtxScalarDListFindByIndex
(const OSRTScalarDList* pList, OSUINT32 idx)
{
   OSRTScalarDListNode* curNode;
   OSUINT32 i;

   if (idx >= pList->count) return 0;
   for (i = 0, curNode = pList->head; i < idx; i++) {
      curNode = curNode->next;
   }
   return curNode;
}

/* Remove node from list. Node is not freed */
EXTRTMETHOD void rtxScalarDListRemove (OSRTScalarDList* pList, OSRTScalarDListNode* node)
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
EXTRTMETHOD void rtxScalarDListFreeNode
(struct OSCTXT* pctxt, OSRTScalarDList* pList, OSRTScalarDListNode* node)
{
   if (0 != node) {
      rtxScalarDListRemove (pList, node);
      rtxMemFreeType (pctxt, node);
   }
}

/* Free all nodes, but not the data */
EXTRTMETHOD void rtxScalarDListFreeNodes (struct OSCTXT* pctxt, OSRTScalarDList* pList)
{
   OSRTScalarDListNode* pNode, *pNextNode;

   for (pNode = pList->head; pNode != 0; pNode = pNextNode) {
      pNextNode = pNode->next;
      rtxMemFreeType (pctxt, pNode);
   }
   pList->count = 0;
   pList->head = pList->tail = 0;
}
