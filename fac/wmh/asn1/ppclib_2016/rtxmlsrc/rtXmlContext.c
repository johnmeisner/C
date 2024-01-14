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
#include "rtxsrc/rtxStream.h"
#ifdef ASN1RT
#include "rtsrc/rtContext.h"
#endif
#ifdef RTEVAL
#include "rtxevalsrc/rtxEval.c"
#endif
#include "rtxsrc/rtxContext.hh"

#include "rtXmlCtxtAppInfo.h"

EXTXMLMETHOD OSXMLCtxtInfo* rtXmlCtxtAppInfoDup
(OSCTXT* pctxt, OSCTXT* pDstCtxt)
{
   OSXMLCtxtInfo* pDstXMLInfo;
   OSXMLCtxtInfo* pSrcXMLInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
   OSRTDListNode* pNode;

   pDstXMLInfo = (OSXMLCtxtInfo*)
      rtxMemSysAlloc (pDstCtxt, sizeof(OSXMLCtxtInfo));
   if (pDstXMLInfo == 0) return 0;

   pDstXMLInfo->pFreeFunc = pSrcXMLInfo->pFreeFunc;
   pDstXMLInfo->pResetFunc = pSrcXMLInfo->pResetFunc;

   pDstXMLInfo->schemaLocation =
      rtxUTF8Strdup (pDstCtxt, pSrcXMLInfo->schemaLocation);

   pDstXMLInfo->noNSSchemaLoc =
      rtxUTF8Strdup (pDstCtxt, pSrcXMLInfo->noNSSchemaLoc);

   pDstXMLInfo->xsiTypeAttr =
      rtxUTF8Strdup (pDstCtxt, pSrcXMLInfo->xsiTypeAttr);

   pDstXMLInfo->encoding = pSrcXMLInfo->encoding;
   rtxDListInit (&pDstXMLInfo->namespaceList);
   rtxDListInit (&pDstXMLInfo->encodedNSList);
   rtxDListInit (&pDstXMLInfo->sortedAttrList);

   pDstXMLInfo->encodingStr = pSrcXMLInfo->encodingStr;

   pDstXMLInfo->pXmlPPReader = 0;  /* force recreation of reader */
   pDstXMLInfo->attrsBuff = 0;
   pDstXMLInfo->attrsBuffSize = 0;
   pDstXMLInfo->attrStartPos = 0;

   for (pNode = pSrcXMLInfo->namespaceList.head;
        pNode != 0; pNode = pNode->next)
   {
      OSXMLNamespace* pNS = (OSXMLNamespace*) pNode->data;
      OSXMLNamespace* pNewNS = rtxMemAllocTypeZ (pDstCtxt, OSXMLNamespace);
      if (pNewNS == 0) return 0;

      if (0 != pNS->prefix) {
         pNewNS->prefix = rtxUTF8Strdup (pDstCtxt, pNS->prefix);
         if (pNewNS->prefix == 0)
            return 0;
      }
      if (0 != pNS->uri) {
         pNewNS->uri = rtxUTF8Strdup (pDstCtxt, pNS->uri);
         if (pNewNS->uri == 0)
            return 0;
      }
      if (rtxDListAppend (pDstCtxt, &pDstXMLInfo->namespaceList, pNewNS) == 0)
         return 0;
   }

   /* Duplicate namespace context stack */
   pDstXMLInfo->nsPfxLinkStack.count = 0;
   pDstXMLInfo->nsPfxLinkStack.top = 0;
   { OSXMLNSPfxLinkStackNode* pStackNode = pSrcXMLInfo->nsPfxLinkStack.top;
   OSXMLNSPfxLink* plink;
   for ( ; 0 != pStackNode; pStackNode = pStackNode->next) {
      rtXmlNSPush (pDstCtxt);
      plink = pStackNode->link;
      for ( ; 0 != plink; plink = plink->next) {
         rtXmlNSAddPrefixLink (pDstCtxt, plink->ns.prefix, plink->ns.uri,
                               pSrcXMLInfo->nsURITable.data,
                               pSrcXMLInfo->nsURITable.nrows);
      }
   }}

   /* Namespace URI table does not need to be copied.  It is constant. */
   pDstXMLInfo->nsURITable.data = pSrcXMLInfo->nsURITable.data;
   pDstXMLInfo->nsURITable.nrows = pSrcXMLInfo->nsURITable.nrows;

   pDstXMLInfo->indent = pSrcXMLInfo->indent;
   pDstXMLInfo->indentChar = pSrcXMLInfo->indentChar;
   pDstXMLInfo->mbCdataProcessed = pSrcXMLInfo->mbCdataProcessed;

   pDstXMLInfo->mSaxLevel = pDstXMLInfo->mSkipLevel = 0;
   pDstXMLInfo->maxSaxErrors = pSrcXMLInfo->maxSaxErrors;
   pDstXMLInfo->errorsCnt = 0;
   pDstXMLInfo->facets.totalDigits = pSrcXMLInfo->facets.totalDigits;
   pDstXMLInfo->facets.fractionDigits = pSrcXMLInfo->facets.fractionDigits;

   rtxMemBufInit (pctxt, &pDstXMLInfo->memBuf, OSRTMEMBUFSEG);

   return pDstXMLInfo;
}

