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
#include "rtxsrc/OSRTFileInputStream.h"
#include "rtxsrc/OSRTMemoryInputStream.h"
#include "rtxmlsrc/OSXMLDecodeBuffer.h"

//////////////////////////////////////////
//                                      //
// OSXMLDecodeBuffer methods            //
//                                      //
//////////////////////////////////////////

EXTXMLMETHOD OSXMLDecodeBuffer::OSXMLDecodeBuffer (const char* xmlFile) :
   OSXMLMessageBuffer (XMLDecode), mpInputStream(0), mbOwnStream(FALSE)
{
   if (0 != xmlFile) {
      mpInputStream = new OSRTFileInputStream (getContext(), xmlFile);
      mbOwnStream = TRUE;
   }
}

EXTXMLMETHOD OSXMLDecodeBuffer::OSXMLDecodeBuffer (const OSOCTET* msgbuf, size_t bufsiz) :
   OSXMLMessageBuffer (XMLDecode), mpInputStream(0), mbOwnStream(FALSE)
{
   if (0 != msgbuf) {
      mpInputStream = new OSRTMemoryInputStream (getContext(), msgbuf, bufsiz);
      mbOwnStream = TRUE;
   }
}

EXTXMLMETHOD OSXMLDecodeBuffer::OSXMLDecodeBuffer (OSRTInputStream& inputStream) :
   OSXMLMessageBuffer (XMLDecode, inputStream.getContext())
{
   mpInputStream = &inputStream;
   mbOwnStream = FALSE;
}

EXTXMLMETHOD OSXMLDecodeBuffer::~OSXMLDecodeBuffer ()
{
   // Delete input stream object if this object created it.
   if (mbOwnStream)
      delete mpInputStream;
}

EXTXMLMETHOD int OSXMLDecodeBuffer::decodeXML (OSXMLReaderClass* pParser)
{
   int stat = RTERR_NOTINIT;

   if (0 != mpInputStream) {
      stat = pParser->parse (*mpInputStream);
   }
   if (stat == 0)
      return rtxErrGetFirstError (getCtxtPtr());

   return stat;
}

EXTXMLMETHOD OSBOOL OSXMLDecodeBuffer::isWellFormed ()
{
   if (0 != mpInputStream) {
      size_t currPos;

      int ret = mpInputStream->getPosition (&currPos);
      if (0 != ret) return FALSE;

      ret = rtXmlpDecAny (getCtxtPtr(), 0);
      if (0 != ret) return FALSE;

      ret = mpInputStream->setPosition (currPos);
      return (0 != ret) ? FALSE : TRUE;
   }
   else return FALSE;
}

EXTXMLMETHOD OSUINT32 OSXMLDecodeBuffer::setMaxErrors (OSUINT32 maxErrors)
{
  return rtSaxSetMaxErrors (getCtxtPtr(), maxErrors);
}


EXTXMLMETHOD int OSXMLDecodeBuffer::parseElementName (OSUTF8CHAR** ppName)
{
   return rtXmlParseElementName (getCtxtPtr(), ppName);
}

EXTXMLMETHOD int OSXMLDecodeBuffer::parseElemQName (OSXMLQName* pQName)
{
   return rtXmlParseElemQName (getCtxtPtr(), pQName);
}

EXTXMLMETHOD int OSXMLDecodeBuffer::init ()
{
   return 0;
}


