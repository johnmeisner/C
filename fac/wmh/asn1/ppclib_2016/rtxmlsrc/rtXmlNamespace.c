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

#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxUTF8.h"
#include "rtxsrc/rtxDynPtrArray.h"

/**
 * Search through the prefix link stack (top to bottom) for a prefix link
 * for the given URI.
 * @return The prefix link for the given uri that is closest to the top of the
 *    stack, or null if none.
 */
static OSXMLNSPfxLink*
getPrefixLinkUsingURI (OSCTXT* pctxt, const OSUTF8CHAR* uri)
{
   OSXMLCtxtInfo* pXMLCtxtInfo = (OSXMLCtxtInfo*) pctxt->pXMLInfo;
   OSXMLNSPfxLinkStackNode* pStackNode = pXMLCtxtInfo->nsPfxLinkStack.top;
   for ( ; 0 != pStackNode; pStackNode = pStackNode->next) {
      OSXMLNSPfxLink* plink = pStackNode->link;
      for ( ; 0 != plink; plink = plink->next) {
         if (0 != plink->ns.uri) {
            if (rtxUTF8StrEqual (plink->ns.uri, uri))
               return plink;
         }
         else if (plink->nsidx >= 0 &&
                  plink->nsidx < (int)pXMLCtxtInfo->nsURITable.nrows) {
            if (rtxUTF8StrEqual
                (pXMLCtxtInfo->nsURITable.data[plink->nsidx], uri)) {
               return plink;
            }
         }
      }
   }
   return 0;
}


/**
 * Return a non-empty prefix from the given namespace-prefix link node.
 * @return pointer to non-empty string or null if there is no non-empty
 *    prefix for the given link.  If the given link is null, null is returned.
 */
static const OSUTF8CHAR* getNonEmptyPrefix(OSXMLNSPfxLink* plink)
{
   if (0 != plink) {
      const OSUTF8CHAR* nsPrefix = plink->ns.prefix;

      if (!OS_ISEMPTY (nsPrefix)) {
         return nsPrefix;
      }
      else if (plink->extraPrefixes.count > 0) {
         /* all extraPrefixes are non-empty; return the first one */
         return (const OSUTF8CHAR*)plink->extraPrefixes.data[0];
      }
      /* else: no extra prefixes */
   }
   return 0;
}


static OSBOOL prefixesEqual
(const OSUTF8CHAR* prefix1, const OSUTF8CHAR* prefix2)
{
   if (OS_ISEMPTY (prefix1)) {
      /* Special case: looking for default namespace */
      return (OSBOOL)(OS_ISEMPTY (prefix2));
   }
   else return (rtxUTF8StrEqual (prefix1, prefix2));
}

EXTXMLMETHOD OSXMLNamespace* rtXmlNSAddNamespace
(OSCTXT* pctxt, OSRTDList* pNSAttrs, const OSUTF8CHAR* prefix,
 const OSUTF8CHAR* uri)
{
   OSXMLNamespace* pNS;

   /* If namespace list not provided, use list in context */
   if (0 == pNSAttrs) {
      OSXMLCtxtInfo* pXMLCtxtInfo = (OSXMLCtxtInfo*) pctxt->pXMLInfo;
      if (0 != pXMLCtxtInfo) pNSAttrs = &pXMLCtxtInfo->namespaceList;
      else return 0;
   }

   /* Allocate and populate namespace record */
   pNS = rtxMemAllocTypeZ (pctxt, OSXMLNamespace);
   if (pNS == 0) return 0;
   if (0 == prefix) prefix = OSUTF8("");
   pNS->prefix = rtxUTF8Strdup (pctxt, prefix);
   if (0 == pNS->prefix) {
      rtxMemFreePtr (pctxt, pNS);
      return 0;
   }
   if (0 != uri) {
      pNS->uri = rtxUTF8Strdup (pctxt, uri);
      if (0 == pNS->uri) {
         rtxMemFreePtr (pctxt, OSRTSAFECONSTCAST (void*, pNS->prefix));
         rtxMemFreePtr (pctxt, pNS);
         return 0;
      }
   }

   /* Append record to namespace list */
   rtxDListAppend (pctxt, pNSAttrs, pNS);

   return pNS;
}

EXTXMLMETHOD OSBOOL rtXmlNSEqual (OSXMLNamespace* pNS1, OSXMLNamespace* pNS2)
{
   if (pNS1 == pNS2) return TRUE;
   else if (pNS1 != 0 && pNS2 != 0) {
      return (OSBOOL)
         (rtxUTF8StrEqual (pNS1->prefix, pNS2->prefix) &&
          rtxUTF8StrEqual (pNS1->uri, pNS2->uri));
   }
   return FALSE;
}

