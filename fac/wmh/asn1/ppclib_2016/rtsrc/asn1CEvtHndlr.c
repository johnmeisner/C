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

/* asn1CEvtHndlr.c - this file contains implementations of the functions
   necessary to register new event handlers..*/

#include "asn1CEvtHndlr.h"

/* addEventHandler : This function adds an event handler object to
   the current list within the specified context. */

EXTRTMETHOD void rtAddEventHandler
(OSCTXT* pctxt, Asn1NamedCEventHandler* pHandler)
{
   if (0 == ACINFO(pctxt)) rtCtxtInitASN1Info(pctxt);
   if (!rtxSListFind (&ACINFO(pctxt)->evtHndlrList, pHandler))
      rtxSListAppend (&ACINFO(pctxt)->evtHndlrList, pHandler);
}

/* removeEventHandler : This function removes an event handler object
   from the current list within the specified context. */

EXTRTMETHOD void rtRemoveEventHandler
(OSCTXT* pctxt, Asn1NamedCEventHandler* pHandler)
{
   if (0 != ACINFO(pctxt)) {
      rtxSListRemove (&ACINFO(pctxt)->evtHndlrList, pHandler);
   }
}

/* invoke methods: These methods loop through the list of event handler
   objects and invoke the appropriate method in each object.. */

EXTRTMETHOD void rtInvokeStartElement
(OSCTXT* pctxt, const char* name, OSSIZE idx)
{
   Asn1NamedCEventHandler* pHandler;
   OSRTSListNode* pListNode;

   if (0 == ACINFO(pctxt)) rtCtxtInitASN1Info(pctxt);
   pListNode = ACINFO(pctxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedCEventHandler*) pListNode->data;
      pHandler->startElement (name, idx);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void rtInvokeEndElement
(OSCTXT* pctxt, const char* name, OSSIZE idx)
{
   Asn1NamedCEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pctxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedCEventHandler*) pListNode->data;
      pHandler->endElement (name, idx);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void rtInvokeBoolValue (OSCTXT* pctxt, OSBOOL value)
{
   Asn1NamedCEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pctxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedCEventHandler*) pListNode->data;
      pHandler->boolValue (value);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void rtInvokeIntValue (OSCTXT* pctxt, OSINT32 value)
{
   Asn1NamedCEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pctxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedCEventHandler*) pListNode->data;
      pHandler->intValue (value);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void rtInvokeUIntValue (OSCTXT* pctxt, OSUINT32 value)
{
   Asn1NamedCEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pctxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedCEventHandler*) pListNode->data;
      pHandler->uIntValue (value);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void rtInvokeInt64Value (OSCTXT* pctxt, OSINT64 value)
{
   Asn1NamedCEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pctxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedCEventHandler*) pListNode->data;
      pHandler->int64Value (value);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void rtInvokeUInt64Value (OSCTXT* pctxt, OSUINT64 value)
{
   Asn1NamedCEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pctxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedCEventHandler*) pListNode->data;
      pHandler->uInt64Value (value);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void rtInvokeBitStrValue
(OSCTXT* pctxt, OSSIZE numbits, const OSOCTET* data)
{
   Asn1NamedCEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pctxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedCEventHandler*) pListNode->data;
      pHandler->bitStrValue (numbits, data);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void rtInvokeOctStrValue
(OSCTXT* pctxt, OSSIZE numocts, const OSOCTET* data)
{
   Asn1NamedCEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pctxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedCEventHandler*) pListNode->data;
      pHandler->octStrValue (numocts, data);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void rtInvokeCharStrValue (OSCTXT* pctxt, const char* value)
{
   Asn1NamedCEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pctxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedCEventHandler*) pListNode->data;
      pHandler->charStrValue (value);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void rtInvokeCharStr16BitValue
(OSCTXT* pctxt, OSSIZE nchars, OSUNICHAR* data)
{
   Asn1NamedCEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pctxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedCEventHandler*) pListNode->data;
      pHandler->charStrValue16Bit (nchars, data);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void rtInvokeCharStr32BitValue
(OSCTXT* pctxt, OSSIZE nchars, OS32BITCHAR* data)
{
   Asn1NamedCEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pctxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedCEventHandler*) pListNode->data;
      pHandler->charStrValue32Bit (nchars, data);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void rtInvokeNullValue (OSCTXT* pctxt)
{
   Asn1NamedCEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pctxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedCEventHandler*) pListNode->data;
      pHandler->nullValue ();
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void rtInvokeOidValue
(OSCTXT* pctxt, OSUINT32 numSubIds, OSUINT32* pSubIds)
{
   Asn1NamedCEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pctxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedCEventHandler*) pListNode->data;
      pHandler->oidValue (numSubIds, pSubIds);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void rtInvokeRealValue (OSCTXT* pctxt, double value)
{
   Asn1NamedCEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pctxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedCEventHandler*) pListNode->data;
      pHandler->realValue (value);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void rtInvokeEnumValue
(OSCTXT* pctxt, OSUINT32 value, const OSUTF8CHAR* text)
{
   Asn1NamedCEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pctxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedCEventHandler*) pListNode->data;
      pHandler->enumValue (value, text);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void rtInvokeOpenTypeValue
(OSCTXT* pctxt, OSSIZE numocts, const OSOCTET* data)
{
   Asn1NamedCEventHandler* pHandler;
   OSRTSListNode* pListNode = ACINFO(pctxt)->evtHndlrList.head;

   while (pListNode) {
      pHandler = (Asn1NamedCEventHandler*) pListNode->data;
      pHandler->openTypeValue (numocts, data);
      pListNode = pListNode->next;
   }
}

EXTRTMETHOD void rtSetErrorHandler (OSCTXT *pctxt, rtErrorHandler pErrCB)
{
   ACINFO(pctxt)->errHndlrCB = (void*)pErrCB;
}

EXTRTMETHOD void rtRemoveErrorHandler (OSCTXT *pctxt)
{
   ACINFO(pctxt)->errHndlrCB = 0;
}

EXTRTMETHOD int rtInvokeErrorHandler
(OSCTXT *pctxt, ASN1CCB *pCCB, int stat)
{
   if (0 != ACINFO(pctxt)->errHndlrCB) {
      rtErrorHandler hnd = (rtErrorHandler)(ACINFO(pctxt)->errHndlrCB);
      return (*hnd)(pctxt, pCCB, stat);
   }
   return 0;
}
