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

#include "rtxsrc/rtxDList.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxmlsrc/osrtxml.hh"

#ifndef _COMPACT
static int checkElement (OSCTXT* pctxt, const OSUTF8CHAR* elemName)
{
   /* Verify element name on the stack is the element being terminated */
   const OSUTF8CHAR* stkElemName;
   OSRTASSERT (pctxt->elemNameStack.count > 0);
   stkElemName = OSUTF8 (pctxt->elemNameStack.tail->data);
   if (!rtxUTF8StrEqual (elemName, stkElemName)) {
      rtxErrAddStrParm (pctxt, (const char*)stkElemName);
      rtxErrAddStrParm (pctxt, (const char*)elemName);
      return LOG_RTERR (pctxt, RTERR_IDNOTFOU);
   }
   return 0;
}
#endif

EXTXMLMETHOD int rtXmlEncEndElement (OSCTXT* pctxt,
                        const OSUTF8CHAR* elemName,
                        OSXMLNamespace* pNS)
{
   int stat = 0;
   OSBOOL canUseEmptyElemTag; /* true if can use <tagname/> */
   OSRTDList* pEncNSList;
   OSRTDListNode* pNode;
   OSXMLNamespace* pEncNS;
   OSXMLCtxtInfo* pXMLCtxtInfo = (OSXMLCtxtInfo*) pctxt->pXMLInfo;
   OSRTASSERT (0 != pXMLCtxtInfo);
   pEncNSList = &pXMLCtxtInfo->encodedNSList;

   /* If current state is OSXMLSTART, can end element by replacing '>'  */
   /* at end with '/>'                                                  */

   if (0 != elemName && 0 != *elemName) {
#ifndef _COMPACT
      stat = checkElement (pctxt, elemName);
      if (0 != stat) LOG_RTERR (pctxt, stat);
#endif
      /* XER: option to use empty tags is given.  In fact, it is required for */
      /* canonical XER.  XML: canonical XML disallows empty tags */
      canUseEmptyElemTag = pctxt->state == OSXMLSTART &&
         ( rtxCtxtTestFlag(pctxt, OSASN1XER) ||
            !rtxCtxtTestFlag (pctxt, OSXMLC14N) );

      if (canUseEmptyElemTag) {
         /* start tag may be closed as empty tag */
         if ((pctxt->flags & OSTERMSTART) == 0) {
            if (pctxt->buffer.byteIndex > 0) {
               if ( pXMLCtxtInfo->encoding == OSXMLUTF8 ||
                  pXMLCtxtInfo->encoding == OSXMLLATIN1 )
               {
                  /* back up one byte */
                  pctxt->buffer.byteIndex--;
               }
               else {
                  /* backup 2 bytes for UTF-16 */
                  pctxt->buffer.byteIndex -= 2;
               }
            } else stat = RTERR_XMLSTATE;

         }

         if (rtXmlCheckBuffer (pctxt, 3) == 0) {
               stat = rtXmlPutChar (pctxt, '/');
               stat = rtXmlPutChar (pctxt, '>');
               OSRTZTERM (pctxt);
         }
         else stat = RTERR_BUFOVFLW;

         if (stat < 0) return LOG_RTERRNEW (pctxt, stat);
         else {
            pctxt->flags &= ~(OSTERMSTART | OSEMPTYELEM);
            pctxt->state = OSXMLEND;
            pctxt->level--;
         }
      }
      else {
         /* Note: it is dangerous to assign pNS->prefix here.  The user code */
         /* may then use the same OSXMLNamespace struct with a different */
         /* namespace list and thus use the prefix without declaring it. */
         /* Thus, we declare prefix here. */
         const OSUTF8CHAR* prefix = pNS == 0 ? 0 : pNS->prefix;
         size_t nsPrefixLen;
         size_t specChars;
         size_t elemLen = rtxUTF8LenBytes (elemName);

         if ( pNS != 0 && !OS_ISEMPTY(pNS->uri) && prefix == 0 )
         {
            /* lookup the prefix; one should be in scope from the start elem */
            prefix = rtXmlNSGetPrefix (pctxt, pNS->uri);
         }
         nsPrefixLen = (0 != prefix) ? rtxUTF8LenBytes(prefix) : 0;
         specChars = (0 != nsPrefixLen) ? 5 : 4;

         if (pctxt->state == OSXMLSTART && pctxt->flags & OSTERMSTART) {
            stat = rtXmlEncTermStartElement (pctxt);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }

         pctxt->level--;

         /* If last element was an end element or comment, indent */
         if (OSXMLEND == pctxt->state ||
             OSXMLCOMMENT == pctxt->state) {
            stat = rtXmlEncIndent (pctxt);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }
         pctxt->state = OSXMLEND;

         /* Verify element will fit in encode buffer */

         stat = rtXmlCheckBuffer (pctxt, elemLen + nsPrefixLen + specChars);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         /* Copy data to buffer */

         stat = rtXmlPutChar (pctxt, '<');
         stat = rtXmlPutChar (pctxt, '/');

         if (0 != nsPrefixLen) {
            stat = rtXmlWriteChars (pctxt, prefix, nsPrefixLen);
            stat = rtXmlPutChar (pctxt, ':');
         }
         stat = rtXmlWriteChars (pctxt, elemName, elemLen);

         stat = rtXmlPutChar (pctxt, '>');
         OSRTZTERM (pctxt);
      }

      /* Pop entry from namespace stack */
      stat = rtXmlNSPop (pctxt);
      if (stat < 0) return LOG_RTERR (pctxt, stat);
#ifndef _COMPACT
      rtxDListFreeNode
         (pctxt, &pctxt->elemNameStack, pctxt->elemNameStack.tail);
#endif
      /* remove namespace entry from encoded list */
      for (;;) {
         pNode = pEncNSList->tail;
         if (0 != pNode) {
            pEncNS = (OSXMLNamespace*) pNode->data;
            rtxDListFreeTailNode (pctxt, pEncNSList);
            if (pEncNS == 0 ) break;
         }
         else break;
      }

   }
   return 0;
}