EXTXMLMETHOD void rtXmlNSFreeAttrList (OSCTXT* pctxt, OSRTDList* pNSAttrs)
{
   if (0 != pNSAttrs) {
      OSXMLNamespace* pNS;
      OSRTDListNode* pNode = pNSAttrs->head;
      while (0 != pNode) {
         pNS = (OSXMLNamespace*) pNode->data;
         if (0 != pNS->prefix) rtxMemFreePtr (pctxt, pNS->prefix);
         if (0 != pNS->uri) rtxMemFreePtr (pctxt, pNS->uri);
         rtxMemFreePtr (pctxt, pNS);
         pNode = pNode->next;
      }
      rtxDListFreeNodes (pctxt, pNSAttrs);
   }
}


EXTXMLMETHOD const OSUTF8CHAR* rtXmlNSGetPrefix (OSCTXT* pctxt,
   const OSUTF8CHAR* uri)
{
   OSXMLNSPfxLink* plink = getPrefixLinkUsingURI (pctxt, uri);
   if ( plink == 0 ) return 0;
   else if ( OS_ISEMPTY(plink->ns.prefix) ) return OSUTF8("");
   else return plink->ns.prefix;
}


EXTXMLMETHOD const OSUTF8CHAR* rtXmlNSGetPrefixUsingIndex
(OSCTXT* pctxt, const OSUTF8CHAR* uri, OSUINT32 idx)
{
   OSXMLNSPfxLink* plink = getPrefixLinkUsingURI (pctxt, uri);
   if (0 != plink) {
      if (0 == idx) {
         return plink->ns.prefix;
      }
      else if (idx <= plink->extraPrefixes.count) {
         return plink->extraPrefixes.data[idx-1];
      }
   }
   return 0;
}

EXTXMLMETHOD OSUINT32 rtXmlNSGetPrefixCount (OSCTXT* pctxt, const OSUTF8CHAR* uri)
{
   OSXMLNSPfxLink* plink = getPrefixLinkUsingURI (pctxt, uri);
   return (0 != plink) ? plink->extraPrefixes.count + 1 : 0;
}

EXTXMLMETHOD int rtXmlNSGetPrefixIndex
(OSCTXT* pctxt, const OSUTF8CHAR* uri, const OSUTF8CHAR* prefix,
 OSUINT32* pcount)
{
   OSXMLNSPfxLink* plink = getPrefixLinkUsingURI (pctxt, uri);
   if (0 != plink) {
      if (0 != pcount) {
         *pcount = plink->extraPrefixes.count + 1;
      }
      if (0 != prefix) {
         if (prefixesEqual (plink->ns.prefix, prefix)) {
            return 0;
         }
         else if (plink->extraPrefixes.count > 0) {
            int i;
            for (i = 0; i < plink->extraPrefixes.count; i++) {
               if (prefixesEqual (plink->extraPrefixes.data[i], prefix)) {
                  return i + 1;
               }
            }
         }
      }
   }
   else if (0 != pcount) {
      *pcount = 0;
   }

   return -1;
}


EXTXMLMETHOD const OSUTF8CHAR* rtXmlNSLookupPrefixForURI
(OSCTXT* pctxt, const OSUTF8CHAR* uri)
{
   OSXMLNSPfxLink* plink = getPrefixLinkUsingURI (pctxt, uri);
   return getNonEmptyPrefix(plink);
}


/* This function looks up a namespace URI in the context stack */
EXTXMLMETHOD OSXMLNamespace* rtXmlNSLookupURI (OSCTXT* pctxt, const OSUTF8CHAR* uri)
{
   OSXMLCtxtInfo* pXMLCtxtInfo = (OSXMLCtxtInfo*) pctxt->pXMLInfo;
   OSXMLNSPfxLink* plink = getPrefixLinkUsingURI (pctxt, uri);
   if (0 != plink) {
      if (0 != plink->ns.uri) {
         if (rtxUTF8StrEqual (plink->ns.uri, uri))
            return &plink->ns;
      }
      else if (plink->nsidx >= 0 &&
               plink->nsidx < (int)pXMLCtxtInfo->nsURITable.nrows) {
         if (rtxUTF8StrEqual
             (pXMLCtxtInfo->nsURITable.data[plink->nsidx], uri)) {
            plink->ns.uri = rtxUTF8Strdup(pctxt,
               pXMLCtxtInfo->nsURITable.data[plink->nsidx]);
            return &plink->ns;
         }
      }
   }
   return 0;
}

