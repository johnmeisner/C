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

#include "rtxmlsrc/osrtxml.h"

EXTXMLMETHOD OSUINT32 rtSaxSetMaxErrors (OSCTXT* pctxt, OSUINT32 maxErrors)
{
   OSXMLCtxtInfo* xmlCtxt;
   OSUINT32 prevMaxErr;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   xmlCtxt = ((OSXMLCtxtInfo*)pctxt->pXMLInfo);

   prevMaxErr = xmlCtxt->maxSaxErrors;
   xmlCtxt->maxSaxErrors = maxErrors;
   return prevMaxErr;
}

EXTXMLMETHOD OSUINT32 rtSaxGetMaxErrors (OSCTXT* pctxt)
{
   OSXMLCtxtInfo* xmlCtxt;
   OSRTASSERT (0 != pctxt->pXMLInfo);
   xmlCtxt = ((OSXMLCtxtInfo*)pctxt->pXMLInfo);
   return xmlCtxt->maxSaxErrors;
}

EXTXMLMETHOD OSBOOL rtSaxIncErrors (OSCTXT* pctxt)
{
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
   OSUINT32 maxErrors, errorsCnt;

   if (0 == ctxtInfo) return FALSE;

   maxErrors = ctxtInfo->maxSaxErrors;
   errorsCnt = ctxtInfo->errorsCnt;

   ctxtInfo->errorsCnt++;

   if (maxErrors == 0) { /* unlimited? */
      return TRUE;
   }

   return (OSBOOL)(errorsCnt < maxErrors);
}


