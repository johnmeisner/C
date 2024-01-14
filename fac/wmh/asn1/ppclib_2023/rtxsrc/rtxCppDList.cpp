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

#include "rtxsrc/rtxCppDList.h"

// Nodes classes

OSRTDListNodeBaseClass::OSRTDListNodeBaseClass (void* pdata)
{
   // This constructor assumes pdata was allocated using the new operator.
   // It will delete the object when the list is destroyed..
   data = pdata; next = prev = 0;
}

OSRTDListNodeBaseClass::OSRTDListNodeBaseClass ()
{
   data = 0; next = prev = 0;
}

OSRTDListNodeClass::~OSRTDListNodeClass()
{
   delete [] (OSOCTET*)data;
}

OSRTObjListNodeClass::OSRTObjListNodeClass (OSRTBaseType* pdata) :
   OSRTDListNodeBaseClass (pdata)
{
}


OSRTObjListNodeClass::OSRTObjListNodeClass (const OSRTObjListNodeClass& orig) :
   OSRTDListNodeBaseClass ()
{
   OSRTBaseType* cloned = ((OSRTBaseType*)orig.data)->clone ();
   OSRTASSERT (0 != cloned);
   data = cloned;
   next = prev = 0;
}

OSRTObjListNodeClass::~OSRTObjListNodeClass()
{
   delete (OSRTBaseType*)data;
}

// DList classes

OSRTDListBaseClass::~OSRTDListBaseClass()
{
   OSRTDListBaseClass::release ();
}

void OSRTDListBaseClass::release ()
{
   OSRTDListNodeBaseClass* pNode, *pNextNode;
   for (pNode = (OSRTDListNodeBaseClass*) head; pNode != 0; pNode = pNextNode) {
      pNextNode = (OSRTDListNodeBaseClass*) pNode->next;
      delete pNode;
   }

   count = 0;
   head = 0;
   tail = 0;
}

void OSRTDListBaseClass::remove (int index)
{
   OSRTDListNodeBaseClass* node =
      (OSRTDListNodeBaseClass*) rtxDListFindByIndex (this, index);
   if (0 != node) {
      rtxDListRemove (this, node);
      delete node;
   }
}

void OSRTDListClass::append (void* pdata)
{
   OSRTDListNodeClass* pNode = new OSRTDListNodeClass (pdata);
   rtxDListAppendNode (*this, *pNode);
}

void OSRTDListClass::appendCopy (void* pdata, size_t nbytes)
{
   void* pcopy = new OSOCTET[nbytes];
   memcpy (pcopy, pdata, nbytes);
   OSRTDListNodeClass* pNode = new OSRTDListNodeClass (pcopy);
   rtxDListAppendNode (*this, *pNode);
}

void OSRTDListClass::insert (int index, void* pdata)
{
   OSRTDListNodeClass* pNode = new OSRTDListNodeClass (pdata);
   rtxDListInsertNode (*this, index, *pNode);
}

void OSRTObjListClass::append (OSRTBaseType* pdata)
{
   OSRTObjListNodeClass* pNode = new OSRTObjListNodeClass (pdata);
   rtxDListAppendNode (*this, *pNode);
}

void OSRTObjListClass::appendCopy (const OSRTBaseType* pdata)
{
   OSRTBaseType* pCopy = pdata->clone();
   OSRTObjListNodeClass* pNode = new OSRTObjListNodeClass (pCopy);
   rtxDListAppendNode (*this, *pNode);
}

void OSRTObjListClass::insert (int index, OSRTBaseType* pdata)
{
   OSRTObjListNodeClass* pNode = new OSRTObjListNodeClass (pdata);
   rtxDListInsertNode (*this, index, *pNode);
}

OSRTObjListClass::OSRTObjListClass(const OSRTObjListClass& orig) :
   OSRTDListBaseClass ()
{
   doCopy (orig);
}

void OSRTObjListClass::doCopy (const OSRTObjListClass& orig)
{
   OSRTObjListNodeClass* pNode;
   for (pNode = (OSRTObjListNodeClass*) orig.head; pNode != 0;
      pNode = (OSRTObjListNodeClass*)((OSRTDListNode*)pNode)->next)
   {
      OSRTBaseType* cloned =
         ((OSRTBaseType*)((OSRTDListNode*)pNode)->data)->clone ();
      if (cloned == 0)
         continue;

      append (cloned);
   }
}

OSRTObjListClass& OSRTObjListClass::operator= (const OSRTObjListClass& orig)
{
   // test for assignment to self
   if (this != &orig) {
      release ();
      doCopy (orig);
   }
   return *this;
}