/* This function looks up a namespace URI in the given list */
EXTXMLMETHOD OSXMLNamespace* rtXmlNSLookupURIInList
(OSRTDList* pNSAttrs, const OSUTF8CHAR* uri)
{
   if (0 != pNSAttrs && 0 != uri) {
      OSRTDListNode* pNode = pNSAttrs->head;
      OSXMLNamespace* pNS;
      for ( ; 0 != pNode; pNode = pNode->next) {
         pNS = (OSXMLNamespace*) pNode->data;
         if (pNS->uri && rtxUTF8StrEqual (pNS->uri, uri))
            return pNS;
      }
   }

   return 0;
}

EXTXMLMETHOD const OSUTF8CHAR* rtXmlNSLookupPrefix (OSCTXT* pctxt, const OSUTF8CHAR* prefix)
{
   const OSUTF8CHAR* uri = 0;
   OSXMLNSPfxLink* plink = 0;
   OSXMLCtxtInfo* pXMLCtxtInfo = (OSXMLCtxtInfo*) pctxt->pXMLInfo;
   OSXMLNSPfxLinkStackNode* pStackNode = pXMLCtxtInfo->nsPfxLinkStack.top;
   for ( ; 0 != pStackNode; pStackNode = pStackNode->next) {
      plink = pStackNode->link;
      for ( ; 0 != plink; plink = plink->next) {
         if (prefixesEqual (plink->ns.prefix, prefix)) {
            break;
         }
         else if (plink->extraPrefixes.count > 0) {
            int i;
            for (i = 0; i < plink->extraPrefixes.count; i++) {
               if (prefixesEqual (plink->extraPrefixes.data[i], prefix)) {
                  break;
               }
            }
            if (i < plink->extraPrefixes.count) break;
         }
      }
      if (0 != plink) break;
   }
   if (0 != plink) {
      if (plink->nsidx >= 0 &&
          plink->nsidx < (int)pXMLCtxtInfo->nsURITable.nrows) {
         uri = pXMLCtxtInfo->nsURITable.data[plink->nsidx];
      }
      else {
         uri = plink->ns.uri;
      }
   }

   return uri;
}

EXTXMLMETHOD const OSUTF8CHAR* rtXmlNSLookupPrefixFrag
(OSCTXT* pctxt, const OSUTF8CHAR* prefix, size_t prefixLen)
{
   OSUTF8CHAR lprefix[100];
   rtxUTF8Strncpy (lprefix, sizeof(lprefix), prefix, prefixLen);
   lprefix[prefixLen] = '\0';
   return rtXmlNSLookupPrefix (pctxt, lprefix);
}

EXTXMLMETHOD void rtXmlNSRemoveAll (OSCTXT* pctxt)
{
   OSXMLCtxtInfo* pXMLCtxtInfo = (OSXMLCtxtInfo*) pctxt->pXMLInfo;
   if (0 != pXMLCtxtInfo) {
      while (pXMLCtxtInfo->nsPfxLinkStack.count > 0) {
         rtXmlNSPop (pctxt);
      }
   }
}

EXTXMLMETHOD OSXMLNamespace* rtXmlNSSetNamespace
(OSCTXT* pctxt, OSRTDList* pNSAttrs, const OSUTF8CHAR* prefix,
 const OSUTF8CHAR* uri, OSBOOL override)
{
   OSXMLNamespace* pNS;

   /* If namespace list not provided, use list in context */
   if (0 == pNSAttrs) {
      OSXMLCtxtInfo* pXMLCtxtInfo = (OSXMLCtxtInfo*) pctxt->pXMLInfo;
      if (0 != pXMLCtxtInfo) pNSAttrs = &pXMLCtxtInfo->namespaceList;
      else return 0;
   }

   /* Lookup namespace record in namespace list.  If not found, add
      record to list; otherwise, only update prefix if override flag
      is set. */
   pNS = rtXmlNSLookupURIInList (pNSAttrs, uri);
   if (0 == pNS) {
      pNS = rtXmlNSAddNamespace (pctxt, pNSAttrs, prefix, uri);
   }
   else if (override) {
      if (0 != pNS->prefix) {
         rtxMemFreePtr (pctxt, OSRTSAFECONSTCAST (OSOCTET*, pNS->prefix));
      }
      if (0 == prefix) prefix = OSUTF8("");
      pNS->prefix = rtxUTF8Strdup (pctxt, prefix);
   }
   else pNS = 0;

   return pNS;
}

