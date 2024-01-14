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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtxmlsrc/osrtxml.h"
#include "rtxsrc/rtxToken.h"
#include "rtxmlsrc/rtSaxCppStrList.h"
#ifndef ASN1RT
#include "rtxsrc/rtxCppXmlString.h"
#ifdef HAS_STL
#include "rtxsrc/rtxCppXmlSTLString.h"
#endif /* HAS_STL */
#endif

//////////////////////////////////////////
//                                      //
// OSXMLStrListHandler methods          //
//                                      //
//////////////////////////////////////////

EXTXMLMETHOD int OSXMLStrListHandler::parse
(OSCTXT* pctxt, OSRTMEMBUF *pMemBuf, OSRTDList* pStrList)
{
   int stat = 0;
   OSTOKENCTXT tokCtxt;
   const OSUTF8CHAR* pToken;
   OSUTF8CHAR* pvalue;
   OSUTF8CHAR* pData;
   size_t dataSize;

   if (pMemBuf != 0) {
      pctxt = pMemBuf->pctxt;
      pData = (OSUTF8CHAR*)OSMEMBUFPTR (pMemBuf);
      dataSize = OSMEMBUFUSEDSIZE (pMemBuf);
   }
   else {
      pData = (OSUTF8CHAR*)OSRTBUFPTR (pctxt);
      dataSize = OSRTBUFSIZE (pctxt) + sizeof (OSUTF8CHAR);
   }

   pToken = rtxTokGetFirst (&tokCtxt, pData, dataSize, " \r\n\t\0", 5);

   while (pToken != 0 && stat == 0) {
      pvalue = rtxUTF8Strndup (pctxt, pToken, tokCtxt.lastTokenSize+1);

      rtxDListAppend (pctxt, pStrList, pvalue);

      pToken = rtxTokGetNext (&tokCtxt);
   }
   if (pMemBuf != 0) {
      rtxMemBufCut (pMemBuf, 0, tokCtxt.pStr - OSMEMBUFPTR (pMemBuf));
   }
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}

#ifndef ASN1RT

EXTXMLMETHOD int OSXMLStrListHandler::parse
(OSCTXT* pctxt, OSRTMEMBUF *pMemBuf,
 OSRTObjListClass* pStrList, OSBOOL useSTL)
{
   int stat = 0;
   OSTOKENCTXT tokCtxt;
   const OSUTF8CHAR* pToken;
#ifdef HAS_STL
   OSXMLSTLStringClass* pSTLValue;
#endif
   OSXMLStringClass* pvalue;
   OSUTF8CHAR* pData;
   size_t dataSize;

   if (pMemBuf != 0) {
      pctxt = pMemBuf->pctxt;
      pData = (OSUTF8CHAR*)OSMEMBUFPTR (pMemBuf);
      dataSize = OSMEMBUFUSEDSIZE (pMemBuf);
   }
   else {
      pData = (OSUTF8CHAR*)OSRTBUFPTR (pctxt);
      dataSize = OSRTBUFSIZE (pctxt) + sizeof (OSUTF8CHAR);
   }

   pToken = rtxTokGetFirst (&tokCtxt, pData, dataSize, " \r\n\t\0", 5);

   while (pToken != 0 && stat == 0) {
      if (useSTL) {
#ifdef HAS_STL
         pSTLValue = new OSXMLSTLStringClass
            (pToken, tokCtxt.lastTokenSize+1);
         pStrList->append (pSTLValue);
#else
         return LOG_RTERR (pctxt, RTERR_NOTSUPP); // No STL support.
#endif /* HAS_STL */
      }
      else {
         pvalue = new OSXMLStringClass (pToken, tokCtxt.lastTokenSize+1);
         pStrList->append (pvalue);
      }

      pToken = rtxTokGetNext (&tokCtxt);
   }
   if (pMemBuf != 0) {
      rtxMemBufCut (pMemBuf, 0, tokCtxt.pStr - OSMEMBUFPTR (pMemBuf));
   }
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}

#endif
