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
#include "rtsrc/ASN1CSeqOfList.h"
#include "rtsrc/asn1intl.h"

//////////////////////////////////////////////////////////////////////
//
// ASN1CSeqOfListIterator
//
// An iterator for lists that allows the programmer to traverse the
// list in either direction and modify the list during iteration.
//
// Author  Artem Bolgar

inline ASN1CSeqOfListIterator::ASN1CSeqOfListIterator(ASN1CSeqOfList* list) :
   stat (0)
{
   pSeqList = list;
   nextNode = pSeqList->pList->head;
   lastNode = 0;
   expectedModCount = list->modCount;
}

inline ASN1CSeqOfListIterator::ASN1CSeqOfListIterator(ASN1CSeqOfList* list,
   OSRTDListNode* startNode) : stat (0)
{
   pSeqList = list;
   nextNode = startNode;
   lastNode = 0;
   expectedModCount = list->modCount;
}

/*
* Returns the next element in the list.  This method may be called
* repeatedly to iterate through the list, or intermixed with calls to
* 'prev' to go back and forth.
*
* Returns the next element in the list. A null value will be returned
* if iteration is not successful.
*/
EXTRTMETHOD void* ASN1CSeqOfListIterator::next() {
   if(pSeqList->modCount != expectedModCount) {
      stat = ASN_E_CONCMODF;
      return (void*)0;
   }
   if(nextNode == 0) {
      stat = RTERR_SETMISRQ;
      return (void*)0;
   }
   lastNode = nextNode;
   nextNode = nextNode->next;
   return lastNode->data;
}

/*
* Returns the previous element in the list.  This method may be called
* repeatedly to iterate through the list backwards, or intermixed with
* calls to 'next' to go back and forth.
*
* Returns the previous element in the list. A null value will be returned
* if iteration is not successful.
*/
EXTRTMETHOD void* ASN1CSeqOfListIterator::prev() {
   if(pSeqList->modCount != expectedModCount) {
      stat = ASN_E_CONCMODF;
      return (void*)0;
   }
   if(nextNode == 0) {
      stat = RTERR_SETMISRQ;
      return (void*)0;
   }
   if(lastNode == 0)
      lastNode = nextNode;
   else
      lastNode = nextNode = nextNode->prev;
   return lastNode->data;
}

/*
* Removes from the list the last element that was returned by
* 'next' or 'prev'.  This call can
* only be made once per call to 'next' or 'prev'.
*
 * Returns 0 if operation is successful, or a negative status value if not.
*/
EXTRTMETHOD int ASN1CSeqOfListIterator::remove() {
   if(pSeqList->modCount != expectedModCount) {
      return (stat = ASN_E_CONCMODF);
   }
   if(lastNode == 0) {
      return (stat = ASN_E_ILLSTATE);
   }
   OSRTDListNode* lastNext = lastNode->next;
   pSeqList->remove(lastNode);
   if(nextNode == lastNode)
      nextNode = lastNext;
   lastNode = 0;
   expectedModCount = pSeqList->modCount;
   return 0;
}

/*
* Replaces the last element returned by 'next' or
* 'prev' with the specified element.
* This call can be made only if neither 'remove' nor
* 'add' have been called after the last call to 'next' or 'prev'.
*
* param 'data' - the element with which to replace the last element returned by
*          'next' or 'prev'.
*
* Returns 0 if operation is successful, or a negative status value if not.
*/
EXTRTMETHOD int ASN1CSeqOfListIterator::set(void* data) {
   if(pSeqList->modCount != expectedModCount) {
      return (stat = ASN_E_CONCMODF);
   }
   if(lastNode == 0) {
      return (stat = ASN_E_ILLSTATE);
   }
   lastNode->data = data;
   return 0;
}

