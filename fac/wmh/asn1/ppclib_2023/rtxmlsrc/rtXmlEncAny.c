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

#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxStreamMemory.h"
#include "rtxmlsrc/rtXmlPull.hh"
#include "rtxsrc/rtxStack.h"

EXTXMLMETHOD int rtXmlEncParseAny
(OSCTXT* pctxt, OSCTXT* parseCtxt, const OSUTF8CHAR* pvalue,
 size_t valueLen, OSBOOL anyType)
{
   int stat, indent, level = 0, startLevel = 0, prevState = 0;
   OSXMLCtxtInfo* xmlCtxt;
   struct OSXMLReader* pXmlReader;
   OSRTStack QNameStack;
   OSRTMEMBUF memBuf;
   const char hdr[] = "<hdr>";
   const char foot[] = "</hdr>";

   /* Do not indent elements. */
   indent = rtXmlGetIndent (pctxt);
   if (indent < 0) return LOG_RTERR (pctxt, indent);
   stat = rtXmlSetIndent (pctxt, 0);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* Stack used to hold element QNames during parse */
   rtxStackInit (parseCtxt, &QNameStack);

   /* anyType must be wrapped in dummy element since it may contain
      only text without tags or start with mixed content. */
   if (anyType) {
      startLevel = 1;
      rtxMemBufInit (parseCtxt, &memBuf, 256);
      /* Add <hdr>. */
      stat = rtxMemBufAppend (&memBuf, (const OSOCTET*)hdr, sizeof (hdr)-1);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      /* Add content. */
      stat = rtxMemBufAppend (&memBuf, (const OSOCTET*)pvalue, valueLen);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      /* Add </hdr>. */
      stat = rtxMemBufAppend (&memBuf, (const OSOCTET*)foot, sizeof (foot)-1);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

#ifndef _NO_STREAM
      stat = rtxStreamMemoryCreateReader
         (parseCtxt, memBuf.buffer, memBuf.usedcnt);
#else /* _NO_STREAM */
      stat = rtxInitContextBuffer (parseCtxt, memBuf.buffer, memBuf.usedcnt);
#endif /* _NO_STREAM */
   }
   else {
#ifndef _NO_STREAM
      stat = rtxStreamMemoryCreateReader
         (parseCtxt, (OSOCTET*)pvalue, valueLen);
#else /* _NO_STREAM */
      stat = rtxInitContextBuffer (parseCtxt, (OSOCTET*)pvalue, valueLen);
#endif /* _NO_STREAM */
   }
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   rtXmlpCreateReader (parseCtxt);
   xmlCtxt = ((OSXMLCtxtInfo*)parseCtxt->pXMLInfo);
   pXmlReader = xmlCtxt->pXmlPPReader;
   OSRTASSERT (0 != pXmlReader);

   rtXmlRdSetWhiteSpaceMode (pXmlReader, OSXMLWSM_PRESERVE);
   pXmlReader->mbNoTransform = FALSE;

   for (stat = pXmlReader->mLastEvent.mId;
        stat >= 0;
        stat = rtXmlRdNext (pXmlReader))
   {
      if (pXmlReader->mLastEvent.mId == OSXMLEVT_END_DOCUMENT) {
         break; /* Done. */
      }
      else if (pXmlReader->mLastEvent.mId == OSXMLEVT_TEXT) {
         size_t len = pXmlReader->mData.length;

         /* Handle CDATA if not in C14N mode. */
         if (pXmlReader->mbCDATA && !rtxCtxtTestFlag (pctxt, OSXMLC14N)) {
            if (prevState >= 0)           /* First CDATA chunk. */
               stat = rtXmlWriteChars (pctxt, OSUTF8("<![CDATA["), 9);

            stat = rtXmlWriteChars (pctxt, pXmlReader->mData.value, len);

            if (pXmlReader->mState >= 0)  /* Last CDATA chunk. */
               stat = rtXmlWriteChars (pctxt, OSUTF8("]]>"), 3);
         }
         else {
            stat = rtXmlEncStringValue2 (pctxt, pXmlReader->mData.value, len);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }
      }
      else if (pXmlReader->mLastEvent.mId == OSXMLEVT_START_TAG) {
         const OSUTF8CHAR* elemName;
         OSXMLQName* pQName;
         OSRTDList nsAttrs;
         OSXMLNamespace ns;
         size_t markedPos;
         OSBOOL hasAttrs;
         ns.prefix = OSUTF8("");
         ns.uri = 0;

         /* Ignore dummy element for anyType. */
         if (level < startLevel) {
            level++;
            pctxt->state = OSXMLDATA;
            continue;
         }

         rtxDListInit (&nsAttrs);

         if (pXmlReader->mMarkedPos != (size_t)-1)
            markedPos = pXmlReader->mMarkedPos;
         else
            markedPos = 0;

         hasAttrs = rtXmlpHasAttributes (parseCtxt);
         /* parse NS declarations */
         if (hasAttrs) {
            stat = rtXmlpGetXmlnsAttrs (parseCtxt, &nsAttrs);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }

         elemName = rtxUTF8Strndup (parseCtxt,
            pXmlReader->mpBuffer + markedPos +
             pXmlReader->mElementName.mLocalName.offset,
            pXmlReader->mElementName.mLocalName.length);

         /* Copy prefix. */
         if (pXmlReader->mElementName.mPrefix.length > 0) {
            ns.prefix = rtxUTF8Strndup (parseCtxt,
               pXmlReader->mpBuffer + markedPos +
                pXmlReader->mElementName.mPrefix.offset,
               pXmlReader->mElementName.mPrefix.length);
         }
         /* Push element's QName onto stack. */
         if (!pXmlReader->mbEmptyElement) {
            pQName = rtxMemAllocType (parseCtxt, OSXMLQName);
            if (0 == pQName) return LOG_RTERR (pctxt, RTERR_NOMEM);

            pQName->nsPrefix = ns.prefix;
            pQName->ncName = elemName;

            stat = rtxStackPush (&QNameStack, pQName);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }
         stat = rtXmlEncStartElement (pctxt, elemName, &ns, &nsAttrs, FALSE);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         /* Parse attributes. */
         if (hasAttrs) {
            stat = rtXmlpGetAttributeCount (parseCtxt);
            if (stat > 0) {
               size_t i;
               size_t nAttrs = (size_t)stat;
               for (i = 0; i < nAttrs; i++) {
                  OSXMLNameFragments attrName;
                  OSINT16 nsidx;
                  OSUTF8CHAR* value;

                  if (rtXmlpSelectAttribute
                      (parseCtxt, &attrName, &nsidx, i) != 0)
                     continue;

                  stat = rtXmlpDecDynUTF8Str (parseCtxt, (const OSUTF8CHAR**)&value);
                  if (stat != 0) return LOG_RTERR (parseCtxt, stat);

                  stat = rtXmlEncUTF8Attr2 (pctxt, attrName.mQName.value,
                     attrName.mQName.length, value, rtxUTF8LenBytes (value));

                  if (stat != 0) return LOG_RTERR (pctxt, stat);
               }
            }
         }
         pctxt->state = OSXMLSTART;

         stat = rtXmlEncTermStartElement (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         /* End empty element. */
         if (pXmlReader->mbEmptyElement) {
            stat = rtXmlEncEndElement (pctxt, elemName, &ns);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }
         pctxt->state = OSXMLDATA;
         level++;
      }
      else if (pXmlReader->mLastEvent.mId == OSXMLEVT_END_TAG) {
         OSXMLQName* pQName;
         OSXMLNamespace ns;
         ns.uri = 0;

         level--;

         /* Empty element handled in START_TAG; ignore dummy elem. */
         if (pXmlReader->mbEmptyElement || level < startLevel)
            continue;

         /* Get element name from stack */
         pQName = (OSXMLQName*)rtxStackPop (&QNameStack);
         if (0 == pQName) return LOG_RTERR (pctxt, RTERR_NULLPTR);

         ns.prefix = pQName->nsPrefix;
         stat = rtXmlEncEndElement (pctxt, pQName->ncName, &ns);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      else if(pXmlReader->mLastEvent.mId == OSXMLEVT_COMMENT) {
         size_t len = pXmlReader->mData.length;

         if (prevState >= 0)        /* First comment chunk. */
            stat = rtXmlWriteChars (pctxt, OSUTF8("<!-"), 3);

         stat = rtXmlWriteChars (pctxt, pXmlReader->mData.value, len);
      }
      else if (pXmlReader->mLastEvent.mId == OSXMLEVT_PI) {
         size_t len = pXmlReader->mData.length;

         if ((prevState >= 0) && (len > 0))        /* First PI chunk. */
            stat = rtXmlPutChar (pctxt, '<');

         stat = rtXmlWriteChars (pctxt, pXmlReader->mData.value, len);
      }
      prevState = pXmlReader->mState;
   }

   /* Restore user's indentation level. */
   rtXmlSetIndent (pctxt, (OSUINT8)indent);

   /* Unbalanced elements -- fragment is not well-formed. */
   return (level != 0 || stat < 0) ?
      LOG_RTERR (pctxt, RTERR_XMLPARSE) : 0;
}

EXTXMLMETHOD int rtXmlEncAny (OSCTXT* pctxt, OSXMLSTRING* pvalue,
                 const OSUTF8CHAR* elemName,
                 OSXMLNamespace* pNS)
{
   size_t valueLen = 0;
   int stat;

   if (0 != pvalue && 0 != pvalue->value) {
      valueLen = OSUTF8LEN (pvalue->value);
   }

   if (0 == valueLen) {
      if (!OS_ISEMPTY (elemName)) {
         OSRTDList namespaces;         /* isolate any ns prefixes created by */
                                       /* rtXmlEncEmptyElement */
         rtxDListInit(&namespaces);

         stat = rtXmlEncIndent (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         stat = rtXmlEncEmptyElement (pctxt, elemName, pNS, &namespaces, TRUE);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
   }
   else {
      if (!OS_ISEMPTY (elemName)) {
         OSRTDList namespaces;
         rtxDListInit(&namespaces);

         stat = rtXmlEncStartElement (pctxt, elemName, pNS, &namespaces, FALSE);

         /* if rtXmlEncStartElement added any namespaces, encode them */
         if ( stat == 0 )
            stat = rtXmlEncNSAttrs(pctxt, &namespaces);

         /* encode close of start tag */
         if ( stat == 0 )
            stat = rtXmlEncTermStartElement(pctxt);

         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      else {
         /* Make sure previous start element is terminated */
         stat = rtXmlEncTermStartElement (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         stat = rtXmlEncIndent (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
         pctxt->level++;
      }
      pctxt->state = OSXMLDATA;

      if (0 != pvalue->value && valueLen > 0) {
         OSCTXT parseCtxt;
         stat = rtXmlInitContext (&parseCtxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         stat = rtXmlEncParseAny
            (pctxt, &parseCtxt, pvalue->value, valueLen, FALSE);

         rtxFreeContext (&parseCtxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }

      if (!OS_ISEMPTY (elemName)) {
         stat = rtXmlEncEndElement (pctxt, elemName, pNS);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      else {
         pctxt->state = OSXMLEND;
         pctxt->level--;
      }
   }

   return (0);
}

EXTXMLMETHOD int rtXmlEncAnyStr
(OSCTXT* pctxt, const OSUTF8CHAR* pvalue,
 const OSUTF8CHAR* elemName, OSXMLNamespace* pNS)
{
   OSXMLSTRING xstrval;
   xstrval.cdata = FALSE;
   xstrval.value = pvalue;
   return rtXmlEncAny (pctxt, &xstrval, elemName, pNS);
}
