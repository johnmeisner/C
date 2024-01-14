/*
 * Copyright (c) 1997-2018 Objective Systems, Inc.
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

#include <stdlib.h>
#include <string.h>
#include "rtxersrc/asn1XerCppTypes.h"
#include "rtsrc/rt_common.hh"
#include "rtxersrc/xed_common.hh"
#include "rtxsrc/OSRTInputStream.h"

//////////////////////////////////////////
//                                      //
// ASN1XERMessageBuffer methods         //
//                                      //
//////////////////////////////////////////

ASN1XERMessageBuffer::ASN1XERMessageBuffer (Type bufferType) :
   ASN1MessageBuffer(bufferType)
{
   if (getStatus() != 0) return;

   // Initialize XML application info in the context
   int stat = xerInitCtxtAppInfo (getCtxtPtr());
   if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);
}

//////////////////////////////////////////
//                                      //
// ASN1XEREncodeBuffer methods          //
//                                      //
//////////////////////////////////////////

ASN1XEREncodeBuffer::ASN1XEREncodeBuffer () :
   ASN1XERMessageBuffer(XEREncode)
{
   if (getStatus () != 0) return;

   int stat = xerSetEncBufPtr (getCtxtPtr(), 0, 0, FALSE);
   if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);
}

ASN1XEREncodeBuffer::ASN1XEREncodeBuffer (OSBOOL canonical) :
   ASN1XERMessageBuffer(XEREncode)
{
   if (getStatus () != 0) return;

   int stat = xerSetEncBufPtr (getCtxtPtr(), 0, 0, canonical);
   if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);
}

ASN1XEREncodeBuffer::ASN1XEREncodeBuffer
   (OSBOOL canonical, OSRTContext* pContext, size_t initBufSize) :
    ASN1XERMessageBuffer (XEREncode, pContext)
{
   if (getStatus () != 0) return;

   OSCTXT* pctxt = getCtxtPtr();
   // rtxCtxtSetMemHeap (pctxt, pSrcCtxt);
   int stat = xerSetEncBufPtr (pctxt, 0, initBufSize, canonical);
   if (stat != 0) LOG_RTERR (pctxt, stat);
}

ASN1XEREncodeBuffer::ASN1XEREncodeBuffer
   (OSOCTET* pMsgBuf, size_t msgBufLen) :
    ASN1XERMessageBuffer (XEREncode)
{
   if (getStatus () != 0) return;

   int stat = xerSetEncBufPtr (getCtxtPtr(), pMsgBuf, msgBufLen, FALSE);
   if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);
}

ASN1XEREncodeBuffer::ASN1XEREncodeBuffer (
   OSOCTET* pMsgBuf, size_t msgBufLen, OSBOOL canonical, OSBOOL openType) :
   ASN1XERMessageBuffer (XEREncode)
{
   if (getStatus () != 0) return;

   int stat = xerSetEncBufPtr (getCtxtPtr(), pMsgBuf, msgBufLen, canonical);
   if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);
   if (openType) setOpenType ();
}

// This method resets the encode buffer to allow another message to be
// encoded..
int ASN1XEREncodeBuffer::init ()
{
   if (getStatus () != 0) return getStatus ();

   OSCTXT* pCtxt = getCtxtPtr();
   pCtxt->buffer.byteIndex = 0;
   return 0;
}

// Write XML document out to given filename
// returns: number of bytes written or negative status code if error

long ASN1XEREncodeBuffer::write (const char* filename)
{
   if (getStatus () != 0) return getStatus ();

   FILE* fp;
   long nbytes = -1;
   fp = fopen (filename, "wb");
   if (0 != fp) {
      nbytes = write (fp);
      fclose (fp);
   }
   return nbytes;
}

// Write XML document out to given file
// returns: number of bytes written or negative status code if error

long ASN1XEREncodeBuffer::write (FILE* fp)
{
   if (getStatus () != 0) return getStatus ();

   return (long)fwrite (getMsgPtr(), 1, getMsgLen(), fp);
}

// Shared memory heap encode buffer object

ASN1XERShdMemHpEncBuf::ASN1XERShdMemHpEncBuf
(OSBOOL canonical, OSRTContext* pContext, size_t initBufSize) :
   ASN1XEREncodeBuffer (canonical)
{
   if (getStatus() != 0 || 0 == pContext) return;

   // We need to keep the original memory heap around because items in
   // the context may have been allocated on initialization (for example,
   // application info blocks)..
   OSCTXT* pctxt = getCtxtPtr();
   mpSavedMemHeap = pctxt->pMemHeap;

   OSCTXT* pSrcCtxt = pContext->getPtr();
   pctxt->pMemHeap = pSrcCtxt->pMemHeap;

   pctxt->key = pSrcCtxt->key;
   pctxt->keylen = pSrcCtxt->keylen;

   int stat = xerSetEncBufPtr (pctxt, 0, initBufSize, canonical);
   if (stat != 0) LOG_RTERR (pctxt, stat);
}

ASN1XERShdMemHpEncBuf::ASN1XERShdMemHpEncBuf (OSBOOL canonical) :
   ASN1XEREncodeBuffer (canonical)
{
}

ASN1XERShdMemHpEncBuf::~ASN1XERShdMemHpEncBuf()
{
   // Replace the memory heap in the context with the original memory
   // heap.  This will allow the context to be properly destructed.
   OSCTXT* pctxt = getCtxtPtr();
   pctxt->pMemHeap = mpSavedMemHeap;
}

//////////////////////////////////////////
//                                      //
// ASN1XERDecodeBuffer methods          //
//                                      //
//////////////////////////////////////////

ASN1XERDecodeBuffer::ASN1XERDecodeBuffer (const char* xmlFile) :
   ASN1XERMessageBuffer (XERDecode)
{
   // Create a SAX parser object
   mInputId = INPUT_FILE;
   mInput.fileName = xmlFile;
}

ASN1XERDecodeBuffer::ASN1XERDecodeBuffer (const OSOCTET* msgbuf,
                                          size_t numocts,
                                          OSBOOL openType) :
   ASN1XERMessageBuffer (XERDecode)
{
   // Create a SAX parser object
   mInputId = INPUT_MEMORY;
   mInput.memBuf.pMemBuf = msgbuf;
   mInput.memBuf.bufSize = numocts;

   if (openType) setOpenType ();
}

ASN1XERDecodeBuffer::ASN1XERDecodeBuffer
   (OSRTInputStreamIF& inputStream, OSBOOL openType) :
   ASN1XERMessageBuffer (XERDecode)
{
   // Create a SAX parser object
   mInputId = INPUT_STREAM_ATTACHED;
   mInput.pInputStream = &inputStream;

   if (openType) setOpenType ();
}

ASN1XERDecodeBuffer::~ASN1XERDecodeBuffer ()
{
   // Delete the memory buffer - this does not look right.  Stream is
   // passed as an argument into the object.  We should not be deleting
   // something someone else is the owner of (ED, 7/31/06)..
   // if (mInputId == INPUT_STREAM_ATTACHED)
   //    delete mInput.pInputStream;
}

int ASN1XERDecodeBuffer::decodeXML (OSXMLReaderClass* pParser)
{
   if (getStatus () != 0) return getStatus ();

   LCHECKXER (getCtxtPtr());

   int stat = 0;

   // Parse the document
   switch (mInputId) {
      case INPUT_FILE:
         stat = pParser->parse (mInput.fileName);
         break;

      case INPUT_STREAM:
      case INPUT_STREAM_ATTACHED:
         stat = pParser->parse (*mInput.pInputStream);
         break;

      case INPUT_MEMORY:
         stat = pParser->parse
            ((const char*)mInput.memBuf.pMemBuf, mInput.memBuf.bufSize);
         break;
   }
   if (stat != 0) return stat;

   return getStatus ();
}

int ASN1XERDecodeBuffer::initBuffer (const OSUTF8CHAR* str)
{
   int len = xerTextLength(str) + 1;
   OSOCTET* data = (OSOCTET*) str;
   return rtxInitContextBuffer (getCtxtPtr(), data, len);
}

//////////////////////////////////////////
//                                      //
// ASN1XERSAXDecodeHandler methods      //
//                                      //
//////////////////////////////////////////

int ASN1XERSAXDecodeHandler::startElement
(const OSUTF8CHAR* const uri, const OSUTF8CHAR* const localname,
 const OSUTF8CHAR* const qname, const OSUTF8CHAR* const* attrs)
{
   return 0;
}

int ASN1XERSAXDecodeHandler::characters
(const OSUTF8CHAR* const chars, unsigned int length)
{
   return 0;
}

int ASN1XERSAXDecodeHandler::endElement
(const OSUTF8CHAR* const uri, const OSUTF8CHAR* const localname,
 const OSUTF8CHAR* const qname)
{
   return 0;
}

void ASN1XERSAXDecodeHandler::startDocument() {}
void ASN1XERSAXDecodeHandler::endDocument () {}

// SAX Error Info handlers
void ASN1XERSAXDecodeHandler::resetErrorInfo () {
   errorInfo.stat = 0;
}

void ASN1XERSAXDecodeHandler::setErrorInfo
   (int status, const char* file, int line)
{
   errorInfo.stat = status;
   errorInfo.file = file;
   errorInfo.line = line;
}

int ASN1XERSAXDecodeHandler::getErrorInfo
   (int* status, const char** file, int* line)
{
   if (errorInfo.stat == 0) return 0;
   *status = errorInfo.stat;
   *file = errorInfo.file;
   *line = errorInfo.line;
   return 1;
}

OSCTXT* ASN1XERSAXDecodeHandler::finalizeMemBuf
   (OSRTMessageBufferIF* pMsgBuf, OSRTMEMBUF& memBuf)
{
   int stat;
   OSCTXT* pctxt = pMsgBuf->getCtxtPtr();

   /* Ensure capacity for data */
   stat = rtxMemBufPreAllocate (&memBuf, sizeof (OSUTF8CHAR));
   if (stat == 0) {
      stat = ((ASN1XERDecodeBuffer*)pMsgBuf)->initBuffer (memBuf);
      rtxMemBufSet (&memBuf, 0, sizeof (OSUTF8CHAR));
   }

   if (stat != 0) LOG_RTERR (pctxt, stat);

   return pctxt;
}
