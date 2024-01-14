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

#include <stdarg.h>
#include <stdlib.h>
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxSList.h"

/* Singly-linked list */

EXTRTMETHOD void rtxSListInit (OSRTSList* pList)
{
   if (pList) {
      pList->count = 0;
      pList->head = (OSRTSListNode*) 0;
      pList->tail = (OSRTSListNode*) 0;
      pList->pctxt = 0;
   }
}

EXTRTMETHOD void rtxSListInitEx (OSCTXT* pctxt, OSRTSList* pList)
{
   if (pList) {
      pList->count = 0;
      pList->head = (OSRTSListNode*) 0;
      pList->tail = (OSRTSListNode*) 0;
      pList->pctxt = pctxt;
   }
}

static void freeSList (OSRTSList* pList, OSBOOL freeData)
{
   if (pList) {
      OSRTSListNode *p1, *p2 = pList->head;
      while (p2) {
         p1 = p2;
         p2 = p1->next;
         if (pList->pctxt) {
            if (freeData) rtxMemFreePtr (pList->pctxt, p1->data);
            rtxMemFreeType (pList->pctxt, p1);
         }
         else {
            if (freeData) OSCRTFREE0 (p1->data);
            OSCRTFREE0 (p1);
         }
      }
      rtxSListInitEx (pList->pctxt, pList);
   }
}

EXTRTMETHOD void rtxSListFree (OSRTSList* pList)
{
   freeSList (pList, FALSE);
}

EXTRTMETHOD void rtxSListFreeAll (OSRTSList* pList)
{
   freeSList (pList, TRUE);
}

EXTRTMETHOD OSRTSList* rtxSListCreate ()
{
   OSRTSList* pList = (OSRTSList*) OSCRTMALLOC0 (sizeof(OSRTSList));
   if (pList == 0) return 0;
   rtxSListInit (pList);
   return pList;
}

EXTRTMETHOD OSRTSList* rtxSListCreateEx (OSCTXT* pctxt)
{
   OSRTSList* pList = rtxMemAllocType (pctxt, OSRTSList);
   if (pList == 0) return 0;
   rtxSListInitEx (pctxt, pList);
   return pList;
}

EXTRTMETHOD OSRTSListNode* rtxSListAppend (OSRTSList* pList, void* pData)
{
   OSRTSListNode* pListNode =
      (OSRTSListNode*) ((pList->pctxt) ?
      rtxMemAllocType (pList->pctxt, OSRTSListNode) :
      OSCRTMALLOC0 (sizeof(OSRTSListNode)));

   if (pListNode) {
      pListNode->data = pData;
      pListNode->next = (OSRTSListNode*) 0;
      if (pList->tail) {
         pList->tail->next = pListNode;
      }
      if (!pList->head) {
         pList->head = pListNode;
      }
      pList->tail = pListNode;
      pList->count++;
   }
   return pListNode;
}

EXTRTMETHOD OSBOOL rtxSListFind (OSRTSList* pList, void* pData)
{
   OSRTSListNode* pNode = pList->head;
   while (0 != pNode) {
      if (pNode->data == pData) return TRUE;
      else pNode = pNode->next;
   }
   return FALSE;
}

EXTRTMETHOD void rtxSListRemove (OSRTSList* pList, void* pData)
{
   OSRTSListNode* pNode = pList->head;
   OSRTSListNode* pPrev = 0;

   while (0 != pNode) {
      if (pNode->data == pData) break;
      else {
         pPrev = pNode;
         pNode = pNode->next;
      }
   }

   if (0 != pNode) {
      if (0 == pPrev) {
         /* head node */
         pList->head = pNode->next;
      }
      else {
         pPrev->next = pNode->next;
      }
      if (pList->tail == pNode) {
         pList->tail = pPrev;
      }
      pList->count--;
      if (pList->pctxt)
         rtxMemFreeType (pList->pctxt, pNode);
      else
         OSCRTFREE0 (pNode);
   }
}

