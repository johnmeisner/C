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
#include "asn1CRawEvtHndlr.h"

EXTRTMETHOD void rtAddRawEventHandler(OSCTXT* pctxt, Asn1CRawEventHandler* pHandler)
{
   if (ACINFO(pctxt) == 0) rtCtxtInitASN1Info(pctxt);

   ACINFO(pctxt)->pRawEvtHndlr = pHandler;
}


EXTRTMETHOD ASN1RawEventResp rtInvokeComponent(OSCTXT* pctxt, OSSIZE componentID,
   OSBOOL nestedValues)
{
   if (ACINFO(pctxt) == 0 || ACINFO(pctxt)->pRawEvtHndlr == 0 )
      return ASN1RER_PARSE;

   return ((Asn1CRawEventHandler*) (ACINFO(pctxt)->pRawEvtHndlr))->
      component(pctxt, componentID, nestedValues);
}


EXTRTMETHOD void rtInvokeEndComponent(OSCTXT* pctxt, OSSIZE componentID)
{
   Asn1CRawEventHandler* pHandler;

   if (ACINFO(pctxt) == 0 || ACINFO(pctxt)->pRawEvtHndlr == 0)
      return;

   pHandler = (Asn1CRawEventHandler*)(ACINFO(pctxt)->pRawEvtHndlr);
   if (pHandler->endComponent)
      pHandler->endComponent(pctxt, componentID);
}


EXTRTMETHOD ASN1RawEventResp rtInvokeActualType(OSCTXT* pctxt,
                                                OSSIZE actualTypeID,
                                                OSSIZE len)
{
   Asn1CRawEventHandler* pHandler;

   if (ACINFO(pctxt) == 0 || ACINFO(pctxt)->pRawEvtHndlr == 0)
      return ASN1RER_PARSE;

   pHandler = (Asn1CRawEventHandler*)(ACINFO(pctxt)->pRawEvtHndlr);
   if (pHandler->actualType)
      return pHandler->actualType(pctxt, actualTypeID, len);
   else return ASN1RER_PARSE;
}



/** Remove the raw event handler from given context. */
EXTRTMETHOD void rtRemoveRawEventHandler(OSCTXT* pctxt)
{
   ACINFO(pctxt)->pRawEvtHndlr = NULL;
}
