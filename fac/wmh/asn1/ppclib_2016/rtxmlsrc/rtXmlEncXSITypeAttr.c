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

#include "rtxsrc/rtxErrCodes.h"
#include "rtxmlsrc/osrtxml.h"

static const OSUTF8CHAR XSI_TYPE[] = "xsi:type";
#define XSI_TYPE_LEN (sizeof(XSI_TYPE)-1)

EXTXMLMETHOD int rtXmlEncXSITypeAttr (OSCTXT* pctxt, const OSUTF8CHAR* value)
{
   int stat;

   OSXMLCtxtInfo* pXMLInfo = pctxt->pXMLInfo;
   OSRTASSERT (0 != pXMLInfo);

   /* Ensure that the XSI namespace has been declared. */
   if ( rtXmlNSGetPrefix(pctxt, OSUTF8(OSXSINSURI)) == 0 ) {
      /* The prefix was not previously declared.  Add it and encode the */
      /* declaration now. */
      rtXmlNSAddPrefixLink (pctxt, OSUTF8("xsi"), OSUTF8(OSXSINSURI),
         pXMLInfo->nsURITable.data, pXMLInfo->nsURITable.nrows);

      stat = rtXmlEncUTF8Attr2
         (pctxt, OSUTF8("xmlns:xsi"), 9, OSUTF8(OSXSINSURI), OSXSINSURI_LEN);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   stat = rtXmlEncUTF8Attr2
      (pctxt, XSI_TYPE, XSI_TYPE_LEN, value, OSUTF8LEN (value));

   return (0 != stat) ? LOG_RTERR (pctxt, stat) : 0;
}

EXTXMLMETHOD int rtXmlEncXSITypeAttr2 (OSCTXT* pctxt,
                                    const OSUTF8CHAR* typeNsUri,
                                    const OSUTF8CHAR* typeName)
{
   int stat;

   if (OS_ISEMPTY(typeNsUri) ) {
      /* no prefix needed for type */
      stat = rtXmlEncXSITypeAttr(pctxt, typeName);
   }
   else {
      /* we need a prefix for the type name */
      const OSUTF8CHAR* prefix = rtXmlNSGetPrefix(pctxt, typeNsUri);
      OSUTF8CHAR* qName;

      if ( prefix == 0 ) prefix = rtXmlNSNewPrefix(pctxt, typeNsUri, 0);

      qName = rtXmlNewQName(pctxt, typeName, prefix);

      stat = rtXmlEncXSITypeAttr(pctxt, qName);
      if (0 != stat) return LOG_RTERR (pctxt, stat);

      rtxMemFreePtr(pctxt, qName);

      /* in case we declared a new namespace prefix, make sure it gets encoded */
      stat = rtXmlEncNSAttrs(pctxt, 0);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   return  (0 != stat) ? LOG_RTERR (pctxt, stat) : 0;
}