/*
* Inserts the specified element into the list.  The
* element is inserted immediately before the next element that would be
* returned by 'next', if any, and after the next element that
* would be returned by 'prev', if any.  (If the list contains
* no elements, the new element becomes the sole element on the list.)
* The new element is inserted before the implicit cursor: a subsequent
* call to 'next' would be unaffected, and a subsequent call to
* 'prev' would return the new element.
*
* param 'data' - the element to insert.
*
* Returns 0 if operation is successful, or a negative status value if not.
*/
EXTRTMETHOD int ASN1CSeqOfListIterator::insert(void* data) {
   if(pSeqList->modCount != expectedModCount) {
      return (stat = ASN_E_CONCMODF);
   }
   pSeqList->insertBefore(data, nextNode);
   lastNode = 0;
   expectedModCount = pSeqList->modCount;
   return 0;
}

//////////////////////////////////////////////////////////////////////
//
// ASN1CSeqOfList
//
// Doubly-linked list implementation. This class provides all functionalty
// necessary for linked list. To iterate list use methods iterator, iteratorFrom,
// iteratorFromLast. Note, this implementation is not thread-safe.
//
// Author Artem Bolgar.
// version 1.09  12 Nov, 2003 (ASN1TPDUSeqOfList ctors have been added)
//
// History:
// version 1.08  11 Feb, 2003

EXTRTMETHOD ASN1CSeqOfList::ASN1CSeqOfList (OSRTMessageBufferIF& msgBuf, OSRTDList& lst,
   OSBOOL initBeforeUse) :
   ASN1CType(msgBuf), pList(&lst), modCount(0),
   wasAssigned (TRUE)
{
   if (initBeforeUse) init();
}

EXTRTMETHOD ASN1CSeqOfList::ASN1CSeqOfList (OSRTMessageBufferIF& msgBuf) :
   ASN1CType(msgBuf), modCount(0), wasAssigned (FALSE)
{
   pList = (OSRTDList*)memAlloc(sizeof(*pList));
   init();
}

EXTRTMETHOD ASN1CSeqOfList::ASN1CSeqOfList (ASN1CType& ccobj) :
   ASN1CType(ccobj), modCount(0), wasAssigned (FALSE)
{
   pList = (OSRTDList*)memAlloc(sizeof(*pList));
   init();
}

EXTRTMETHOD ASN1CSeqOfList::ASN1CSeqOfList
(OSRTMessageBufferIF& msgBuf, ASN1TSeqOfList& lst) :
   ASN1CType(msgBuf), pList((OSRTDList*)&lst), modCount(0), wasAssigned (TRUE)
{
}

EXTRTMETHOD ASN1CSeqOfList::ASN1CSeqOfList (ASN1CType& ccobj, ASN1TSeqOfList& lst) :
   ASN1CType(ccobj), pList((OSRTDList*)&lst), modCount(0), wasAssigned (TRUE)
{
}

EXTRTMETHOD ASN1CSeqOfList::ASN1CSeqOfList
(OSRTMessageBufferIF& msgBuf, ASN1TPDUSeqOfList& lst) :
   ASN1CType(msgBuf), pList((OSRTDList*)&lst), modCount(0), wasAssigned (TRUE)
{
}

EXTRTMETHOD ASN1CSeqOfList::ASN1CSeqOfList (OSRTContext& ctxt, OSRTDList& lst,
   OSBOOL initBeforeUse) :
   ASN1CType(ctxt), pList(&lst), modCount(0),
   wasAssigned (TRUE)
{
   if (initBeforeUse) init();
}

EXTRTMETHOD ASN1CSeqOfList::ASN1CSeqOfList (OSRTContext& ctxt) :
   ASN1CType(ctxt), modCount(0), wasAssigned (FALSE)
{
   pList = (OSRTDList*)memAlloc(sizeof(*pList));
   init();
}

EXTRTMETHOD ASN1CSeqOfList::ASN1CSeqOfList (OSRTContext& ctxt, ASN1TSeqOfList& lst) :
   ASN1CType(ctxt), pList((OSRTDList*)&lst), modCount(0), wasAssigned (TRUE)
{
}

