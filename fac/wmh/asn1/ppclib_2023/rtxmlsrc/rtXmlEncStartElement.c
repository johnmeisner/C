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

#include "rtxsrc/rtxDList.h"
#include "rtxmlsrc/osrtxml.hh"

#ifdef RT_XML_CPP_ENC_START_ELEMENT
#include "rtxmlsrc/OSXSDComplexType.h"
#endif

/*
When RT_XML_CPP_ENC_START_ELEMENT is defined, we are being included into
rtXmlCppEncStartElement.cpp to declare rtXmlCppEncStartElement.
The purpose here is to avoid allocating/releasing memory using C methods and C
structs where C++ structs should be used.
*/
#ifdef RT_XML_CPP_ENC_START_ELEMENT
EXTXMLMETHOD int rtXmlCppEncStartElement (OSCTXT* pctxt,
                          const OSUTF8CHAR* elemName,
                          OSXMLNamespace* pNS,
                          OSRTDListClass* pNSAttrs,
                          OSBOOL terminate)
#else
EXTXMLMETHOD int rtXmlEncStartElement (OSCTXT* pctxt,
                          const OSUTF8CHAR* elemName,
                          OSXMLNamespace* pNS,
                          OSRTDList* pNSAttrs,
                          OSBOOL terminate)
#endif
{
   OSXMLNamespace tempNsStruct;  /* used when modifiying pNS is not safe */
   int stat = 0;
   OSRTDList* pEncNSList;
   OSXMLCtxtInfo* pXMLCtxtInfo = (OSXMLCtxtInfo*) pctxt->pXMLInfo;
   OSRTASSERT (0 != pXMLCtxtInfo);
   pEncNSList = &pXMLCtxtInfo->encodedNSList;


   /* Note: in order to have a consistent side-effect, this method will */
   /* always clear the OSXSINIL context flag, whether it encodes (or causes to */
   /* be encoded) the xsi:nil attribute or not. */

   if (0 != elemName && 0 != *elemName) {
      size_t elemLen = rtxUTF8LenBytes (elemName);
      size_t nsPrefixLen;
      size_t specChars;

      /* Push new entry onto namespace stack */
      stat = rtXmlNSPush (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      /* add an null entry to encoded list to mark start of namespaces encoded
         at the current level */
      rtxDListAppend (pctxt, pEncNSList, 0);

      /* Add namespaces to namespace prefix link stack.  This makes
         those namespaces part of what is searched.  They will be encooded
         when generated code invokes rtXmlEncNSAttrs.
      */
      stat = rtXmlSetNSPrefixLinks (pctxt, pNSAttrs);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      /* If we do not create a new prefix, it is safe (and probably desirable)
         to set the namespace prefix in the passed namespace structure.  This
         lets us avoid further lookups.
         However, if we create a new prefix, this is not safe to do.  The reason
         being that the user code may end up using that structure (and that
         prefix) with a different namespace list.  The result is that we
         encode the prefix without declaring it.  For this case, we change
         pNS to point to a temporary structure.
      */
      if (0 != pNS) {
         if ( OS_ISEMPTY(pNS->uri) && pNS->prefix == 0 ) {
            /* if no namespace was given, we don't want to create a prefix. */
            /* However, we cannot leave prefix null because of the call to */
            /* OSUTF8LEN below, so assign empty string.  This follows */
            /* pre-existing behavior. */
            pNS->prefix = OSUTF8("");
         }
         else if (0 == pNS->prefix) {
            /* pNS->uri is not empty and prefix is null.  We must have a prefix. */
            /* Possibly, an empty string prefix will be used (ns is default),  */
            /* but null means we must find or create a prefix. */
            pNS->prefix = rtXmlNSGetPrefix (pctxt, pNS->uri);
            if ( pNS->prefix == 0 ) {
               /* change to using temp struct */
               tempNsStruct.uri = pNS->uri;
#ifdef RT_XML_CPP_ENC_START_ELEMENT
               tempNsStruct.prefix = OSXSDComplexType::newPrefix(pctxt,
                  pNS->uri, pNSAttrs);
#else
               tempNsStruct.prefix = rtXmlNSNewPrefix(pctxt,
                  pNS->uri, pNSAttrs);
#endif
               pNS = &tempNsStruct;    /* repoint pNS to the temp structure! */
            }
         }
         /* else: prefix is not null */

         nsPrefixLen = OSUTF8LEN (pNS->prefix);
      }
      else nsPrefixLen = 0;

      specChars = (0 != nsPrefixLen) ? 3 : 2;

      /* Terminate previous element if still open */
      stat = rtXmlEncTermStartElement (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      stat = rtXmlEncIndent (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      pctxt->level++;
      pctxt->state = OSXMLSTART;

      /* Verify element will fit in encode buffer */

      stat = rtXmlCheckBuffer (pctxt, elemLen + nsPrefixLen + specChars);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      /* Copy data to buffer */

      stat = rtXmlPutChar (pctxt, '<');

      if (0 != nsPrefixLen) {
         stat = rtXmlWriteChars (pctxt, pNS->prefix, nsPrefixLen);
         stat = rtXmlPutChar (pctxt, ':');
      }
      stat = rtXmlWriteChars (pctxt, elemName, elemLen);

      /* Encode xsi:nil attribute if OSXSINIL is set */
      if (rtxCtxtTestFlag (pctxt, OSXSINIL))
         stat = rtXmlEncXSINilAttr (pctxt);

      if (terminate) {
         stat = rtXmlPutChar (pctxt, '>');
         pctxt->flags &= ~OSTERMSTART;
      }
      else /* set flag in context indicating terminator needed */
         pctxt->flags |= OSTERMSTART;

#ifndef _COMPACT
      /* Add name to element name stack in context */
      rtxDListAppend (pctxt, &pctxt->elemNameStack, (void*)elemName);
#endif
      if (!terminate && rtxCtxtTestFlag (pctxt, OSXMLC14N)
         && !rtxCtxtTestFlag(pctxt, OSASN1XER ) )
         stat = rtXmlEncStartAttrC14N (pctxt);
   }
   else {
      if (terminate)
         stat = rtXmlEncTermStartElement (pctxt);

      if (rtxCtxtTestFlag (pctxt, OSXSINIL))
         rtxCtxtClearFlag(pctxt, OSXSINIL);
   }

   if (stat < 0) return LOG_RTERR (pctxt, stat);
   return 0;
}

#ifndef RT_XML_CPP_ENC_START_ELEMENT
/* This function will terminate a currently open start element */

EXTXMLMETHOD int rtXmlEncTermStartElement (OSCTXT* pctxt)
{
   if (pctxt->state == OSXMLSTART && pctxt->flags & OSTERMSTART) {
      if (!rtxCtxtTestFlag(pctxt, OSASN1XER ) &&
         rtxCtxtTestFlag (pctxt, OSXMLC14N)) {
         int stat = rtXmlEncEndAttrC14N (pctxt);
         if (stat < 0) return LOG_RTERRNEW (pctxt, stat);
      }

      /* If empty element, terminate with '/>', otherwise '>' */
      if (pctxt->flags & OSEMPTYELEM)
         rtXmlPutChar (pctxt, '/');

      rtXmlPutChar (pctxt, '>');

      /* Indicate element is terminated */
      pctxt->flags &= ~(OSTERMSTART | OSEMPTYELEM);
   }
   return 0;
}

#endif      /* #ifndef RT_XML_CPP_ENC_START_ELEMENT */
