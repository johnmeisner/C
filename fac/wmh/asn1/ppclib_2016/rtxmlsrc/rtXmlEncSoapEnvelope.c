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
#include "rtxmlsrc/osrtxml.h"

static const char gSoapEnvElemName[] = "Envelope";
static const char gSoapEnvNSPrefix[] = "SOAP-ENV";
static const char gSoapEnvNSURI[]= "http://schemas.xmlsoap.org/soap/envelope/";
static const char gSoap12EnvNSURI[]= "http://www.w3.org/2003/05/soap-envelope";

EXTXMLMETHOD void rtXmlSetSoapVersion (OSCTXT* pctxt, OSUINT8 version)
{
   OSXMLCtxtInfo* pXmlCtxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
   if (pXmlCtxtInfo != 0) {
      pXmlCtxtInfo->soapVersion = version;
   }
}

EXTXMLMETHOD int rtXmlEncStartSoapEnv (OSCTXT* pctxt, OSRTDList* pNSAttrs)
{
   int stat;
   OSXMLNamespace ns;
   OSXMLCtxtInfo* pXmlCtxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
   pctxt->level = 0;

   ns.prefix = OSUTF8(gSoapEnvNSPrefix);
   if (pXmlCtxtInfo != 0) {
      if (pXmlCtxtInfo->soapVersion == 11)
         ns.uri = OSUTF8(gSoapEnvNSURI);
      else if (pXmlCtxtInfo->soapVersion == 12)
         ns.uri = OSUTF8(gSoap12EnvNSURI);
   } else {
      ns.uri = OSUTF8(gSoap12EnvNSURI);
   }

   stat = rtXmlEncStartElement
      (pctxt, OSUTF8(gSoapEnvElemName), &ns, pNSAttrs, FALSE);

   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* add namespace attributes */

   rtXmlNSSetNamespace (pctxt, 0, ns.prefix, ns.uri, FALSE);

#if 0
   if (pXmlCtxtInfo != 0) {
      if (pXmlCtxtInfo->soapVersion == 11)
         stat = rtXmlEncUTF8Attr2
            (pctxt, (const OSUTF8CHAR*)"xmlns:SOAP-ENV", 14,
            (const OSUTF8CHAR*)gSoapEnvNSURI, sizeof(gSoapEnvNSURI)-1);
      else if (pXmlCtxtInfo->soapVersion == 12)
         stat = rtXmlEncUTF8Attr2
            (pctxt, (const OSUTF8CHAR*)"xmlns:SOAP-ENV", 14,
            (const OSUTF8CHAR*)gSoap12EnvNSURI, sizeof(gSoap12EnvNSURI)-1);
   }
#endif

   stat = rtXmlEncXSIAttrs (pctxt, TRUE);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   if (0 != pNSAttrs && 0 != pNSAttrs->count) {
      stat = rtXmlEncNSAttrs (pctxt, pNSAttrs);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   stat = rtXmlEncNSAttrs (pctxt, 0);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   rtXmlEncTermStartElement (pctxt);

   return 0;
}

EXTXMLMETHOD int rtXmlEncStartSoapElems (OSCTXT* pctxt, OSXMLSOAPMsgType msgtype)
{
   OSXMLNamespace ns;
   OSXMLCtxtInfo* pXmlCtxtInfo = pctxt->pXMLInfo;
   int stat = rtXmlEncStartSoapEnv (pctxt, 0);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   ns.prefix = OSUTF8(gSoapEnvNSPrefix);
   if (pXmlCtxtInfo != 0) {
      if (pXmlCtxtInfo->soapVersion == 11)
         ns.uri = OSUTF8(gSoapEnvNSURI);
      else if (pXmlCtxtInfo->soapVersion == 12)
         ns.uri = OSUTF8(gSoap12EnvNSURI);
   }

   switch (msgtype) {
   case OSSOAPHEADER:
      stat = rtXmlEncStartElement (pctxt, OSUTF8("Header"), &ns, 0, FALSE);
      break;

   case OSSOAPBODY:
      stat = rtXmlEncStartElement (pctxt, OSUTF8("Body"), &ns, 0, FALSE);
      break;

   case OSSOAPFAULT:
      stat = rtXmlEncStartElement (pctxt, OSUTF8("Fault"), &ns, 0, FALSE);
      break;

   default:;
   }

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}

EXTXMLMETHOD int rtXmlEncEndSoapEnv (OSCTXT* pctxt)
{
   OSXMLNamespace ns;
   OSXMLCtxtInfo* pXmlCtxtInfo = pctxt->pXMLInfo;
   ns.prefix = OSUTF8(gSoapEnvNSPrefix);
   if (pXmlCtxtInfo != 0) {
      if (pXmlCtxtInfo->soapVersion == 11)
         ns.uri = OSUTF8(gSoapEnvNSURI);
      else if (pXmlCtxtInfo->soapVersion == 12)
         ns.uri = OSUTF8(gSoap12EnvNSURI);
   }

   return rtXmlEncEndElement (pctxt, OSUTF8(gSoapEnvElemName), &ns);
}

EXTXMLMETHOD int rtXmlEncEndSoapElems (OSCTXT* pctxt, OSXMLSOAPMsgType msgtype)
{
   OSXMLNamespace ns;
   OSXMLCtxtInfo* pXmlCtxtInfo = pctxt->pXMLInfo;
   int stat;

   ns.prefix = OSUTF8(gSoapEnvNSPrefix);
   if (pXmlCtxtInfo != 0) {
      if (pXmlCtxtInfo->soapVersion == 11)
         ns.uri = OSUTF8(gSoapEnvNSURI);
      else if (pXmlCtxtInfo->soapVersion == 12)
         ns.uri = OSUTF8(gSoap12EnvNSURI);
   }

   switch (msgtype) {
   case OSSOAPHEADER:
      stat = rtXmlEncEndElement (pctxt, OSUTF8("Header"), &ns);
      break;

   case OSSOAPBODY:
      stat = rtXmlEncEndElement (pctxt, OSUTF8("Body"), &ns);
      break;

   case OSSOAPFAULT:
      stat = rtXmlEncEndElement (pctxt, OSUTF8("Fault"), &ns);
      break;

   default: stat = 0;
   }
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return rtXmlEncEndSoapEnv (pctxt);
}