EXTXMLMETHOD const OSUTF8CHAR* rtXmlNSNewPrefix
(OSCTXT* pctxt, const OSUTF8CHAR* uri, OSRTDList* pNSAttrs)
{
   OSUINT32 i;
   OSUTF8CHAR prefix[15]; /* UINT32_MAX is 10 digits */
   OSXMLNamespace* pNS;

   if ( uri == 0 || *uri == 0 ) return 0;

   /* If namespace list not provided, use list in context */
   if (0 == pNSAttrs) {
      OSXMLCtxtInfo* pXMLCtxtInfo = (OSXMLCtxtInfo*) pctxt->pXMLInfo;
      if (0 != pXMLCtxtInfo) pNSAttrs = &pXMLCtxtInfo->namespaceList;
      else return 0;
   }

   /* Create unique prefix name */
   rtxStrcpy ((char*)prefix, sizeof(prefix), "ns");
   for (i = 1; ; i++) {
      rtxUIntToCharStr (i, (char*)&prefix[2], sizeof(prefix)-2, 0);
      if (0 == rtXmlNSLookupPrefix (pctxt, prefix)) break;
   }

   /* Add prefix/URI mapping to local list and namespace stack */
   rtXmlNSAddPrefixLink (pctxt, prefix, uri, 0, 0);

   pNS = rtXmlNSAddNamespace (pctxt, pNSAttrs, prefix, uri);
   if (pNS == 0) return 0;

   return pNS->prefix;
}

EXTXMLMETHOD int rtXmlNSAddPrefixLink
(OSCTXT* pctxt, const OSUTF8CHAR* prefix, const OSUTF8CHAR* uri,
 const OSUTF8CHAR* nsTable[], OSUINT32 nsTableRowCount)
{
   OSUINT32 ui;
   OSXMLNSPfxLink* plink;
   OSXMLCtxtInfo* pXMLCtxtInfo = (OSXMLCtxtInfo*) pctxt->pXMLInfo;
   if (0 == pXMLCtxtInfo) return RTERR_NOTINIT;

   if (0 == pXMLCtxtInfo->nsPfxLinkStack.count)
      return LOG_RTERR (pctxt, RTERR_NULLPTR);

   /* If namespace table parameter not passed, used table in context
      (note: this should be the typical case) */
   if (0 == nsTable) {
      nsTable = pXMLCtxtInfo->nsURITable.data;
      nsTableRowCount = pXMLCtxtInfo->nsURITable.nrows;
   }

   /* Look up namespace URI in table */
   for (ui = 0; ui < nsTableRowCount; ui++) {
      if (rtxUTF8StrEqual (uri, nsTable[ui])) break;
   }

   /* Determine if a record exists in the list of the given URI */
   plink = pXMLCtxtInfo->nsPfxLinkStack.top->link;
   while (0 != plink) {
      if (ui < nsTableRowCount) {
         /*  URI was in table, so look for a link with a matching index */
         if (plink->nsidx == (int)ui) break;
      }
      else if (0 != plink->ns.uri) {
         /*  URI was not in table, so look for matching URI */
         if (rtxUTF8StrEqual (uri, plink->ns.uri)) break;
      }
      plink = plink->next;
   }

   if (0 == plink) {
      /* Record does not exist at this level in stack.  Create a new
         namespace URI/prefix link object and add to list */
      plink = rtxMemAllocType (pctxt, OSXMLNSPfxLink);
      if (0 == plink) return LOG_RTERR (pctxt, RTERR_NOMEM);

      plink->ns.prefix = rtxUTF8StrRefOrDup (pctxt, prefix);
      if (ui < nsTableRowCount) {
         /* URI was found in table */
         plink->ns.uri = 0;  /* not necessary to store URI */
         plink->nsidx = (OSINT32) ui;
      }
      else { /* not found */
         plink->ns.uri = rtxUTF8StrRefOrDup (pctxt, uri);
         plink->nsidx = -1;
      }
      rtxDynPtrArrayInit (pctxt, &plink->extraPrefixes, 4);

      /* Add record to head of list */
      plink->next = pXMLCtxtInfo->nsPfxLinkStack.top->link;
      pXMLCtxtInfo->nsPfxLinkStack.top->link = plink;
   }
   else {
      /* Add extra prefix to existing record.
         Keep any empty prefix in ns.prefix for quick access
      */
      if ( !OS_ISEMPTY(prefix) || !OS_ISEMPTY(plink->ns.prefix) ) {
         /* if both were empty, we wouldn't need to add this prefix. */

         const OSUTF8CHAR* listPrefix = rtxUTF8StrRefOrDup (pctxt, prefix);

         if ( OS_ISEMPTY(prefix) && !OS_ISEMPTY(plink->ns.prefix) ) {
            /* swap the empty prefix into ns.prefix and put non-empty prefix */
            /* into extraPrefixes */
            rtxDynPtrArrayAppend (pctxt, &plink->extraPrefixes,
               (void*)plink->ns.prefix);

            plink->ns.prefix = listPrefix;
         }
         else {
            rtxDynPtrArrayAppend (pctxt, &plink->extraPrefixes,
               (void*)listPrefix);
         }
      }
      /* else: already have empty prefix */
   }

   return 0;
}

