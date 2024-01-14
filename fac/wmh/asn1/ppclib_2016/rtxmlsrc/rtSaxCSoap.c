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

#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxToken.h"

#include "rtxmlsrc/rtSaxCSoap.h"

#define SOAPENVELOPE "http://schemas.xmlsoap.org/soap/envelope/"
#define SOAP12ENVELOPE "http://www.w3.org/2003/05/soap-envelope"

/**************************************************************/
/*                                                            */
/*  soap                                                */
/*                                                            */
/**************************************************************/
EXTXMLMETHOD int SAXSoapStartElement
   (void *userData, const OSUTF8CHAR* localname,
    const OSUTF8CHAR* qname, const OSUTF8CHAR* const* atts)
{
   SoapSaxHandler* pSaxHandler = (SoapSaxHandler*) userData;
   OSSAXHandlerBase* pSaxBase = &pSaxHandler->mSaxBase;
   OSSAXHandlerBase* pMsgSaxBase =
      (OSSAXHandlerBase*)pSaxHandler->mpMsgSaxHandler;
   OSSAXHandlerBase* pFaultMsgSaxBase =
      (OSSAXHandlerBase*)pSaxHandler->mpFaultMsgSaxHandler;
   OSCTXT* pctxt = pSaxBase->mpCtxt;
   const OSUTF8CHAR* nsURI = 0;
   const OSUTF8CHAR* nsPrefix = 0;
   int i, stat;

   SAXDIAGFUNCSTART (pSaxBase, "SAXSoapStartElement", qname);

   if (pSaxBase->mState == 0) { /* SOAP-ENV:Envelope */
      /* parse attributes */
      if (0 != atts) {
         rtXmlNSPush (pctxt);
         i = 0;
         stat = 0;
         for (; 0 != atts[i]; i += 2) {
            OSXMLSETUTF8DECPTR (pctxt, atts[i+1]);

            /* check for namespace attribute (xmlns) */
            if (IS_XMLNSATTR (atts[i])) {
               stat = rtXmlDecNSAttr (pctxt, atts[i], atts[i+1], 0, 0, 0);
               if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);
               nsPrefix = (atts[i][5] == ':') ? &atts[i][6] : 0;

               rtXmlNSAddPrefixLink (pctxt, nsPrefix, atts[i+1], 0, 0);
            }
            else if (IS_XSIATTR (atts[i])) {
               stat = rtXmlDecXSIAttr (pctxt, atts[i], atts[i+1]);
               if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);
            }
            else {
               { LOG_SAXERRNEW2 (pctxt, RTERR_INVATTR,
                  rtxErrAddStrParm (pctxt, "Envelope"),
                  rtxErrAddStrParm (pctxt, (const char*)atts[i]));
                continue; }
            }
         }
      }

      /* lookup prefix */
      rtXmlDecQName (pctxt, qname, &nsPrefix);
      nsURI = rtXmlNSLookupPrefix (pctxt, nsPrefix);

      if (isSoapEnv(nsURI) &&
          rtxUTF8StrEqual (
           (const OSUTF8CHAR*)localname,
           (const OSUTF8CHAR*)"Envelope"))
      {
         pSaxBase->mState++;
      }
      else
      {
         return LOG_SAXERRNEW2_AND_SKIP (pctxt, RTERR_IDNOTFOU,
            rtxErrAddStrParm
               (pctxt, "Envelope"),
            rtxErrAddStrParm (pctxt, (const char*)localname));
      }
   }
   else if (pSaxBase->mLevel == 1 && pSaxBase->mState == 1) {
      /* SOAP-ENV:Header or SOAP-ENV:Body */
      /* lookup prefix */
      rtXmlDecQName (pctxt, qname, &nsPrefix);
      nsURI = rtXmlNSLookupPrefix (pctxt, nsPrefix);

      if (isSoapEnv(nsURI) &&
          rtxUTF8StrEqual (
           (const OSUTF8CHAR*)localname,
           (const OSUTF8CHAR*)"Header"))
      {
         pSaxBase->mState++;
         pSaxHandler->mbParsingHeader = TRUE;
      }
      else if (isSoapEnv(nsURI) &&
               rtxUTF8StrEqual (
           (const OSUTF8CHAR*)localname,
           (const OSUTF8CHAR*)"Body"))
      {
         pSaxBase->mState++;
      }
   }
   else if (pSaxBase->mState >= 2) {
      /* lookup prefix */
      rtXmlDecQName (pctxt, qname, &nsPrefix);
      nsURI = rtXmlNSLookupPrefix (pctxt, nsPrefix);

      if (pSaxHandler->mbFault ||
          (pSaxHandler->mbParsingHeader && pFaultMsgSaxBase != 0)) {
         pFaultMsgSaxBase->mState = 0;
         pFaultMsgSaxBase->mpStartElement
            (pSaxHandler->mpFaultMsgSaxHandler, localname, qname, atts);
         pSaxBase->mState++;
         if (pSaxHandler->mbFault) {
            if (rtxUTF8StrEqual (
                (const OSUTF8CHAR*)localname,
                (const OSUTF8CHAR*)"detail"))
               pSaxHandler->mbProcessingDetailFault = TRUE;

            pSaxHandler->mbEnvelopeParsed = TRUE;
         }
      }
      else if (isSoapEnv(nsURI) &&
               rtxUTF8StrEqual (
           (const OSUTF8CHAR*)localname,
           (const OSUTF8CHAR*)"Fault"))
      {
         pSaxBase->mState++;
         pSaxHandler->mbFault = TRUE;
         LOG_RTERR (pctxt, RTERR_SOAPFAULT);
      }
      else {
         pMsgSaxBase->mpStartElement
            (pSaxHandler->mpMsgSaxHandler, localname, qname, atts);
         pSaxBase->mState++;
         pSaxHandler->mbEnvelopeParsed = TRUE;
      }
   }

   pSaxBase->mLevel++;

   SAXDIAGFUNCEND (pSaxBase, "SAXSoapStartElement", qname);
   return 0;
}

