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

#ifndef _COMPACT
#include "rtxmlsrc/osrtxml.h"
#include "rtxmlsrc/rtXmlNamespace.h"

EXTXMLMETHOD int rtXmlPrintNSAttrs (const char* name, const OSRTDList* data)
{
   if (data->count > 0) {
     OSRTDListNode* pNode;
     OSXMLNamespace* pNS;

     for (pNode = data->head; 0 != pNode; pNode = pNode->next) {
        pNS = (OSXMLNamespace*) pNode->data;
        if (OS_ISEMPTY (pNS->prefix)) {
           printf ("%s.xmlns = \"%s\"\n", name, pNS->uri);
        }
        else {
           printf ("%s.xmlns:%s = \"%s\"\n", name, pNS->prefix, pNS->uri);
        }
     }
   }

   return 0;
}

#endif