EXTRTMETHOD ASN1CSeqOfList::ASN1CSeqOfList (OSRTContext& ctxt, ASN1TPDUSeqOfList& lst) :
   ASN1CType(ctxt), pList((OSRTDList*)&lst), modCount(0),
   wasAssigned (TRUE)
{
}

EXTRTMETHOD ASN1CSeqOfList::ASN1CSeqOfList (OSRTDList& lst) :
   ASN1CType(), pList(&lst), modCount(0),
   wasAssigned (TRUE)
{
}

EXTRTMETHOD ASN1CSeqOfList::ASN1CSeqOfList (ASN1TSeqOfList& lst) :
   ASN1CType(), pList((OSRTDList*)&lst), modCount(0), wasAssigned (TRUE)
{
}

EXTRTMETHOD ASN1CSeqOfList::ASN1CSeqOfList (ASN1TPDUSeqOfList& lst) :
   ASN1CType(), pList((OSRTDList*)&lst), modCount(0), wasAssigned (TRUE)
{
}

EXTRTMETHOD ASN1CSeqOfList::~ASN1CSeqOfList()
{
   if(!wasAssigned) {
      clear();
      memFreePtr (pList);
   }
}

// Appends new list node with data
EXTRTMETHOD void ASN1CSeqOfList::append (void* data)
{
   if (getCtxtPtr () == 0) return; //check the context
   rtxDListAppend(getCtxtPtr (), pList, data);
   modCount++;
}

// Appends array to list data. Data won't be copied, just assigned.
EXTRTMETHOD void ASN1CSeqOfList::appendArray
(const void* data, OSSIZE numElems, OSSIZE elemSize)
{
   if (getCtxtPtr() == 0) return; //check the context
   rtxDListAppendArray (getCtxtPtr(), pList, (void*)data, numElems, elemSize);
   modCount++;
}

// Appends array to list data. Data will be copied.
EXTRTMETHOD void ASN1CSeqOfList::appendArrayCopy
(const void* data, OSSIZE numElems, OSSIZE elemSize)
{
   if (getCtxtPtr () == 0) return; //check the context
   rtxDListAppendArrayCopy (getCtxtPtr (), pList, data, numElems, elemSize);
   modCount++;
}

// Inserts new list node at the specified index
EXTRTMETHOD void ASN1CSeqOfList::insert(int index, void* data) {
   if (getCtxtPtr () == 0) return; //check the context
   rtxDListInsert(getCtxtPtr (), pList, index, data);
   modCount++;
}

// Inserts new node ('data') before another node ('node')
EXTRTMETHOD void ASN1CSeqOfList::insertBefore(void* data, OSRTDListNode* node) {
   if (getCtxtPtr () == 0) return; //check the context
   rtxDListInsertBefore(getCtxtPtr (), pList, node, data);
   modCount++;
}

// Inserts new node ('data') after another node ('node')
EXTRTMETHOD void ASN1CSeqOfList::insertAfter(void* data, OSRTDListNode* node) {
   if (getCtxtPtr () == 0) return; //check the context
   rtxDListInsertAfter(getCtxtPtr (), pList, node, data);
   modCount++;
}

// Removes list node at specified index from the list
EXTRTMETHOD void ASN1CSeqOfList::remove(int index) {
   OSRTDListNode* node = rtxDListFindByIndex(pList, index);
   if(!node) return;
   rtxDListRemove(pList, node);
   modCount++;
}

// Removes  the first occurrence of the specified element in the list.
EXTRTMETHOD void ASN1CSeqOfList::remove(void* data) {
   OSRTDListNode* node = rtxDListFindByData(pList, data);
   if(!node) return;
   rtxDListRemove(pList, node);
   modCount++;
}

// Removes specified node from the list
EXTRTMETHOD void ASN1CSeqOfList::remove(OSRTDListNode* node) {
   if(!node) return;
   rtxDListRemove(pList, node);
   modCount++;
}

// Returns index of the list node with specified data
EXTRTMETHOD int ASN1CSeqOfList::indexOf(void* data) const {
   return rtxDListFindIndexByData(pList, data);
}

