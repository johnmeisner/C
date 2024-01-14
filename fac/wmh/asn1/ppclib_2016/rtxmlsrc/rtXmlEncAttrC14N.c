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

#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxmlsrc/osrtxml.hh"

static int c14nAttrsCmp (const void* a1, const void* b1, const void* sortCtxt)
{
   const OSXMLSortedAttrOffset* a = (const OSXMLSortedAttrOffset*) a1;
   const OSXMLSortedAttrOffset* b = (const OSXMLSortedAttrOffset*) b1;
   const OSCTXT* pctxt = (const OSCTXT*) sortCtxt;
   const OSOCTET* nameA = pctxt->buffer.data + a->offset;
   const OSOCTET* nameB = pctxt->buffer.data + b->offset;
   OSBOOL aIsNs = FALSE;
   OSBOOL bIsNs = FALSE;

   if ((a->prefixLength == 5 ||
        (a->prefixLength == 0 && a->nameLength == 5)) &&
        OSCRTLMEMCMP (nameA, "xmlns", 5) == 0)
      aIsNs = TRUE;

   if ((b->prefixLength == 5 ||
        (b->prefixLength == 0 && b->nameLength == 5)) &&
        OSCRTLMEMCMP (nameB, "xmlns", 5) == 0)
      bIsNs = TRUE;

   if (aIsNs && !bIsNs)
      return -1;
   else if (!aIsNs && bIsNs)
      return 1;
   else if (aIsNs && bIsNs) {
      /* both is namespaces */
      if (a->prefixLength == 0) {
         if (b->prefixLength != 0)
            return -1;
         else
            return 0; /* ??? duplicate default namespace */
      }
      else if (b->prefixLength == 0)
         return 1;
      else {
         nameA += a->prefixLength + 1;
         nameB += b->prefixLength + 1;

         if (a->nameLength < b->nameLength) {
            int res = OSCRTLMEMCMP (nameA, nameB, a->nameLength);
            if (res == 0) return -1;
            else return res;
         }
         else if (a->nameLength > b->nameLength) {
            int res = OSCRTLMEMCMP (nameA, nameB, b->nameLength);
            if (res == 0) return 1;
            else return res;
         }
         else
            return OSCRTLMEMCMP (nameA, nameB, a->nameLength);
      }
   }
   else if (a->prefixLength == 0) {
      if (b->prefixLength != 0)
         return -1;
      else { /* default namespace or unqialified */
         if (a->nameLength < b->nameLength) {
            int res = OSCRTLMEMCMP (nameA, nameB, a->nameLength);
            if (res == 0) return -1;
            else return res;
         }
         else if (a->nameLength > b->nameLength) {
            int res = OSCRTLMEMCMP (nameA, nameB, b->nameLength);
            if (res == 0) return 1;
            else return res;
         }
         else
            return OSCRTLMEMCMP (nameA, nameB, a->nameLength);
      }
   }
   else if (b->prefixLength == 0)
      return 1;
   else {
      if (a->prefixLength == b->prefixLength &&
          OSCRTLMEMCMP (nameA, nameB, a->prefixLength) == 0)
      {
         /* prefixes is equal; compare names */

         nameA += a->prefixLength + 1;
         nameB += b->prefixLength + 1;

         if (a->nameLength < b->nameLength) {
            int res = OSCRTLMEMCMP (nameA, nameB, a->nameLength);
            if (res == 0) return -1;
            else return res;
         }
         else if (a->nameLength > b->nameLength) {
            int res = OSCRTLMEMCMP (nameA, nameB, b->nameLength);
            if (res == 0) return 1;
            else return res;
         }
         else
            return OSCRTLMEMCMP (nameA, nameB, a->nameLength);
      }
      else {
         const OSUTF8CHAR* pUriA = rtXmlNSLookupPrefixFrag ((OSCTXT*) pctxt,
            nameA, a->prefixLength);
         const OSUTF8CHAR* pUriB = rtXmlNSLookupPrefixFrag ((OSCTXT*) pctxt,
            nameB, b->prefixLength);
         int res;

         /* xml:lang or incorrect setted prefix is not presented in ns list */
         if (!pUriA && pUriB)
            return -1;
         else if (pUriA && !pUriB)
            return 1;
         else if  (!pUriA && !pUriB)
            return 0;

         /* namespace prefixes are presented always */
         res = rtxUTF8Strcmp (pUriA, pUriB);
         if (res != 0)
            return res;
         else {
            nameA += a->prefixLength + 1;
            nameB += b->prefixLength + 1;

            if (a->nameLength < b->nameLength) {
               res = OSCRTLMEMCMP (nameA, nameB, a->nameLength);
               if (res == 0) return -1;
               else return res;
            }
            else if (a->nameLength > b->nameLength) {
               res = OSCRTLMEMCMP (nameA, nameB, b->nameLength);
               if (res == 0) return 1;
               else return res;
            }
            else
               return OSCRTLMEMCMP (nameA, nameB, a->nameLength);
         }
      }
   }

   /* return 0; */
}

