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

EXTXMLMETHOD int rtXmlEncEmptyElement (OSCTXT* pctxt,
                          const OSUTF8CHAR* elemName,
                          OSXMLNamespace* pNS,
                          OSRTDList* pNSAttrs,
                          OSBOOL terminate)
{
   int stat = 0;

   if (0 != elemName) {
      stat = rtXmlEncStartElement (pctxt, elemName, pNS, pNSAttrs, FALSE);

      if ( stat == 0 )
         stat = rtXmlEncNSAttrs(pctxt, pNSAttrs);

      if (0 == stat && terminate) {
         stat = rtXmlEncEndElement (pctxt, elemName, pNS);
      }
   }

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}