EXTXMLMETHOD void rtXmlSetCtxtAppInfo (OSCTXT* pctxt, OSXMLCtxtInfo* pXMLInfo)
{
   OSXMLCtxtInfo* pCtxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (pCtxtInfo != 0) {
      rtXmlFreeCtxtAppInfo (pctxt);
      rtxMemSysFreePtr (pctxt, (void*)pCtxtInfo);
   }
   pctxt->pXMLInfo = pXMLInfo;
}

EXTXMLMETHOD int rtXmlGetIndent (OSCTXT* pctxt)
{
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo == NULL) {
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
   }

   return ctxtInfo->indent;
}

EXTXMLMETHOD int rtXmlGetIndentChar (OSCTXT* pctxt)
{
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo == NULL) {
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
   }

   return (int)ctxtInfo->indentChar;
}

EXTXMLMETHOD OSBOOL rtXmlGetWriteBOM (OSCTXT* pctxt)
{
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo == NULL) {
      return FALSE;
   }
   return (OSBOOL)(ctxtInfo->byteOrderMark != OSXMLBOM_NO_BOM);
}

EXTXMLMETHOD int rtXmlPrepareContext (OSCTXT* pctxt)
{
   if (0 != pctxt)  {
      pctxt->state = OSXMLINIT;
      pctxt->buffer.byteIndex = 0;
      return 0;
   }
   return RTERR_NOTINIT;
}

EXTXMLMETHOD int rtXmlSetEncC14N (OSCTXT* pctxt, OSBOOL value)
{
   int stat = 0;

   if (value) {
      rtxCtxtSetFlag (pctxt, OSXMLC14N);
      stat += rtXmlSetFormatting (pctxt, FALSE);
   }
   else { /* turn off */
      rtxCtxtClearFlag (pctxt, OSXMLC14N);
      stat += rtXmlSetFormatting (pctxt, TRUE);
   }

   return stat;
}

EXTXMLMETHOD int rtXmlSetEncXSINamespace (OSCTXT* pctxt, OSBOOL value)
{
   if (value) {
      rtxCtxtSetFlag (pctxt, OSXSIATTR);
   }
   else {
      rtxCtxtClearFlag (pctxt, OSXSIATTR);
   }
   return 0;
}

EXTXMLMETHOD int rtXmlSetEncXSINilAttr (OSCTXT* pctxt, OSBOOL value)
{
   if (value) {
      rtxCtxtSetFlag (pctxt, OSXSINIL);
   }
   else {
      rtxCtxtClearFlag (pctxt, OSXSINIL);
   }
   return 0;
}

EXTXMLMETHOD int rtXmlSetEncoding (OSCTXT* pctxt, OSXMLEncoding encoding)
{
   OSXMLCtxtInfo* ctxtInfo;
   if (0 != pctxt->pXMLInfo) {
      /* Only UTF-8/UTF-16/ISO-8859-1 are currently supported */
      if (encoding != OSXMLUTF8 &&
          encoding != OSXMLUTF16 &&
          encoding != OSXMLUTF16BE &&
          encoding != OSXMLUTF16LE &&
          encoding != OSXMLLATIN1)
         return LOG_RTERRNEW (pctxt, RTERR_NOTSUPP);
      ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
      ctxtInfo->encoding = encoding;
      if (ctxtInfo->byteOrderMark == OSXMLBOM_NO_BOM)
         rtXmlUpdateBOM (ctxtInfo, encoding);
      return 0;
   }
   return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
}

EXTXMLMETHOD int rtXmlUpdateBOM (OSXMLCtxtInfo* pXMLInfo, OSXMLEncoding encoding)
{
   switch (encoding) {
   case OSXMLUTF8:
/*       pXMLInfo->byteOrderMark = OSXMLBOM_UTF8; */
      break;
   case OSXMLUTF16:
      pXMLInfo->byteOrderMark = OSXMLBOM_UTF16_BE;
      break;
   case OSXMLUTF16BE:
      pXMLInfo->byteOrderMark = OSXMLBOM_UTF16_BE;
      break;
   case OSXMLUTF16LE:
      pXMLInfo->byteOrderMark = OSXMLBOM_UTF16_LE;
      break;
   default: break;
   }
   return 0;
}

EXTXMLMETHOD int rtXmlSetFormatting (OSCTXT* pctxt, OSBOOL doFormatting)
{
   return rtXmlSetIndent (pctxt, (OSUINT8)((doFormatting) ? OSXMLINDENT : 0));
}

EXTXMLMETHOD int rtXmlSetWriteBOM (OSCTXT* pctxt, OSBOOL write)
{
   if (0 != pctxt->pXMLInfo) {
      OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
      if (write)
         rtXmlUpdateBOM (ctxtInfo, ctxtInfo->encoding);
      else
         ctxtInfo->byteOrderMark = OSXMLBOM_NO_BOM;
      return 0;
   }
   return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
}

