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

// asn1CppEvtHndlr.cpp - this file contains implementations of the static
// methods necessary to register new event handlers..

#include "rtsrc/asn1CppEvtHndlr.h"

// addEventHandler : This static method adds an event handler object to
// the current list within the specified context.

EXTRTMETHOD void Asn1NamedEventHandler::addEventHandler (OSCTXT* pCtxt,
                                             Asn1NamedEventHandler* pHandler)
{
   if (0 == ACINFO(pCtxt)) rtCtxtInitASN1Info(pCtxt);
   if (!rtxSListFind (&ACINFO(pCtxt)->evtHndlrList, pHandler))
      rtxSListAppend (&ACINFO(pCtxt)->evtHndlrList, pHandler);
}

// removeEventHandler : This static method removes an event handler object
// from the current list within the specified context.

EXTRTMETHOD void Asn1NamedEventHandler::removeEventHandler
(OSCTXT* pCtxt, Asn1NamedEventHandler* pHandler)
{
   if (0 != ACINFO(pCtxt)) {
      rtxSListRemove (&ACINFO(pCtxt)->evtHndlrList, pHandler);
   }
}

// invoke methods: These methods loop through the list of event handler
// objects and invoke the appropriate method in each object..

EXTRTMETHOD void Asn1NamedEventHandler::invokeStartElement (OSCTXT* pCtxt,
                                                const char* name,
                                                int index)
{
   Asn1NamedEventHandler* pHandler;
   if (0 == ACINFO(pCtxt)) rtCtxtInitASN1Info(pCtxt);
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->startElement (name, index);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler::invokeEndElement (OSCTXT* pCtxt,
                                              const char* name,
                                              int index)
{
   Asn1NamedEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->endElement (name, index);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler::invokeBoolValue (OSCTXT* pCtxt, OSBOOL value)
{
   Asn1NamedEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->boolValue (value);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler::invokeIntValue (OSCTXT* pCtxt, OSINT32 value)
{
   Asn1NamedEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->intValue (value);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler::invokeUIntValue (OSCTXT* pCtxt, OSUINT32 value)
{
   Asn1NamedEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->uIntValue (value);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler::invokeInt64Value (OSCTXT* pCtxt, OSINT64 value)
{
   Asn1NamedEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->int64Value (value);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler::invokeUInt64Value (OSCTXT* pCtxt, OSUINT64 value)
{
   Asn1NamedEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->uInt64Value (value);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler::invokeBitStrValue (OSCTXT* pCtxt,
                                               OSUINT32 numbits,
                                               const OSOCTET* data)
{
   Asn1NamedEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->bitStrValue (numbits, data);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler::invokeOctStrValue (OSCTXT* pCtxt,
                                               OSUINT32 numocts,
                                               const OSOCTET* data)
{
   Asn1NamedEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->octStrValue (numocts, data);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler::invokeCharStrValue (OSCTXT* pCtxt,
                                                const char* value)
{
   Asn1NamedEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->charStrValue (value);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler::invokeCharStrValue (OSCTXT* pCtxt,
                                                OSUINT32 nchars,
                                                const OSUTF8CHAR* data)
{
   Asn1NamedEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->charStrValue (nchars, data);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler::invokeCharStrValue (OSCTXT* pCtxt,
                                                OSUINT32 nchars,
                                                OSUNICHAR* data)
{
   Asn1NamedEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->charStrValue (nchars, data);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler::invokeCharStrValue (OSCTXT* pCtxt,
                                                OSUINT32 nchars,
                                                OS32BITCHAR* data)
{
   Asn1NamedEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->charStrValue (nchars, data);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler::invokeNullValue (OSCTXT* pCtxt)
{
   Asn1NamedEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->nullValue ();
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler::invokeOidValue (OSCTXT* pCtxt,
                                            OSUINT32 numSubIds,
                                            OSUINT32* pSubIds)
{
   Asn1NamedEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->oidValue (numSubIds, pSubIds);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler::invokeRealValue (OSCTXT* pCtxt, double value)
{
   Asn1NamedEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->realValue (value);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler::invokeEnumValue
(OSCTXT* pCtxt, OSUINT32 value, const OSUTF8CHAR* text)
{
   Asn1NamedEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->enumValue (value, text);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void Asn1NamedEventHandler::invokeOpenTypeValue (OSCTXT* pCtxt,
                                                 OSUINT32 numocts,
                                                 const OSOCTET* data)
{
   Asn1NamedEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pCtxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedEventHandler*) pListNode->data;
      pHandler->openTypeValue (numocts, data);
      pListNode = pListNode->next;
   }
}

// Error handler methods

// setErrorHandler : This static method sets an error handler within
// the specified context.

EXTRTMETHOD void Asn1ErrorHandler::setErrorHandler (OSCTXT* pCtxt,
                                        Asn1ErrorHandler* pHandler)
{
   if (0 == ACINFO(pCtxt)) rtCtxtInitASN1Info(pCtxt);
   ACINFO(pCtxt)->errHndlrCB = (void*) pHandler;
}

// invoke method: This method invokes the error handler callback when
// a parsing error occurs..

EXTRTMETHOD int Asn1ErrorHandler::invoke 
(OSCTXT* pCtxt, ASN1CCB* pCCB, int stat)
{
   if (0 != ACINFO(pCtxt) && 0 != ACINFO(pCtxt)->errHndlrCB) {
      Asn1ErrorHandler* pHandler =
         (Asn1ErrorHandler*) ACINFO(pCtxt)->errHndlrCB;
      return pHandler->error (pCtxt, pCCB, stat);
   }
   else return stat;
}

// The following static method is called from within the generated
// code to call the virtual error callback method..

EXTRTMETHOD int Asn1ErrorHandler::invoke 
(OSCTXT* pCtxt, OSOCTET* ptr, int len, int stat)
{
   if (0 != ACINFO(pCtxt) && 0 != ACINFO(pCtxt)->errHndlrCB) {
      ASN1CCB ccb;
      Asn1ErrorHandler* pHandler =
         (Asn1ErrorHandler*) ACINFO(pCtxt)->errHndlrCB;
      memset (&ccb, 0, sizeof (ccb));
      ccb.ptr = ptr;
      ccb.len = len;
      return pHandler->error (pCtxt, &ccb, stat);
   }
   else return stat;
}