EXTXMLMETHOD int rtXmlNSFreeAllPrefixLinks
(OSCTXT* pctxt, OSXMLNSPfxLinkStackNode* pStackNode)
{
   if (0 != pStackNode) {
      while (0 != pStackNode->link) {
         OSXMLNSPfxLink* pnext = pStackNode->link->next;
         rtXmlNSFreePrefixLink (pctxt, pStackNode->link);
         pStackNode->link = pnext;
      }
   }
   return 0;
}

EXTXMLMETHOD int rtXmlNSFreePrefixLink (OSCTXT* pctxt, OSXMLNSPfxLink* plink)
{
   if (0 != plink) {
      if (0 != plink->ns.prefix)
         rtxMemFreePtr (pctxt, OSRTSAFECONSTCAST (OSOCTET*, plink->ns.prefix));

      if (0 != plink->ns.uri)
         rtxMemFreePtr (pctxt, OSRTSAFECONSTCAST (OSOCTET*, plink->ns.uri));

      if (plink->extraPrefixes.count > 0) {
         int i;
         for (i = 0; i < plink->extraPrefixes.count; i++) {
            rtxMemFreePtr
               (pctxt, OSRTSAFECONSTCAST
                (OSOCTET*, plink->extraPrefixes.data[i]));
         }
         rtxMemFreePtr (pctxt, plink->extraPrefixes.data);
      }
      rtxMemFreePtr (pctxt, plink);
   }
   return 0;
}

EXTXMLMETHOD int rtXmlNSGetIndex (OSCTXT* pctxt, const OSUTF8CHAR* prefix)
{
   OSXMLCtxtInfo* pXMLCtxtInfo = (OSXMLCtxtInfo*) pctxt->pXMLInfo;
   OSXMLNSPfxLinkStackNode* pStackNode = pXMLCtxtInfo->nsPfxLinkStack.top;
   for ( ; 0 != pStackNode; pStackNode = pStackNode->next) {
      OSXMLNSPfxLink* plink = pStackNode->link;
      for ( ; 0 != plink; plink = plink->next) {
         if (prefixesEqual (prefix, plink->ns.prefix))
            return plink->nsidx;

         /* If extra prefixes exist, check these */
         if (plink->extraPrefixes.count > 0) {
            int i;
            for (i = 0; i < plink->extraPrefixes.count; i++) {
               if (prefixesEqual (plink->extraPrefixes.data[i], prefix)) {
                  return plink->nsidx;
               }
            }
         }
      }
   }
   return -1;
}

EXTXMLMETHOD const OSUTF8CHAR* rtXmlNSGetPrefixUsingNsIdx (OSCTXT* pctxt, OSINT32 nsidx)
{
   OSXMLCtxtInfo* pXMLCtxtInfo = (OSXMLCtxtInfo*) pctxt->pXMLInfo;
   OSXMLNSPfxLinkStackNode* pStackNode = pXMLCtxtInfo->nsPfxLinkStack.top;
   for ( ; 0 != pStackNode; pStackNode = pStackNode->next) {
      OSXMLNSPfxLink* plink = pStackNode->link;
      for ( ; 0 != plink; plink = plink->next) {
         if (nsidx == plink->nsidx) {
            return plink->ns.prefix;
         }
      }
   }
   return (const OSUTF8CHAR*)0;
}