EXTXMLMETHOD int rtXmlEncAttrC14N (OSCTXT* pctxt)
{
   OSXMLCtxtInfo* pCtxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
   OSRTDListNode* pNode = 0;
   OSXMLSortedAttrOffset* pAttrOffs = 0;
   size_t attrOffset = pCtxtInfo->attrStartPos + 1;
   const char* name = (const char*) pctxt->buffer.data + attrOffset;
   const char* p = name;

   rtxDListAllocNodeAndData
      (pctxt, OSXMLSortedAttrOffset, &pNode, &pAttrOffs);
   if (!pNode) return LOG_RTERR (pctxt, RTERR_NOMEM);

   pAttrOffs->offset = attrOffset;
   pAttrOffs->length = pctxt->buffer.byteIndex - attrOffset;

   for (; *p != ':' && *p != '='; p++);

   if (*p == ':') {
      pAttrOffs->prefixLength = (size_t) (p - name);
      p++;
      for (name = p; *p != '='; p++);
      pAttrOffs->nameLength = (size_t) (p - name);
   }
   else {
      pAttrOffs->prefixLength = 0;
      pAttrOffs->nameLength = (size_t) (p - name);
   }

   rtxDListInsertNodeSorted (&pCtxtInfo->sortedAttrList, pNode,
                             c14nAttrsCmp, pctxt);

   pCtxtInfo->attrStartPos = pctxt->buffer.byteIndex;

   return 0;
}

EXTXMLMETHOD int rtXmlEncStartAttrC14N (OSCTXT* pctxt)
{
   /* substitute buffer for attributes */
   OSXMLCtxtInfo* pCtxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   OSCRTLSAFEMEMCPY (&pCtxtInfo->savedBuffer, sizeof(OSRTBuffer),
         &pctxt->buffer, sizeof (OSRTBuffer));

   pCtxtInfo->savedFlags = pctxt->flags;
   pCtxtInfo->attrStartPos = 0;

   if (!pCtxtInfo->attrsBuff) {
      /* allocate atributes buffer */
      pCtxtInfo->attrsBuffSize = 512;
      pCtxtInfo->attrsBuff =
         rtxMemAllocArray (pctxt, pCtxtInfo->attrsBuffSize, OSOCTET);
      if (!pCtxtInfo->attrsBuff)
         return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
   }

   pctxt->flags |= OSDISSTRM; /* ??? */
   pctxt->buffer.data = pCtxtInfo->attrsBuff;
   pctxt->buffer.size = pCtxtInfo->attrsBuffSize;
   pctxt->buffer.dynamic = TRUE;
   pctxt->buffer.byteIndex = 0;
   pctxt->buffer.bitOffset = 8;

   return 0;
}

EXTXMLMETHOD int rtXmlEncEndAttrC14N (OSCTXT* pctxt)
{
   int stat;

   /* restore buffer */
   OSXMLCtxtInfo* pCtxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
   OSRTDListNode* pNode = pCtxtInfo->sortedAttrList.head;

   pCtxtInfo->attrsBuff = pctxt->buffer.data;
   pCtxtInfo->attrsBuffSize = pctxt->buffer.size;

   OSCRTLSAFEMEMCPY (&pctxt->buffer, sizeof(OSRTBuffer),
         &pCtxtInfo->savedBuffer, sizeof (OSRTBuffer));
   pctxt->flags = pCtxtInfo->savedFlags;

   while (pNode) {
      OSXMLSortedAttrOffset* pAttrOffs =
         (OSXMLSortedAttrOffset*) pNode->data;
      const OSOCTET* attr = pCtxtInfo->attrsBuff + pAttrOffs->offset - 1;

      stat = rtXmlWriteChars (pctxt, attr, pAttrOffs->length + 1);
      if (0 != stat) {
         return LOG_RTERR (pctxt, stat);
      }
      pNode = pNode->next;
   }

   rtxDListFreeNodes (pctxt, &pCtxtInfo->sortedAttrList);

   return 0;
}

