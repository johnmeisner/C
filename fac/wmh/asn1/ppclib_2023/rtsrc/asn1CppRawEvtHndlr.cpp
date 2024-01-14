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
#include "asn1CppRawEvtHndlr.h"
#include "rtxsrc/OSRTMsgBuf.h"

EXTRTMETHOD void Asn1RawEventHandler::addEventHandler(OSCTXT* pctxt)
{
   if (ACINFO(pctxt) == 0) rtCtxtInitASN1Info(pctxt);

   ACINFO(pctxt)->pRawEvtHndlr = this;
}

EXTRTMETHOD void Asn1RawEventHandler::addEventHandler(OSRTMessageBuffer& buffer)
{
   addEventHandler(buffer.getCtxtPtr());
}

EXTRTMETHOD ASN1RawEventResp Asn1RawEventHandler::invokeComponent(
   OSCTXT* pctxt, OSSIZE componentID,
   OSBOOL nestedValues)
{
   if (ACINFO(pctxt) == 0 || ACINFO(pctxt)->pRawEvtHndlr == 0)
      return ASN1RER_PARSE;

   return ((Asn1RawEventHandler*)(ACINFO(pctxt)->pRawEvtHndlr))->
      component(pctxt, componentID, nestedValues);
}


EXTRTMETHOD void Asn1RawEventHandler::invokeEndComponent(
   OSCTXT* pctxt, OSSIZE componentID)
{
   Asn1RawEventHandler* pHandler;

   if (ACINFO(pctxt) == 0 || ACINFO(pctxt)->pRawEvtHndlr == 0)
      return;

   pHandler = (Asn1RawEventHandler*)(ACINFO(pctxt)->pRawEvtHndlr);
   pHandler->endComponent(pctxt, componentID);
}


EXTRTMETHOD ASN1RawEventResp Asn1RawEventHandler::invokeActualType(
   OSCTXT* pctxt, OSSIZE actualTypeID, OSSIZE len)
{
   Asn1RawEventHandler* pHandler;

   if (ACINFO(pctxt) == 0 || ACINFO(pctxt)->pRawEvtHndlr == 0)
      return ASN1RER_PARSE;

   pHandler = (Asn1RawEventHandler*)(ACINFO(pctxt)->pRawEvtHndlr);
   return pHandler->actualType(pctxt, actualTypeID, len);
}


EXTRTMETHOD void Asn1RawEventHandler::removeEventHandler(OSCTXT* pctxt)
{
   ACINFO(pctxt)->pRawEvtHndlr = NULL;
}
