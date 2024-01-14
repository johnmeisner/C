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
/**
 * @file OSRTVoidPtrList.h
 * Void pointer list class definition.  This is used to hold a list of
 * generic pointers.
 */
#ifndef _OSRTVOIDPTRLIST_H_
#define _OSRTVOIDPTRLIST_H_

#include "osSysTypes.h"

struct OSRTVoidPtrListNode {
   void* mpObject;
   OSRTVoidPtrListNode* mpNext;
   OSRTVoidPtrListNode* mpPrev;

   OSRTVoidPtrListNode () {
      mpObject = 0; mpNext = mpPrev = 0;
   }
   OSRTVoidPtrListNode (void* pObject) {
      mpObject = pObject; mpNext = mpPrev = 0;
   }
} ;

class OSRTVoidPtrList {
 protected:
   OSRTVoidPtrListNode* mpHead;
   OSRTVoidPtrListNode* mpTail;
   size_t mCount;

 public:
   OSRTVoidPtrList ();
   virtual ~OSRTVoidPtrList ();

   /**
    * This method appends a pointer to the end of the list.
    */
   void append (void* object);

   /**
    * This method return the count of pointers in the list.
    */
   inline size_t count() const { return mCount; }

   /**
    * This method deletes the first node in the list.  The pointer to the
    * object contained in this node is returned.
    */
   inline void* deleteHead () { return deleteNode(mpHead); }

   /**
    * This method deletes the given node fromthe list.  The pointer to the
    * object contained in this node is returned.
    */
   void* deleteNode (OSRTVoidPtrListNode* node);

   /**
    * This method returns the first node in the list.
    */
   inline OSRTVoidPtrListNode* getFirstNode () const { return mpHead; }

   /**
    * This method returns the last node in the list.
    */
   inline OSRTVoidPtrListNode* getLastNode () const { return mpTail; }

   /**
    * This method frees all nodes in the list.
    */
   void release();
} ;

class OSRTVoidPtrListIter {
 protected:
   OSRTVoidPtrListNode* mpNode;
   OSRTVoidPtrListNode  mTempNode;  // to hold a deleted node record
   OSRTVoidPtrList& mList;

 public:
   OSRTVoidPtrListIter (OSRTVoidPtrList& ll) :
      mList(ll) { setFirst(); }

   inline size_t getCount () const { return mList.count(); }
   inline void* getCurrentItem () const {
      return mpNode ? mpNode->mpObject : 0;
   }
   inline void* getNextItem () const {
      if (mpNode) return mpNode->mpNext ? mpNode->mpNext->mpObject : 0;
      else return 0;
   }
   inline void* getPrevItem () const {
      if (mpNode) return mpNode->mpPrev ? mpNode->mpPrev->mpObject : 0;
      else return 0;
   }

   inline OSBOOL isDone () const { return (mpNode == 0); }
   inline OSBOOL isFirst () const { return (mpNode == mList.getFirstNode()); }
   inline OSBOOL isLast () const { return (mpNode == mList.getLastNode()); }
   inline void setFirst () { mpNode = mList.getFirstNode(); }
   inline void setLast ()  { mpNode = mList.getLastNode(); }
   inline void setNext ()  { if (0 != mpNode) mpNode = mpNode->mpNext; }
   inline void setPrev ()  { if (0 != mpNode) mpNode = mpNode->mpPrev; }
   inline void setObject (void* pData) {
      if (mpNode) mpNode->mpObject = pData;
   }
} ;

#define ForAllOSRTVoidPtrListIter(p,listIter) \
for((listIter).setFirst(),p=(listIter).getCurrentItem();!(listIter).isDone();\
(listIter).setNext(),p=(listIter).getCurrentItem())

#endif