EXTXMLMETHOD int rtXmlNSPush (OSCTXT* pctxt)
{
   OSXMLCtxtInfo* pXMLCtxtInfo = (OSXMLCtxtInfo*) pctxt->pXMLInfo;
   OSXMLNSPfxLinkStackNode* pStackNode =
      rtxMemAllocType (pctxt, OSXMLNSPfxLinkStackNode);

   if (0 == pStackNode) return LOG_RTERR (pctxt, RTERR_NOMEM);

   pStackNode->link = 0;
   pStackNode->next = pXMLCtxtInfo->nsPfxLinkStack.top;
   pXMLCtxtInfo->nsPfxLinkStack.top = pStackNode;
   pXMLCtxtInfo->nsPfxLinkStack.count++;

   return 0;
}

EXTXMLMETHOD int rtXmlNSPop (OSCTXT* pctxt)
{
   OSXMLCtxtInfo* pXMLCtxtInfo = (OSXMLCtxtInfo*) pctxt->pXMLInfo;
   if (0 != pXMLCtxtInfo->nsPfxLinkStack.count) {
      OSXMLNSPfxLinkStackNode* pStackNode = pXMLCtxtInfo->nsPfxLinkStack.top;
      pXMLCtxtInfo->nsPfxLinkStack.top = pStackNode->next;
      pXMLCtxtInfo->nsPfxLinkStack.count--;

      /* Free memory of all prefix links */
      rtXmlNSFreeAllPrefixLinks (pctxt, pStackNode);

      /* Free stack entry memory */
      rtxMemFreePtr (pctxt, pStackNode);

      return 0;
   }
   else return LOG_RTERR (pctxt, RTERR_NULLPTR);
}

EXTXMLMETHOD void rtXmlNSSetURITable
(OSCTXT* pctxt, const OSUTF8CHAR* data[], OSUINT32 nrows)
{
   OSXMLCtxtInfo* pXMLCtxtInfo = (OSXMLCtxtInfo*) pctxt->pXMLInfo;
   pXMLCtxtInfo->nsURITable.nrows = nrows;
   pXMLCtxtInfo->nsURITable.data = data;
}

EXTXMLMETHOD const OSUTF8CHAR* rtXmlNSGetQName (OSCTXT* pctxt, OSUTF8CHAR* buf,
   size_t bufsiz, const OSUTF8CHAR* uri, const OSUTF8CHAR* localName)
{
   size_t idx = 0;
   const OSUTF8CHAR* nsPrefix = rtXmlNSGetPrefix (pctxt, uri);
   if (!OS_ISEMPTY (nsPrefix)) {
      rtxUTF8Strcpy (buf, bufsiz, nsPrefix);
      idx += OSUTF8LEN(nsPrefix);
      buf[idx++] = ':';
   }

   rtxUTF8Strcpy (&buf[idx], bufsiz-idx, localName);

   return buf;
}

EXTXMLMETHOD const OSUTF8CHAR* rtXmlNSGetAttrPrefix (OSCTXT* pctxt,
   const OSUTF8CHAR* namespaceURI, OSRTDList* pNSAttrs)
{
   if (namespaceURI != 0 && *namespaceURI != 0) {
      /* we have a (non-empty) URI */
      const OSUTF8CHAR* nsPrefix = rtXmlNSLookupPrefixForURI(pctxt,
         namespaceURI);

      /* create a new prefix if we didn't find one */
      if ( nsPrefix == 0 )
         nsPrefix = rtXmlNSNewPrefix (pctxt, namespaceURI, pNSAttrs);

      return nsPrefix;
   }
   else return 0; /* no (non-empty) URI */
}


EXTXMLMETHOD const OSUTF8CHAR* rtXmlNSGetAttrQName (OSCTXT* pctxt, OSUTF8CHAR* buf,
   size_t bufsiz, OSXMLNamespace* pNS, const OSUTF8CHAR* localName,
   OSRTDList* pNSAttrs)
{
   size_t idx = 0;

   if (0 != pNS && pNS->uri != 0 && *pNS->uri != 0) {
      /* we have a (non-empty) URI */
      const OSUTF8CHAR* nsPrefix = rtXmlNSGetAttrPrefix(pctxt, pNS->uri,
                                    pNSAttrs);

      /* copy prefix into buffer */
      rtxUTF8Strcpy (buf, bufsiz, nsPrefix);
      idx += OSUTF8LEN(nsPrefix);
      buf[idx++] = ':';
   }
   /* else: no (non-empty) URI */

   rtxUTF8Strcpy (&buf[idx], bufsiz-idx, localName);

   return buf;
}

