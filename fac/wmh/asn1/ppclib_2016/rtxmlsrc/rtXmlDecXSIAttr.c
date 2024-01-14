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

/* Decode XSI namespace attributes */

EXTXMLMETHOD int rtXmlDecXSIAttr
   (OSCTXT* pctxt, const OSUTF8CHAR* attrName, const OSUTF8CHAR* attrValue)
{
   int stat = 1; /* zero - decoded OK, negative - NOK, > 0 - skipped */

   if (IS_XSIATTR (attrName)) {
      rtXmlSetEncXSINamespace (pctxt, TRUE);
      if (rtXmlStrCmpAsc (attrName, "xsi:schemaLocation")) {
         stat = rtXmlSetSchemaLocation (pctxt, attrValue);
      }
      else if (rtXmlStrCmpAsc (attrName, "xsi:noNamespaceSchemaLocation")) {
         stat = rtXmlSetNoNSSchemaLocation (pctxt, attrValue);
      }
      stat = 0;
   }
   else if (rtXmlStrnCmpAsc (attrName, "SOAP-ENV:", 9)) {
      stat = 0;
   }

   return stat;
}