// Returns the first element from the list (null, if no elements in the list)
EXTRTMETHOD void* ASN1CSeqOfList::getFirst() {
   if(pList->head != 0)
      return pList->head->data;
   return 0;
}

// Returns the last element from the list (null, if no elements in the list)
EXTRTMETHOD void* ASN1CSeqOfList::getLast() {
   if(pList->tail != 0)
      return pList->tail->data;
   return 0;
}

// Returns element at specified index
EXTRTMETHOD void* ASN1CSeqOfList::get(int index) const {
   OSRTDListNode* node = rtxDListFindByIndex(pList, index);
   if(node == 0) return 0;
   return node->data;
}

// Sets data to list node at the specified index
EXTRTMETHOD void* ASN1CSeqOfList::set(int index, void* data) {
   OSRTDListNode* node = rtxDListFindByIndex(pList, index);
   if(node == 0) return 0;
   void* oldData = node->data;
   node->data = data;
   modCount++;
   return oldData;
}

// Clears list
EXTRTMETHOD void ASN1CSeqOfList::clear() {
   if (getCtxtPtr () == 0) return; //check the context
   rtxDListFreeNodes (getCtxtPtr (), pList);
   modCount++;
}

// Clears and frees list
EXTRTMETHOD void ASN1CSeqOfList::freeMemory() {
   if (getCtxtPtr () == 0) return; //check the context
   rtxDListFreeAll (getCtxtPtr (), pList);
   modCount++;
}

// Returns TRUE, if list is empty
EXTRTMETHOD OSBOOL ASN1CSeqOfList::isEmpty() const {
   return pList->count == 0;
}

// Returns the number of list's nodes
EXTRTMETHOD OSSIZE ASN1CSeqOfList::size() const {
   return pList->count;
}

// Converts to array
EXTRTMETHOD void* ASN1CSeqOfList::toArray (OSSIZE elemSize) {
   void* pArray;
   OSSIZE nelem = 0;

   if (getCtxtPtr () == 0) return 0; //check the context
   rtxDListToArray (getCtxtPtr (), pList, &pArray, &nelem, elemSize);
   return pArray;
}

// Converts to array
EXTRTMETHOD void* ASN1CSeqOfList::toArray
(void* pArray, OSSIZE elemSize, OSSIZE allocatedElems)
{
   OSSIZE nelem = allocatedElems;

   if (getCtxtPtr () == 0) return 0; //check the context
   rtxDListToArray (getCtxtPtr (), pList, &pArray, &nelem, elemSize);
   return pArray;
}

// Creates iterator from the head of the list
EXTRTMETHOD ASN1CSeqOfListIterator* ASN1CSeqOfList::iterator() {
   if (getCtxtPtr () == 0) return 0; //check the context
   void* iterPlace = memAlloc(sizeof(ASN1CSeqOfListIterator));
   ASN1CSeqOfListIterator* iter = new (iterPlace) ASN1CSeqOfListIterator(this);
   return iter;
}

// Creates iterator from the tail of the list
EXTRTMETHOD ASN1CSeqOfListIterator* ASN1CSeqOfList::iteratorFromLast() {
   if (getCtxtPtr () == 0) return 0; //check the context
   void* iterPlace = memAlloc(sizeof(ASN1CSeqOfListIterator));
   ASN1CSeqOfListIterator* iter = new (iterPlace) ASN1CSeqOfListIterator(this, pList->tail);
   return iter;
}

// Creates iterator from the node with specified data
EXTRTMETHOD ASN1CSeqOfListIterator* ASN1CSeqOfList::iteratorFrom(void* data) {
   if (getCtxtPtr () == 0) return 0; //check the context
   OSRTDListNode* node = rtxDListFindByData(pList, data);
   void* iterPlace = memAlloc(sizeof(ASN1CSeqOfListIterator));
   ASN1CSeqOfListIterator* iter = new (iterPlace) ASN1CSeqOfListIterator(this, node);
   return iter;
}

