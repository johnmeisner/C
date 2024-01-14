/*
 * Copyright (c) 1997-2023 Objective Systems, Inc.
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
#include "asn1OerCppTypes.h"
#include "rtxsrc/rtxContext.hh"
#include "rtxsrc/rtxDiagBitTrace.h"
#include "rtxsrc/rtxFile.h"

#if !defined(_NO_CPP_STREAM)
#include "rtxsrc/OSRTInputStream.h"
#include "rtxsrc/OSRTOutputStream.h"
#endif

//////////////////////////////////////////
//                                      //
// ASN1OERMessageBuffer methods          //
//                                      //
//////////////////////////////////////////

EXTOERMETHOD ASN1OERMessageBuffer::ASN1OERMessageBuffer
(Type bufferType) : ASN1MessageBuffer(bufferType)
{
   if (!OSRTISSTREAM (getCtxtPtr())) {
      int stat = rtxInitContextBuffer (getCtxtPtr(), 0, 0);
      if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);
   }
}

EXTOERMETHOD ASN1OERMessageBuffer::ASN1OERMessageBuffer
(Type bufferType, OSOCTET* pMsgBuf, size_t msgBufLen) :
   ASN1MessageBuffer (bufferType)
{
   int stat = rtxInitContextBuffer(getCtxtPtr(), pMsgBuf, msgBufLen);
   if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);
}

EXTOERMETHOD ASN1OERMessageBuffer::ASN1OERMessageBuffer
(Type bufferType, OSOCTET* pMsgBuf, size_t msgBufLen,
 OSRTContext *pContext) : ASN1MessageBuffer (bufferType, pContext)
{
   int stat = rtxInitContextBuffer(getCtxtPtr(), pMsgBuf, msgBufLen);
   if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);
}

#if !defined(_NO_CPP_STREAM)
EXTOERMETHOD ASN1OERMessageBuffer::ASN1OERMessageBuffer
(OSRTStream& stream) :
   ASN1MessageBuffer (OEREncode, stream.getContext())
{
   OSCTXT* pctxt = getCtxtPtr();
   if (0 == pctxt->pASN1Info) {
      int stat = rtCtxtInitASN1Info (pctxt);
      if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);
   }
}
#endif

EXTOERMETHOD int ASN1OERMessageBuffer::setBuffer
(const OSOCTET* pMsgBuf, size_t msgBufLen)
{
   if (getStatus () != 0) return getStatus ();

   OSCTXT* pctxt = getCtxtPtr();

   LCHECKCPP (pctxt);

   int stat = rtxInitContextBuffer (pctxt, (OSOCTET*)pMsgBuf, msgBufLen);

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}

//////////////////////////////////////////
//                                      //
// ASN1OEREncodeBuffer methods          //
//                                      //
//////////////////////////////////////////

#if !defined(_NO_CPP_STREAM)
EXTOERMETHOD ASN1OEREncodeBuffer::ASN1OEREncodeBuffer
(OSRTOutputStream& ostream) :
   ASN1OERMessageBuffer (ostream)
{}
#endif


EXTOERMETHOD OSOCTET* ASN1OEREncodeBuffer::getMsgCopy ()
{
   if (getStatus () != 0) return 0;

   size_t msgLen;
   OSCTXT*  pCtxt = getCtxtPtr();
   OSOCTET* msg_p = pCtxt->buffer.data;
   msgLen = rtxCtxtGetMsgLen(pCtxt);

   LCHECKX2 (pCtxt);

   if (msg_p) {
      OSOCTET* temp_p = msg_p;
      msg_p = new OSOCTET [msgLen];
      memcpy (msg_p, temp_p, msgLen);
   }

   return (msg_p);
}


// This method returns a pointer to the encoded message.  The encode
// buffer class retains ownership of the message; therefore, this
// pointer will become invalid when the encode buffer object instance
// goes out-of-scope..

EXTOERMETHOD const OSOCTET* ASN1OEREncodeBuffer::getMsgPtr ()
{
   if (getStatus () != 0) return 0;
   return getCtxtPtr()->buffer.data;
}

// This method initializes the encode buffer for encoding
// Frees the memory used by trace information of previous encode
// This function should only keep memory heap / stream & reset all other
// parameter of context
EXTOERMETHOD int ASN1OEREncodeBuffer::init ()
{
   if (getStatus () != 0) return getStatus ();

   OSCTXT* pCtxt = getCtxtPtr();
   pCtxt->buffer.byteIndex = 0;
   pCtxt->buffer.bitOffset = 8;
   if (0 != pCtxt->pBitFldList) {
      rtxSListFree (&pCtxt->pBitFldList->fieldList);
   }
   return 0;
}

EXTOERMETHOD int ASN1OEREncodeBuffer::writeBytes
(const OSOCTET* buffer, size_t nbytes)
{
   return rtxWriteBytes (getCtxtPtr(), buffer, nbytes);
}

//////////////////////////////////////////
//                                      //
// ASN1OERDecodeBuffer methods          //
//                                      //
//////////////////////////////////////////

#if !defined(_NO_CPP_STREAM)
EXTOERMETHOD ASN1OERDecodeBuffer::ASN1OERDecodeBuffer
(OSRTInputStream& istream) :
   ASN1OERMessageBuffer (istream)
{}
#endif

EXTOERMETHOD
ASN1OERDecodeBuffer::ASN1OERDecodeBuffer
(const char* filePath) :
   ASN1OERMessageBuffer (OERDecode, 0, 0)
{
   if (getStatus () != 0) return;

   int stat = readBinaryFile (filePath);
   if (stat != 0) getContext()->setStatus (stat);
}

EXTOERMETHOD int ASN1OERDecodeBuffer::peekByte (OSOCTET& ub)
{
   OSCTXT* pctxt = getCtxtPtr();
   size_t pos;

   int stat = rtxMarkPos (pctxt, &pos);
   if (0 == stat) {
      stat = rtxReadBytes (pctxt, &ub, 1);
   }
   if (0 == stat) {
      stat = rtxResetToPos (pctxt, pos);
   }

   return stat;
}

EXTOERMETHOD int ASN1OERDecodeBuffer::readBinaryFile (const char* filePath)
{
   OSOCTET* pMsgBuf;
   OSCTXT* pctxt = getCtxtPtr();
   size_t msgLen = 0;
   int stat = rtxFileReadBinary (pctxt, filePath, &pMsgBuf, &msgLen);

   if (stat == 0)
      stat = setBuffer (pMsgBuf, msgLen);

   return getContext()->setStatus (stat);
}

EXTOERMETHOD int ASN1OERDecodeBuffer::readBytes
(OSOCTET* buffer, size_t bufsize, size_t nbytes)
{
   return (nbytes <= bufsize) ?
      rtxReadBytes (getCtxtPtr(), buffer, nbytes) : RTERR_BUFOVFLW;
}

