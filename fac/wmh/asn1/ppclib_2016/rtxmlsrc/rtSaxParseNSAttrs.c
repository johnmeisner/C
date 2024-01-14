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
/*
 * Returns namespace count if successful or negative status code on failure.
 */
EXTXMLMETHOD int rtSaxParseNSAttrs
(OSCTXT* pctxt, const OSUTF8CHAR* const* attrs, OSRTDList* pNSAttrs,
 const OSUTF8CHAR* nsTable[], OSUINT32 nsTableRowCount)
{
   int count = 0, stat;
   if (0 != attrs) {
      size_t i;
      const OSUTF8CHAR* nsPrefix;
      for (i = 0; 0 != attrs[i]; i += 2) {
         if (rtXmlStrnCmpAsc (attrs[i], "xmlns", 5)) {
            if (0 == count++) {
               /* Push new namespace level onto stack */
               rtXmlNSPush (pctxt);
            }
            nsPrefix = (attrs[i][5] == ':') ?
               &attrs[i][6] : 0;

            stat = rtXmlNSAddPrefixLink
               (pctxt, nsPrefix, attrs[i+1], nsTable, nsTableRowCount);

            if (0 != stat) return LOG_RTERR (pctxt, stat);

            /* If output namespace list pointer provided, add a record  */
            /* to this list..                                           */
            if (0 != pNSAttrs) {
               OSXMLNamespace* pNS = rtxMemAllocType (pctxt, OSXMLNamespace);
               if (0 == pNS) return LOG_RTERR (pctxt, RTERR_NOMEM);

               pNS->prefix = (0 != nsPrefix) ?
                  rtxUTF8Strdup (pctxt, nsPrefix) : 0;

               pNS->uri = rtxUTF8Strdup (pctxt, attrs[i+1]);

               rtxDListAppend (pctxt, pNSAttrs, (void*)pNS);
            }
         }
      }
   }
   return count;
}


