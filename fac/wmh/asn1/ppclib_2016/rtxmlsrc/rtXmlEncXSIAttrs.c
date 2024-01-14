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

/* Encode XSI namespace attributes: note that the attributes will be
   cleared from the context after encoding.  This is to prevent the
   attributes from being encode twice. */

EXTXMLMETHOD int rtXmlEncXSIAttrs (OSCTXT* pctxt, OSBOOL needXSI)
{
   size_t valueLen;
   int stat = 0;
   OSXMLCtxtInfo* pXMLInfo = pctxt->pXMLInfo;
   OSRTASSERT (0 != pXMLInfo);

   /* Encode XSI schema instance namespace attribute */

   if (needXSI || 0 != pXMLInfo->schemaLocation ||
       0 != pXMLInfo->noNSSchemaLoc || 0 != pXMLInfo->xsiTypeAttr ||
       rtxCtxtTestFlag (pctxt, OSXSIATTR))
   {
      rtXmlNSAddPrefixLink (pctxt, OSUTF8("xsi"), OSUTF8(OSXSINSURI),
         pXMLInfo->nsURITable.data, pXMLInfo->nsURITable.nrows);

      stat = rtXmlEncUTF8Attr2
         (pctxt, OSUTF8("xmlns:xsi"), 9, OSUTF8(OSXSINSURI), OSXSINSURI_LEN);
      if (0 != stat) return LOG_RTERR (pctxt, stat);

      /* Clear flag indicating XSI namespace should be encoded */
      rtXmlSetEncXSINamespace (pctxt, FALSE);
   }

   /* Encode schema location attribute if present */

   if (0 != pXMLInfo->schemaLocation) {
      valueLen  = OSUTF8LEN (pXMLInfo->schemaLocation);

      stat = rtXmlEncUTF8Attr2
         (pctxt, OSUTF8("xsi:schemaLocation"), 18,
          pXMLInfo->schemaLocation, valueLen);

      if (0 != stat) return LOG_RTERR (pctxt, stat);
      else {
         rtxMemFreePtr (pctxt, pXMLInfo->schemaLocation);
         pXMLInfo->schemaLocation = 0;
      }
   }

   /* Encode no namespace schema location attribute if present */

   if (0 != pXMLInfo->noNSSchemaLoc) {
      valueLen  = OSUTF8LEN (pXMLInfo->noNSSchemaLoc);

      stat = rtXmlEncUTF8Attr2
         (pctxt, OSUTF8("xsi:noNamespaceSchemaLocation"), 29,
          pXMLInfo->noNSSchemaLoc, valueLen);

      if (0 != stat) return LOG_RTERR (pctxt, stat);
      else {
         rtxMemFreePtr (pctxt, pXMLInfo->noNSSchemaLoc);
         pXMLInfo->noNSSchemaLoc = 0;
      }
   }

   /* Encode xsi:type attribute if present */

   if (0 != pXMLInfo->xsiTypeAttr) {
      stat = rtXmlEncXSITypeAttr (pctxt, pXMLInfo->xsiTypeAttr);

      if (0 != stat) return LOG_RTERR (pctxt, stat);
      else {
         rtxMemFreePtr (pctxt, pXMLInfo->xsiTypeAttr);
         pXMLInfo->xsiTypeAttr = 0;
      }
   }

   return stat;
}