EXTXMLMETHOD int SAXSoapCharacters
   (void *userData, const OSUTF8CHAR* chars, int length)
{
   SoapSaxHandler* pSaxHandler = (SoapSaxHandler*) userData;
   OSSAXHandlerBase* pSaxBase = &pSaxHandler->mSaxBase;
   OSSAXHandlerBase* pMsgSaxBase =
      (OSSAXHandlerBase*)pSaxHandler->mpMsgSaxHandler;
   OSSAXHandlerBase* pFaultMsgSaxBase =
      (OSSAXHandlerBase*)pSaxHandler->mpFaultMsgSaxHandler;

   RTDIAG1 (pSaxBase->mpCtxt, "SAXSoapCharacters: start\n");

   if (pSaxHandler->mbParsingHeader && pFaultMsgSaxBase != 0) {
      if (pSaxBase->mState >= 3) {
         pFaultMsgSaxBase->mpCharacters
            (pSaxHandler->mpFaultMsgSaxHandler, chars, length);
      }
   }
   else if (pSaxHandler->mbFault && pFaultMsgSaxBase != 0) {
      if (pSaxHandler->mbProcessingDetailFault) {
         if (pSaxBase->mState >= 5) {
            pFaultMsgSaxBase->mpCharacters
               (pSaxHandler->mpFaultMsgSaxHandler, chars, length);
         }
      }
      else if (pSaxBase->mState >= 4) {
         pFaultMsgSaxBase->mpCharacters
            (pSaxHandler->mpFaultMsgSaxHandler, chars, length);
      }
   }
   else if (pSaxBase->mState >= 3) {
      pMsgSaxBase->mpCharacters
         (pSaxHandler->mpMsgSaxHandler, chars, length);
   }

   RTDIAG1 (pSaxBase->mpCtxt, "SAXSoapCharacters: end\n");
   return 0;
}