EXTXMLMETHOD int rtXmlSetIndent (OSCTXT* pctxt, OSUINT8 indent)
{
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo == NULL) {
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
   }

   ctxtInfo->indent = indent;

   return 0;
}

EXTXMLMETHOD int rtXmlSetIndentChar (OSCTXT* pctxt, char indentChar)
{
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo == NULL) {
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
   }

   ctxtInfo->indentChar = indentChar;

   return 0;
}

EXTXMLMETHOD int rtXmlSetSchemaLocation (OSCTXT* pctxt, const OSUTF8CHAR* schemaLocation)
{
   if (0 != pctxt->pXMLInfo) {
      OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
      if (ctxtInfo->schemaLocation != 0)
         rtxMemFreeArray (pctxt, ctxtInfo->schemaLocation);

      ctxtInfo->schemaLocation =
         rtxUTF8Strdup (pctxt, schemaLocation);
      OSRT_CHECK_EVAL_DATE1(pctxt);
      LCHECKX (pctxt);
      return 0;
   }
   return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
}

EXTXMLMETHOD int rtXmlSetNoNSSchemaLocation (OSCTXT* pctxt,
                                const OSUTF8CHAR* schemaLocation)
{
   if (0 != pctxt->pXMLInfo) {
      OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
      if (ctxtInfo->noNSSchemaLoc != 0)
         rtxMemFreeArray (pctxt, ctxtInfo->noNSSchemaLoc);

      ctxtInfo->noNSSchemaLoc =
         rtxUTF8Strdup (pctxt, schemaLocation);
      LCHECKX (pctxt);
      OSRT_CHECK_EVAL_DATE1(pctxt);
      return 0;
   }
   return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
}

EXTXMLMETHOD int rtXmlSetSchemaLocationByStrFrag
   (OSCTXT* pctxt, const OSUTF8CHAR* schemaLocation, size_t nbytes)
{
   if (0 != pctxt->pXMLInfo) {
      OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
      if (ctxtInfo->schemaLocation != 0)
         rtxMemFreeArray (pctxt, ctxtInfo->schemaLocation);

      ctxtInfo->schemaLocation =
         rtxUTF8Strndup (pctxt, schemaLocation, nbytes);
      LCHECKX (pctxt);
      OSRT_CHECK_EVAL_DATE1(pctxt);
      return 0;
   }
   return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
}

EXTXMLMETHOD int rtXmlSetNoNSSchemaLocationByStrFrag
   (OSCTXT* pctxt, const OSUTF8CHAR* schemaLocation, size_t nbytes)
{
   if (0 != pctxt->pXMLInfo) {
      OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
      if (ctxtInfo->noNSSchemaLoc != 0)
         rtxMemFreeArray (pctxt, ctxtInfo->noNSSchemaLoc);

      ctxtInfo->noNSSchemaLoc =
         rtxUTF8Strndup (pctxt, schemaLocation, nbytes);
      LCHECKX (pctxt);
      OSRT_CHECK_EVAL_DATE1(pctxt);
      return 0;
   }
   return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
}

EXTXMLMETHOD int rtXmlSetXSITypeAttr (OSCTXT* pctxt, const OSUTF8CHAR* xsiType)
{
   if (0 != pctxt->pXMLInfo) {
      ((OSXMLCtxtInfo*)pctxt->pXMLInfo)->xsiTypeAttr =
         rtxUTF8Strdup (pctxt, xsiType);
      LCHECKX (pctxt);
      OSRT_CHECK_EVAL_DATE1(pctxt);
      return 0;
   }
   return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
}

EXTXMLMETHOD int rtXmlSetEncDocHdr (OSCTXT* pctxt, OSBOOL value)
{
   if (value)
      rtxCtxtClearFlag (pctxt, OSXMLFRAG);
   else
      rtxCtxtSetFlag (pctxt, OSXMLFRAG);

   return 0;
}

EXTXMLMETHOD int rtXmlSetDigitsFacets (OSCTXT* pctxt, int totalDigits, int fractionDigits)
{
   OSXMLCtxtInfo* pCtxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (pCtxtInfo != 0) {
      if (totalDigits >= 0 && pCtxtInfo->facets.totalDigits < 0)
         pCtxtInfo->facets.totalDigits = totalDigits;
      if (fractionDigits >= 0 && pCtxtInfo->facets.fractionDigits < 0)
         pCtxtInfo->facets.fractionDigits = fractionDigits;
      return 0;
   }
   return RTERR_NOTINIT;
}

EXTXMLMETHOD void rtXmlSetNamespacesSet (OSCTXT* pctxt, OSBOOL value)
{
   if (value)
      rtxCtxtSetFlag (pctxt, OSXMLNSSET);
   else
      rtxCtxtClearFlag (pctxt, OSXMLNSSET);
}

