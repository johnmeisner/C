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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtxmlsrc/osrtxml.hh"
#include "rtxmlsrc/OSXMLMessageBuffer.h"

//////////////////////////////////////////
//                                      //
// OSXMLMessageBuffer methods           //
//                                      //
//////////////////////////////////////////

EXTXMLMETHOD OSXMLMessageBuffer::OSXMLMessageBuffer
(Type bufferType, OSRTContext* pContext) :
OSRTMessageBuffer (bufferType, pContext)
{
   if (getContext()->getStatus() != 0) return;

   int stat = rtXmlInitCtxtAppInfo (getCtxtPtr());
   if (0 != stat) LOG_RTERR (getCtxtPtr(), stat);

   /* Add XML error codes to global table */
   rtErrXmlInit ();
}

EXTXMLMETHOD void* OSXMLMessageBuffer::getAppInfo ()
{
   return rtXmlCtxtAppInfoDup
      (getCtxtPtr(), getCtxtPtr());
}

EXTXMLMETHOD void OSXMLMessageBuffer::setNamespace
(const OSUTF8CHAR* prefix, const OSUTF8CHAR* uri, OSRTDList* pNSAttrs)
{
   rtXmlNSSetNamespace (getCtxtPtr(), pNSAttrs, prefix, uri, TRUE);
}

EXTXMLMETHOD void OSXMLMessageBuffer::setAppInfo (void* pXMLInfo)
{
   rtXmlSetCtxtAppInfo (getCtxtPtr(), (OSXMLCtxtInfo*)pXMLInfo);
#if 0  // don't see why this is needed (ED, 10/12/09)..
   // it is necessary to make a copy of pXMLInfo, using the current memory
   // heap. rtXmlSetCtxtAppInfo does not do copying of pXMLInfo, that is
   // why we need to call ...Dup and ...Set functions.
   pXMLInfo = rtXmlCtxtAppInfoDup
      (getCtxtPtr(), getCtxtPtr());
   rtXmlSetCtxtAppInfo (getCtxtPtr(), (OSXMLCtxtInfo*)pXMLInfo);
#endif
}

EXTXMLMETHOD int OSXMLMessageBuffer::getIndent ()
{
   if (getContext()->getStatus () != 0) return RTERR_NOTINIT;

   return rtXmlGetIndent (getCtxtPtr());
}

EXTXMLMETHOD int OSXMLMessageBuffer::getIndentChar ()
{
   if (getContext()->getStatus () != 0) return RTERR_NOTINIT;

   return rtXmlGetIndentChar (getCtxtPtr());
}

EXTXMLMETHOD OSBOOL OSXMLMessageBuffer::getWriteBOM ()
{
   if (getContext()->getStatus () != 0) return FALSE;

   return rtXmlGetWriteBOM (getCtxtPtr());
}

EXTXMLMETHOD void OSXMLMessageBuffer::setFormatting (OSBOOL doFormatting)
{
   if (getContext()->getStatus () != 0) return;

   rtXmlSetFormatting (getCtxtPtr(), doFormatting);
}

EXTXMLMETHOD void OSXMLMessageBuffer::setIndent (OSUINT8 indent)
{
   if (getContext()->getStatus () != 0) return;

   rtXmlSetIndent (getCtxtPtr(), indent);
}

EXTXMLMETHOD void OSXMLMessageBuffer::setIndentChar (char indentChar)
{
   if (getContext()->getStatus () != 0) return;

   rtXmlSetIndentChar (getCtxtPtr(), indentChar);
}

EXTXMLMETHOD void OSXMLMessageBuffer::setWriteBOM (OSBOOL write)
{
   if (getContext()->getStatus () != 0) return;

   rtXmlSetWriteBOM (getCtxtPtr(), write);
}


/***********************  OSXMLEncodeBase    ********************/

EXTXMLMETHOD OSXMLEncodeBase::OSXMLEncodeBase ( OSRTContext* pContext ) :
      OSXMLMessageBuffer(OSRTMessageBufferIF::XMLEncode, pContext)
{}


// Encode attribute

EXTXMLMETHOD int OSXMLEncodeBase::encodeAttr
(const OSUTF8CHAR* name, const OSUTF8CHAR* value)
{
   return rtXmlEncUTF8Attr (getCtxtPtr(), name, value);
}

// Start XML document

EXTXMLMETHOD int OSXMLEncodeBase::startDocument ()
{
   return rtXmlEncStartDocument (getCtxtPtr());
}

// Encode start element

EXTXMLMETHOD int OSXMLEncodeBase::startElement
(const OSUTF8CHAR* elemName, OSXMLNamespace* pNS,
 OSRTDList* pNSAttrs, OSBOOL terminate)
{
   return rtXmlEncStartElement (getCtxtPtr(), elemName, pNS, pNSAttrs,
                                 terminate);
}

// Terminate open start element

EXTXMLMETHOD int OSXMLEncodeBase::termStartElement()
{
   return rtXmlEncTermStartElement (getCtxtPtr());
}



// Encode XML textual content

EXTXMLMETHOD int OSXMLEncodeBase::encodeText (const OSUTF8CHAR* value)
{
   return rtXmlEncStringValue (getCtxtPtr(), value);
}

// Encode XML end element

EXTXMLMETHOD int OSXMLEncodeBase::endElement
(const OSUTF8CHAR* elemName, OSXMLNamespace* pNS)
{
   return rtXmlEncEndElement (getCtxtPtr(), elemName, pNS);
}

// End XML document

EXTXMLMETHOD int OSXMLEncodeBase::endDocument ()
{
   return rtXmlEncEndDocument (getCtxtPtr());
}
