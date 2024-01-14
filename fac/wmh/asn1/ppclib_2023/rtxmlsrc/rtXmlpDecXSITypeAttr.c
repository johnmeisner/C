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

#include "rtxsrc/rtxErrCodes.h"
#include "rtxmlsrc/osrtxml.h"
#include "rtxmlsrc/rtXmlPull.h"

/* Decode XSI type (xsi:type) attribute */

EXTXMLMETHOD int rtXmlpDecXSITypeAttr (OSCTXT* pctxt,
                           const OSXMLNameFragments* attrName,
                           const OSUTF8CHAR** ppAttrValue)
{
   int stat = 1; /* zero - decoded OK, negative - NOK, > 0 - skipped */
   const OSUTF8CHAR* attrValue;

   if (rtxUTF8StrnEqual (attrName->mPrefix.value, OSUTF8("xsi"), 3) &&
       rtxUTF8StrnEqual (attrName->mLocalName.value, OSUTF8("type"), 4)) {
      stat = rtXmlpDecDynUTF8Str (pctxt, &attrValue);
      if (0 != stat) return LOG_RTERR (pctxt, stat);

      if (ppAttrValue != 0)
         *ppAttrValue = attrValue;

      rtXmlSetEncXSINamespace (pctxt, TRUE);
   }
   else {
      /* If this were an xsi:type attribute, rtXmlpDecDynUTF8Str above would
         have set the data mode to OSXMLDM_NONE, so I do that here.
         Without doing this, subsequent calls to rtXmlpHasAttributes will
         return false.
      */
      OSXMLReader* pXmlReader = rtXmlpGetReader (pctxt);
      OSRTASSERT (0 != pXmlReader);
      pXmlReader->mDataMode = OSXMLDM_NONE;
   }

   return stat;
}

EXTXMLMETHOD int rtXmlpGetXSITypeAttr (OSCTXT* pctxt,
                           const OSUTF8CHAR** ppAttrValue,
                           OSINT16* nsidx, size_t* pLocalOffs)
{
   return rtXmlRdGetXSITypeAttr (XMLPREADER(pctxt), ppAttrValue,
                                 nsidx, pLocalOffs);
}

EXTXMLMETHOD int rtXmlpLookupXSITypeIndex (OSCTXT* pctxt,
                              const OSUTF8CHAR* xsiType,
                              OSINT16 xsiTypeIdx,
                              const OSXMLItemDescr typetab[],
                              size_t typetabsiz)
{
   OSXMLCtxtInfo* xmlCtxt;

   if (!OS_ISEMPTY (xsiType)) {
      const OSXMLItemDescr* p = typetab;
      size_t xsiTypeLen = rtxUTF8LenBytes (xsiType);
      size_t typeidx;

      for (typeidx = 0; typeidx < typetabsiz; typeidx++, p++) {
         if (xsiTypeIdx == p->nsidx && xsiTypeLen == p->localName.length &&
             rtxUTF8StrEqual (xsiType, p->localName.value))
         {
            return (int) typeidx;
         }
      }
   }
   else
      return RTERR_NOTYPEINFO;

   /* unknown xsi:type */

   xmlCtxt = ((OSXMLCtxtInfo*)pctxt->pXMLInfo);

   if (xsiTypeIdx < 0 || xsiTypeIdx > (int) xmlCtxt->nsURITable.nrows)
      rtxErrAddStrParm (pctxt, "?");
   else if (xsiTypeIdx == 0)
      rtxErrAddStrParm (pctxt, "");
   else
      rtxErrAddStrParm (pctxt, (const char*)
         xmlCtxt->nsURITable.data[xsiTypeIdx - 1]);

   rtxErrAddStrParm (pctxt, (const char*) (xsiType));
   return LOG_RTERRNEW (pctxt, XML_E_INVTYPEINFO);
}

EXTXMLMETHOD int rtXmlpGetXSITypeIndex (OSCTXT* pctxt,
                           const OSXMLItemDescr typetab[],
                           size_t typetabsiz)
{
   const OSUTF8CHAR* xsiType;
   OSINT16 xsiTypeIdx;
   size_t offs;
   int stat;

   stat = rtXmlRdGetXSITypeAttr (XMLPREADER(pctxt), &xsiType, &xsiTypeIdx,
                                 &offs);

   if (stat != 0) return stat; /* RTERR_NOMEM */

   if (!OS_ISEMPTY (xsiType)) {
#ifdef XMLPTRACE
      RTDIAG2 (pctxt, "XSI type is '%s'\n", xsiType);
#endif
      rtXmlSetEncXSINamespace (pctxt, TRUE);
      stat = rtXmlpLookupXSITypeIndex (pctxt, xsiType + offs, xsiTypeIdx,
                                       typetab, typetabsiz);
      rtxMemFreePtr (pctxt, xsiType);
   }
   else /* error value processed in upper level */
      stat = RTERR_NOTYPEINFO;

   return stat;
}

