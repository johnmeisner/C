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

// asn1CppEvtHndlr.cpp - this file contains implementations of the static
// methods necessary to register new event handlers..

#include "rtsrc/asn1CppEvtHndlr64.h"

// invoke methods: These methods loop through the list of event handler
// objects and invoke the appropriate method in each object..

EXTRTMETHOD void Asn1NamedEventHandler64::invokeStartElement
(OSCTXT* pCtxt, const char* name, OSSIZE index)
{
   Asn1NamedEventHandler64* pHandler;
   if (0 == ACINFO(pCtxt)) rtCtxtInitASN1Info(pCtxt);
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler64*) pListNode->data;
      pHandler->startElement (name, index);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler64::invokeEndElement
(OSCTXT* pCtxt, const char* name, OSSIZE index)
{
   Asn1NamedEventHandler64* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler64*) pListNode->data;
      pHandler->endElement (name, index);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler64::invokeBitStrValue
(OSCTXT* pCtxt, OSSIZE numbits, const OSOCTET* data)
{
   Asn1NamedEventHandler64* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler64*) pListNode->data;
      pHandler->bitStrValue (numbits, data);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler64::invokeOctStrValue
(OSCTXT* pCtxt, OSSIZE numocts, const OSOCTET* data)
{
   Asn1NamedEventHandler64* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler64*) pListNode->data;
      pHandler->octStrValue (numocts, data);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler64::invokeCharStrValue
(OSCTXT* pCtxt, OSSIZE nchars, const OSUTF8CHAR* data)
{
   Asn1NamedEventHandler64* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler64*) pListNode->data;
      pHandler->charStrValue (nchars, data);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler64::invokeCharStrValue
(OSCTXT* pCtxt, OSSIZE nchars, OSUNICHAR* data)
{
   Asn1NamedEventHandler64* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler64*) pListNode->data;
      pHandler->charStrValue (nchars, data);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler64::invokeCharStrValue
(OSCTXT* pCtxt, OSSIZE nchars, OS32BITCHAR* data)
{
   Asn1NamedEventHandler64* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler64*) pListNode->data;
      pHandler->charStrValue (nchars, data);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler64::invokeOpenTypeValue
(OSCTXT* pCtxt, OSSIZE numocts, const OSOCTET* data)
{
   Asn1NamedEventHandler64* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler64*) pListNode->data;
      pHandler->openTypeValue (numocts, data);
      pListNode = pListNode->next;
   }
}
