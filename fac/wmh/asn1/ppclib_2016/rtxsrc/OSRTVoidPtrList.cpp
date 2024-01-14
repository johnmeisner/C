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

#include "rtxsrc/OSRTVoidPtrList.h"

OSRTVoidPtrList::OSRTVoidPtrList ()
{
   mpHead = mpTail = 0; mCount = 0;
}


OSRTVoidPtrList::~OSRTVoidPtrList () {
   release ();
}

void OSRTVoidPtrList::append (OSVoid* object)
{
   OSRTVoidPtrListNode* node = new OSRTVoidPtrListNode (object);

   if (0 != mpTail) {
      mpTail->mpNext = node;
      node->mpPrev = mpTail;
      mpTail = node;
   }
   else
      mpHead = mpTail = node;

   mCount++;
}

void* OSRTVoidPtrList::deleteNode (OSRTVoidPtrListNode* node)
{
   if (0 == node) return (NULL);

   if (0 != node->mpPrev)
      node->mpPrev->mpNext = node->mpNext;
   else
      mpHead = node->mpNext;

   if (0 != node->mpNext)
      node->mpNext->mpPrev = node->mpPrev;
   else
      mpTail = node->mpPrev;

   void* object = node->mpObject;
   delete node;
   mCount--;

   return object;
}

void OSRTVoidPtrList::release () {
   while (0 != mpHead) deleteHead ();
   mCount = 0;
}

