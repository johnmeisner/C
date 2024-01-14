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

#include "rtxmlsrc/rtSaxCppParser.h"
#include "rtxsrc/OSRTFileInputStream.h"
#include "rtxsrc/OSRTMemoryInputStream.h"

//////////////////////////////////////////
//                                      //
// OSXMLDefaultHandler methods      //
//                                      //
//////////////////////////////////////////

EXTXMLMETHOD int OSXMLDefaultHandler::startElement
(const OSUTF8CHAR* const /*uri*/, const OSUTF8CHAR* const /*localname*/,
 const OSUTF8CHAR* const /*qname*/, const OSUTF8CHAR* const* /*attrs*/)
{
   return 0;
}

EXTXMLMETHOD int OSXMLDefaultHandler::characters
(const OSUTF8CHAR* const /*chars*/, unsigned int /*length*/)
{
   return 0;
}

EXTXMLMETHOD int OSXMLDefaultHandler::endElement
(const OSUTF8CHAR* const /*uri*/,
 const OSUTF8CHAR* const /*localname*/,
 const OSUTF8CHAR* const /*qname*/)
{
   return 0;
}

EXTXMLMETHOD void OSXMLDefaultHandler::startDocument() {}
EXTXMLMETHOD void OSXMLDefaultHandler::endDocument () {}

EXTXMLMETHOD int OSXMLDefaultHandler::finalize ()
{
   return 0;
}

// SAX Error Info handlers
EXTXMLMETHOD void OSXMLDefaultHandler::resetErrorInfo () {
   errorInfo.stat = 0;
}

EXTXMLMETHOD void OSXMLDefaultHandler::setErrorInfo
   (int status, const char* file, int line)
{
   errorInfo.stat = status;
   errorInfo.file = file;
   errorInfo.line = line;
}

EXTXMLMETHOD int OSXMLDefaultHandler::getErrorInfo
   (int* status, const char** file, int* line)
{
   if (errorInfo.stat == 0) return 0;
   *status = errorInfo.stat;
   *file = errorInfo.file;
   *line = errorInfo.line;
   return 1;
}

#ifdef XML_UNICODE
#define XMLSTRPRINT(a) rtxUCSStrPrint(a)
#else
#define XMLSTRPRINT(a) printf((const char*)a)
#endif

#ifndef _COMPACT
EXTXMLMETHOD void OSXMLDefaultHandler::traceStartElement
(const char* funcName, const OSUTF8CHAR* localName)
{
   if (rtxDiagEnabled (mpContext->getPtr())) {
      printf ("%s: ", funcName);
      XMLSTRPRINT (localName);
      printf (" (%i): start\n", mLevel);
   }
}

EXTXMLMETHOD void OSXMLDefaultHandler::traceEndElement
(const char* funcName, const OSUTF8CHAR* localName)
{
   if (rtxDiagEnabled (mpContext->getPtr())) {
      printf ("%s: ", funcName);
      XMLSTRPRINT (localName);
      printf (" (%i): end\n", mLevel);
   }
}
#endif


EXTXMLMETHOD OSXMLParserCtxt::OSXMLParserCtxt (OSRTContext* pContext) :
   mpContext (pContext)
{
}

EXTXMLMETHOD OSRTInputStreamIF* OSXMLParserCtxt::createInputStream ()
{
   return new OSRTInputStream (mpContext, TRUE);
}

EXTXMLMETHOD OSRTInputStreamIF* OSXMLParserCtxt::createFileInputStream
   (const char* const filename)
{
   return new OSRTFileInputStream (mpContext, filename);
}

EXTXMLMETHOD OSRTInputStreamIF* OSXMLParserCtxt::createMemoryInputStream
   (OSOCTET* pMemBuf, size_t bufSize)
{
   return new OSRTMemoryInputStream (mpContext, pMemBuf, bufSize);
}

EXTXMLMETHOD OSCTXT* OSXMLParserCtxt::getContext ()
{
   return mpContext->getPtr ();
}

EXTXMLMETHOD const OSUTF8CHAR* OSXMLParserCtxt::parseQName (const OSUTF8CHAR* const qname)
{
   return rtXmlDecQName (mpContext->getPtr (), qname, 0);
}


