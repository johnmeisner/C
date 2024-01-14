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

#include "rtxmlsrc/osrtxml.h"
#include "rtxmlsrc/rtXmlNamespace.h"

/*
 * Returns namespace count if successful or negative status code on failure.
 */
EXTXMLMETHOD int rtXmlDecNSAttr
(OSCTXT* pctxt, const OSUTF8CHAR* attrName, const OSUTF8CHAR* attrValue,
 OSRTDList* pNSAttrs, const OSUTF8CHAR* nsTable[], OSUINT32 nsTableRowCount)
{
   int stat;
   const OSUTF8CHAR* nsPrefix;

   /* Check arguments */
   if (0 == attrName || 0 == attrValue)
      return LOG_RTERR (pctxt, RTERR_NULLPTR);

   if (!rtXmlStrnCmpAsc (attrName, "xmlns", 5))
      return LOG_RTERR (pctxt, RTERR_BADVALUE);

   nsPrefix = (attrName[5] == ':') ? &attrName[6] : 0;

   /* If namespace list not provided, use list in context */
   if (0 == pNSAttrs) {
      OSXMLCtxtInfo* pXMLCtxtInfo = (OSXMLCtxtInfo*) pctxt->pXMLInfo;
      if (0 == pXMLCtxtInfo) return RTERR_NOTINIT;

      pNSAttrs = &pXMLCtxtInfo->namespaceList;
   }
   else {
      /* Add namespace prefix link to the context stack.  This is used by
         the decoder to determine the namespace of incoming elements */
      stat = rtXmlNSAddPrefixLink
         (pctxt, nsPrefix, attrValue, nsTable, nsTableRowCount);

      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   /* Add record to decoded namespace list */
   rtXmlNSAddNamespace (pctxt, pNSAttrs, nsPrefix, attrValue);

   return 0;
}