EXTXMLMETHOD int SAXSoapEndElement
   (void *userData, const OSUTF8CHAR* localname, const OSUTF8CHAR* qname)
{
   SoapSaxHandler* pSaxHandler = (SoapSaxHandler*) userData;
   OSSAXHandlerBase* pSaxBase = &pSaxHandler->mSaxBase;
   OSSAXHandlerBase* pMsgSaxBase =
      (OSSAXHandlerBase*)pSaxHandler->mpMsgSaxHandler;
   OSSAXHandlerBase* pFaultMsgSaxBase =
      (OSSAXHandlerBase*)pSaxHandler->mpFaultMsgSaxHandler;
   OSCTXT* pctxt = pSaxBase->mpCtxt;
   const OSUTF8CHAR* nsURI = 0;
   const OSUTF8CHAR* nsPrefix = 0;

   SAXDIAGFUNCSTART (pSaxBase, "SAXSoapEndElement", qname);

   pSaxBase->mLevel = pSaxBase->mLevel - 1;

   if (pSaxHandler->mbParsingHeader && pFaultMsgSaxBase != 0) {
      if (pSaxBase->mLevel > 1 && pSaxBase->mState >= 2) {
         pFaultMsgSaxBase->mpEndElement
            (pSaxHandler->mpFaultMsgSaxHandler, localname, qname);
         pSaxBase->mState--;
      }
      else if (pSaxBase->mLevel == 1) {
         /* lookup prefix */
         rtXmlDecQName (pctxt, qname, &nsPrefix);
         nsURI = rtXmlNSLookupPrefix (pctxt, nsPrefix);

         if (isSoapEnv(nsURI) &&
             rtxUTF8StrEqual (
              (const OSUTF8CHAR*)localname,
              (const OSUTF8CHAR*)"Header"))
         {
            pSaxBase->mState--;
            pSaxHandler->mbParsingHeader = FALSE;
         }
      }
   }
   else if (pSaxHandler->mbFault &&
            (pSaxBase->mLevel >= 2 && pSaxBase->mState >= 3)) {
      if (pSaxHandler->mbProcessingDetailFault && pFaultMsgSaxBase != 0) {
         if (pSaxBase->mLevel > 2 && pSaxBase->mState >= 4) {
            pFaultMsgSaxBase->mpEndElement
               (pSaxHandler->mpFaultMsgSaxHandler, localname, qname);
            pSaxBase->mState--;
         }
         if (rtxUTF8StrEqual (
             (const OSUTF8CHAR*)localname,
             (const OSUTF8CHAR*)"detail"))
            pSaxHandler->mbProcessingDetailFault = FALSE;
      }
      else if (pSaxBase->mLevel > 2 && pSaxBase->mState >= 3 &&
               pFaultMsgSaxBase != 0) {
         pFaultMsgSaxBase->mpEndElement
            (pSaxHandler->mpFaultMsgSaxHandler, localname, qname);
         pSaxBase->mState--;
      }
      else if (pSaxBase->mLevel == 2) {
         /* lookup prefix */
         rtXmlDecQName (pctxt, qname, &nsPrefix);
         nsURI = rtXmlNSLookupPrefix (pctxt, nsPrefix);

         if (isSoapEnv(nsURI) &&
             rtxUTF8StrEqual (
           (const OSUTF8CHAR*)localname,
           (const OSUTF8CHAR*)"Fault"))
         {
            pSaxBase->mState--;
         }
      }
   }
   else if (pSaxBase->mLevel > 1 && pSaxBase->mState >= 2) {
      pMsgSaxBase->mpEndElement
         (pSaxHandler->mpMsgSaxHandler, localname, qname);
      pSaxBase->mState--;
   }
   else if (pSaxBase->mLevel == 1) {
      /* lookup prefix */
      rtXmlDecQName (pctxt, qname, &nsPrefix);
      nsURI = rtXmlNSLookupPrefix (pctxt, nsPrefix);

      if (isSoapEnv(nsURI) &&
          rtxUTF8StrEqual (
           (const OSUTF8CHAR*)localname,
           (const OSUTF8CHAR*)"Header"))
      {
         pSaxBase->mState--;
         pSaxHandler->mbParsingHeader = FALSE;
      }
      else if (isSoapEnv(nsURI) &&
               rtxUTF8StrEqual (
           (const OSUTF8CHAR*)localname,
           (const OSUTF8CHAR*)"Body"))
      {
         pSaxBase->mState--;
      }
   }
   else if (pSaxBase->mLevel == 0) {
      /* lookup prefix */
      rtXmlDecQName (pctxt, qname, &nsPrefix);
      nsURI = rtXmlNSLookupPrefix (pctxt, nsPrefix);
      rtXmlNSPop (pctxt);

      if (isSoapEnv(nsURI) &&
          rtxUTF8StrEqual (
           (const OSUTF8CHAR*)localname,
           (const OSUTF8CHAR*)"Envelope"))
      {
         pSaxBase->mState--;
         if (!pSaxHandler->mbEnvelopeParsed && !pSaxHandler->mbFault)
            return LOG_SAXERRNEW_AND_SKIP (pctxt, RTERR_XMLSTATE);
      }
      else {
         return LOG_SAXERRNEW2_AND_SKIP (pctxt, RTERR_IDNOTFOU,
            rtxErrAddStrParm
               (pctxt, "Envelope"),
            rtxErrAddStrParm (pctxt, (const char*)localname));
      }
   }

   SAXDIAGFUNCEND (pSaxBase, "SAXSoapEndElement", qname);
   return 0;
}

EXTXMLMETHOD void SAXSoapFree (OSCTXT* pctxt, SoapSaxHandler* pSaxHandler)
{
   if (pctxt) {} /* to suppress level 4 warning */
   rtxMemBufFree (&pSaxHandler->mCurrElemValue);
}

EXTXMLMETHOD int SAXSoapInit
   (OSCTXT* pctxt, SoapSaxHandler* pSaxHandler,
    void* pMsgSaxHandler, void* pFaultMsgSaxHandler)
{
   OSCRTLMEMSET (pSaxHandler, 0, sizeof (*pSaxHandler));
   rtxMemBufInit (pctxt, &pSaxHandler->mCurrElemValue, OSRTMEMBUFSEG);

   pSaxHandler->mSaxBase.mpCtxt = pctxt;
   pSaxHandler->mSaxBase.mpStartElement =
      (CSAX_StartElementHandler)SAXSoapStartElement;
   pSaxHandler->mSaxBase.mpEndElement   =
      (CSAX_EndElementHandler)SAXSoapEndElement;
   pSaxHandler->mSaxBase.mpCharacters   =
      (CSAX_CharacterDataHandler)SAXSoapCharacters;
   pSaxHandler->mSaxBase.mpFree = (CSAX_FreeHandler)SAXSoapFree;

   pSaxHandler->mpMsgSaxHandler = pMsgSaxHandler;
   pSaxHandler->mpFaultMsgSaxHandler = pFaultMsgSaxHandler;
   return 0;
}

OSBOOL isSoapEnv(const OSUTF8CHAR* uri)
{
    return ((OSBOOL)(rtxUTF8StrEqual (uri, OSUTF8(SOAPENVELOPE)) ||
            rtxUTF8StrEqual (uri, OSUTF8(SOAP12ENVELOPE))));
}

