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
#include "asn1PerCppTypes.h"
#include "rtsrc/rt_common.hh"
#include "rtxsrc/rtxFile.h"

#if !defined(_NO_CPP_STREAM)
#include "rtxsrc/OSRTInputStream.h"
#include "rtxsrc/OSRTOutputStream.h"
#include "rtxsrc/rtxStream.h"
#endif

//////////////////////////////////////////
//                                      //
// ASN1PERMessageBuffer methods          //
//                                      //
//////////////////////////////////////////

EXTPERMETHOD ASN1PERMessageBuffer::ASN1PERMessageBuffer
(Type bufferType, OSBOOL aligned) : ASN1MessageBuffer(bufferType)
{
   if (!OSRTISSTREAM (getCtxtPtr())) {
      int stat = pu_setBuffer (getCtxtPtr(), 0, 0, aligned);
      if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);
   }
}

EXTPERMETHOD ASN1PERMessageBuffer::ASN1PERMessageBuffer
(Type bufferType, OSOCTET* pMsgBuf, size_t msgBufLen, OSBOOL aligned) :
   ASN1MessageBuffer (bufferType)
{
   int stat = pu_setBuffer
      (getCtxtPtr(), pMsgBuf, msgBufLen, aligned);
   if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);
}

EXTPERMETHOD ASN1PERMessageBuffer::ASN1PERMessageBuffer
(Type bufferType, OSOCTET* pMsgBuf, size_t msgBufLen, OSBOOL aligned,
 OSRTContext *pContext) : ASN1MessageBuffer (bufferType, pContext)
{
   int stat = pu_setBuffer
      (getCtxtPtr(), pMsgBuf, msgBufLen, aligned);
   if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);
}

#if !defined(_NO_CPP_STREAM)
EXTPERMETHOD ASN1PERMessageBuffer::ASN1PERMessageBuffer
(OSRTStream& stream, OSBOOL aligned) :
   ASN1MessageBuffer (PEREncode, stream.getContext())
{
   OSCTXT* pctxt = getCtxtPtr();
   pctxt->buffer.aligned = aligned;
   if (0 == pctxt->pASN1Info) {
      int stat = rtCtxtInitASN1Info (pctxt);
      if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);
   }
}
#endif

EXTPERMETHOD int ASN1PERMessageBuffer::setBuffer
(const OSOCTET* pMsgBuf, size_t msgBufLen)
{
   if (getStatus () != 0) return getStatus ();

   OSCTXT* pctxt = getCtxtPtr();

   LCHECKCPP (pctxt);

   int stat = pu_setBuffer (pctxt, (OSOCTET*)pMsgBuf, msgBufLen,
                            pctxt->buffer.aligned);

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}

//////////////////////////////////////////
//                                      //
// ASN1PEREncodeBuffer methods          //
//                                      //
//////////////////////////////////////////

#if !defined(_NO_CPP_STREAM)
EXTPERMETHOD ASN1PEREncodeBuffer::ASN1PEREncodeBuffer
(OSRTOutputStream& ostream, OSBOOL aligned) :
   ASN1PERMessageBuffer (ostream, aligned)
{}
#endif

EXTPERMETHOD ASN1PEREncodeBuffer::~ASN1PEREncodeBuffer()
{
#if !defined(_NO_CPP_STREAM)
   OSCTXT* pCtxt = getCtxtPtr();
   if (OSRTISSTREAM(pCtxt)) {
      rtxStreamFlush (pCtxt);
   }
#endif
}

EXTPERMETHOD OSOCTET* ASN1PEREncodeBuffer::getMsgCopy ()
{
   if (getStatus () != 0) return 0;

   int msgLen;
   OSCTXT*  pCtxt = getCtxtPtr();
   OSOCTET* msg_p = pe_GetMsgPtr (pCtxt, &msgLen);

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

EXTPERMETHOD const OSOCTET* ASN1PEREncodeBuffer::getMsgPtr ()
{
   if (getStatus () != 0) return 0;
   return pe_GetMsgPtr (getCtxtPtr(), 0);
}

// This method initializes the encode buffer for encoding
// Frees the memory used by trace information of previous encode
// This function should only keep memory heap / stream & reset all other
// parameter of context
EXTPERMETHOD int ASN1PEREncodeBuffer::init ()
{
   if (getStatus () != 0) return getStatus ();

   OSCTXT* pCtxt = getCtxtPtr();
#if !defined(_NO_CPP_STREAM)
   if (OSRTISSTREAM(pCtxt)) {
      rtxStreamFlush (pCtxt);
   }
#endif
   pCtxt->buffer.byteIndex = 0;
   pCtxt->buffer.bitOffset = 8;
   rtxSListFree (&ACINFO(pCtxt)->fieldList);

   return 0;
}

//////////////////////////////////////////
//                                      //
// ASN1PERDecodeBuffer methods          //
//                                      //
//////////////////////////////////////////

#if !defined(_NO_CPP_STREAM)
EXTPERMETHOD ASN1PERDecodeBuffer::ASN1PERDecodeBuffer
(OSRTInputStream& istream, OSBOOL aligned) :
   ASN1PERMessageBuffer (istream, aligned)
{}
#endif

EXTPERMETHOD
ASN1PERDecodeBuffer::ASN1PERDecodeBuffer
(const char* filePath, OSBOOL aligned) :
   ASN1PERMessageBuffer (PERDecode, 0, 0, aligned)
{
   if (getStatus () != 0) return;

   int stat = readBinaryFile (filePath);
   if (stat != 0) getContext()->setStatus (stat);
}

EXTPERMETHOD int ASN1PERDecodeBuffer::peekByte (OSOCTET& ub)
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

EXTPERMETHOD int ASN1PERDecodeBuffer::readBinaryFile (const char* filePath)
{
   OSOCTET* pMsgBuf;
   OSCTXT* pctxt = getCtxtPtr();
   size_t msgLen = 0;
   int stat = rtxFileReadBinary (pctxt, filePath, &pMsgBuf, &msgLen);

   if (stat == 0)
      stat = setBuffer (pMsgBuf, msgLen);

   return getContext()->setStatus (stat);
}

EXTPERMETHOD int ASN1PERDecodeBuffer::readBytes
(OSOCTET* buffer, size_t bufsize, size_t nbytes)
{
   return (nbytes <= bufsize) ?
      rtxReadBytes (getCtxtPtr(), buffer, nbytes) : RTERR_BUFOVFLW;
}

