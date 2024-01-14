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
#include "rtbersrc/asn1BerCppTypes.h"
#include "rtsrc/asn1intl.h"
#include "rtsrc/rt3GPPTS32297.h"
#include "rtxsrc/rtxContext.hh"
#include "rtxsrc/rtxFile.h"

//////////////////////////////////////////
//                                      //
// ASN1BEREncodeBuffer methods          //
//                                      //
//////////////////////////////////////////

ASN1BEREncodeBuffer::ASN1BEREncodeBuffer () : ASN1BERMessageBuffer(BEREncode)
{
   int stat;
   if ((stat = xe_setp (getCtxtPtr(), 0, 0)) != 0) {
      LOG_RTERR (getCtxtPtr(), stat);
      setStatus (stat);
   }
}

ASN1BEREncodeBuffer::ASN1BEREncodeBuffer (OSOCTET* pMsgBuf, size_t msgBufLen) :
   ASN1BERMessageBuffer (BEREncode)
{
   int stat;
   if ((stat = xe_setp
        (getCtxtPtr(), pMsgBuf, (int)msgBufLen)) != 0)
   {
      LOG_RTERR (getCtxtPtr(), stat);
      setStatus (stat);
   }
}

ASN1BEREncodeBuffer::ASN1BEREncodeBuffer
(OSOCTET* pMsgBuf, size_t msgBufLen, OSRTContext *pContext) :
   ASN1BERMessageBuffer (BEREncode, pContext)
{
   int stat;
   if ((stat = xe_setp
        (getCtxtPtr(), pMsgBuf, (int)msgBufLen)) != 0)
   {
      LOG_RTERR (getCtxtPtr(), stat);
      setStatus (stat);
   }
}

int ASN1BEREncodeBuffer::encodeBool (OSBOOL val, ASN1TagType tagging)
{
   OSCTXT* pctxt = getCtxtPtr();
   int len = xe_boolean (pctxt, &val, tagging);
   return (len < 0) ? LOG_RTERR(pctxt, len) : 0;
}

int ASN1BEREncodeBuffer::encodeBigInt (const char *pval, ASN1TagType tagging)
{
   OSCTXT* pctxt = getCtxtPtr();
   int len = xe_bigint (pctxt, pval, tagging);
   return (len < 0) ? LOG_RTERR(pctxt, len) : 0;
}

int ASN1BEREncodeBuffer::encodeBigIntNchars
(const char *pval, size_t nchars, ASN1TagType tagging)
{
   OSCTXT* pctxt = getCtxtPtr();
   int len = xe_bigintn (pctxt, pval, nchars, tagging);
   return (len < 0) ? LOG_RTERR(pctxt, len) : 0;
}

int ASN1BEREncodeBuffer::encodeObjId (ASN1OBJID* pval, ASN1TagType tagging)
{
   OSCTXT* pctxt = getCtxtPtr();
   int len = xe_objid (pctxt, pval, tagging);
   return (len < 0) ? LOG_RTERR(pctxt, len) : 0;
}

// This method returns a copy of the encoded message in a dynamic
// memory buffer.  It is the programmer's responsibility to free
// this memory when done with the message..

OSOCTET* ASN1BEREncodeBuffer::getMsgCopy ()
{
   OSCTXT* pCtxt = getCtxtPtr();
   OSOCTET* msg_p = xe_getp (pCtxt);
   size_t msgLen = pCtxt->buffer.size - pCtxt->buffer.byteIndex;

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

const OSOCTET* ASN1BEREncodeBuffer::getMsgPtr ()
{
   return xe_getp (getCtxtPtr());
}

// Ths method return the length of the encoded message

size_t ASN1BEREncodeBuffer::getMsgLen()
{
   OSCTXT* pctxt = getCtxtPtr();
   return (pctxt->buffer.size > pctxt->buffer.byteIndex) ?
      pctxt->buffer.size - pctxt->buffer.byteIndex : 0;
}

// This method moves the encode buffer cursor to the end of the
// buffer to allow a new message to be encoded..

int ASN1BEREncodeBuffer::init ()
{
   OSCTXT* pCtxt = getCtxtPtr();

   LCHECKCPP (pCtxt);

   pCtxt->buffer.byteIndex = pCtxt->buffer.size;
   return 0;
}

int ASN1BEREncodeBuffer::setBuffer (OSOCTET* pMsgBuf, size_t msgBufLen)
{
   OSCTXT* pctxt = getCtxtPtr();

   /* Free existing buffer */
   xe_free (pctxt);

   int stat = xe_setp (pctxt, pMsgBuf, msgBufLen);
   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}

ASN1BEREncodeBuffer& ASN1BEREncodeBuffer::operator << (ASN1CType& val)
{
   OSCTXT* pCtxt = getCtxtPtr();
   int stat;

   stat = val.EncodeTo (*this);
   if (stat != 0) LOG_RTERR (pCtxt, stat);

   return *this;
}

//////////////////////////////////////////
//                                      //
// ASN1BERDecodeBuffer methods          //
//                                      //
//////////////////////////////////////////

ASN1BERDecodeBuffer::ASN1BERDecodeBuffer () :
   ASN1BERMessageBuffer(BERDecode), mpMsgBuf(0), mMsgBufLen(0),
   mBufSetFlag (FALSE)
{
}

ASN1BERDecodeBuffer::ASN1BERDecodeBuffer
(const OSOCTET* pMsgBuf, size_t msgBufLen) :
   ASN1BERMessageBuffer(BERDecode), mpMsgBuf(pMsgBuf), mMsgBufLen(msgBufLen),
   mBufSetFlag (FALSE)
{
}

ASN1BERDecodeBuffer::ASN1BERDecodeBuffer
(const OSOCTET* pMsgBuf, size_t msgBufLen, OSRTContext* pContext) :
   ASN1BERMessageBuffer(BERDecode, pContext), mpMsgBuf(pMsgBuf),
   mMsgBufLen(msgBufLen), mBufSetFlag (FALSE)
{
}

int ASN1BERDecodeBuffer::init () {
   int stat = setBuffer (mpMsgBuf, mMsgBufLen);
   mBufSetFlag = FALSE; // necessary to allow reset buffer by call to
                        // setBuffer with the same address and length.
   return stat;
}

OSOCTET* ASN1BERDecodeBuffer::findElement (ASN1TAG tag,
                                           OSINT32& elemLen,
                                           OSBOOL firstFlag)
{
   OSOCTET lflgs = XM_SEEK;
   OSCTXT* pCtxt = getCtxtPtr();
   int     stat;

   if (getStatus () != 0) return 0;

   // If first time called, set the decode buffer pointer; otherwise,
   // modify the match flags to skip the current element

   if (firstFlag) {
      mBufSetFlag = TRUE;
      stat = xd_setp (pCtxt, mpMsgBuf, (int)mMsgBufLen, 0, 0);
      if (stat != 0) return ((OSOCTET*) 0);
   }
   else
      lflgs |= XM_SKIP;

   stat = xd_match (pCtxt, tag, &elemLen, lflgs);
   getContext()->setStatus (stat);

   return (stat == 0) ? OSRTBUFPTR(pCtxt) : ((OSOCTET*) 0);
}

int ASN1BERDecodeBuffer::readBinaryFile (const char* filePath)
{
   if (getStatus () != 0) return getStatus ();

   OSOCTET* pMsgBuf;
   OSCTXT* pctxt = getCtxtPtr();
   size_t msgLen = 0;
   int stat = rtxFileReadBinary (pctxt, filePath, &pMsgBuf, &msgLen);

   if (stat == 0) {
      stat = setBuffer (pMsgBuf, msgLen);
      pctxt->buffer.dynamic = TRUE; /* we allocated this memory dynamically */
   }

   LCHECKCPP (pctxt);

   return getContext()->setStatus (stat);
}

int ASN1BERDecodeBuffer::setBuffer (const OSOCTET* pMsgBuf, size_t msgBufLen)
{
   if (getStatus () != 0) return getStatus ();

   OSCTXT* pctxt = getCtxtPtr();

   if (mpMsgBuf == pMsgBuf && mMsgBufLen == msgBufLen && mBufSetFlag)
      return 0;

   LCHECKCPP (pctxt);

   mpMsgBuf = pMsgBuf;
   mMsgBufLen = msgBufLen;

   mBufSetFlag = TRUE;
   return xd_setp (pctxt, mpMsgBuf, (int)mMsgBufLen, 0, 0);
}

int ASN1BERDecodeBuffer::setBufPtr (const OSOCTET* pMsgBuf, size_t msgBufLen)
{
   if (getStatus () != 0) return getStatus ();

   if (mpMsgBuf == pMsgBuf && mMsgBufLen == msgBufLen && mBufSetFlag)
      return 0;

   mpMsgBuf = pMsgBuf;
   mMsgBufLen = msgBufLen;

   mBufSetFlag = TRUE;
   return 0;
}

ASN1BERDecodeBuffer& ASN1BERDecodeBuffer::operator >> (ASN1CType& val)
{
   getContext()->setStatus (0);

   int stat = val.DecodeFrom (*this);
   if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);

   return *this;
}

ASN1BERLength::ASN1BERLength (int length)
{
   if (length == ASN_K_INDEFLEN) {
      mLength = 0;
      mbIndef = TRUE;
   }
   else {
      mLength = (OSSIZE)length;
      mbIndef = FALSE;
   }
}

int ASN1BERLength::getIntLength() const
{
   return (isIndef()) ? ASN_K_INDEFLEN : (int)getLength();
}

int ASN1BERLength::decodeLen (OSCTXT* pctxt)
{
   return xd_len64 (pctxt, &mLength, &mbIndef);
}

int ASN1BERLength::decodeTagLen (OSCTXT* pctxt, ASN1TAG* ptag, OSOCTET flags)
{
   return xd_tag_len_64 (pctxt, ptag, &mLength, &mbIndef, flags);
}

int ASN1BERLength::getElemCount (OSCTXT* pctxt, OSSIZE& count)
{
   return xd_count64 (pctxt, mLength, mbIndef, &count);
}

int ASN1BERLength::matchTag (OSCTXT* pctxt, OSOCTET tag)
{
   return xd_match1_64 (pctxt, tag, &mLength, &mbIndef);
}

int ASN1BERLength::matchTag (OSCTXT* pctxt, ASN1TAG tag, OSOCTET flags)
{
   return xd_match64 (pctxt, tag, &mLength, &mbIndef, flags);
}

void ASN1BERLength::setCCBLength (ASN1CCB& ccb) const
{
   ccb.len = isIndef() ? ASN_K_INDEFLEN : (long)getLength();
}

ASN1BERTS32297CDRFileHeader::ASN1BERTS32297CDRFileHeader(ASN1MessageBuffer* pMessageBuffer) :
   mpBufferOrStream(pMessageBuffer)
{
   rtInitTS32297FileHdr(&mFileHeaderData);

   // If pMessageBuffer is a stream, isA() returns true for both Stream and BERDecode.
   // So we'll check to see if it's a stream first, and then check to see if it's
   // anything other than BERDecode.
   if (pMessageBuffer->isA(OSRTMessageBufferIF::Stream)) return;
   if (!pMessageBuffer->isA(OSRTMessageBufferIF::BERDecode)) return;
   ASN1BERDecodeBuffer* pDecodeBuffer = static_cast<ASN1BERDecodeBuffer*>(pMessageBuffer);
   OSCTXT* pctxt = pMessageBuffer->getCtxtPtr();
   const OSOCTET* pMsgBuf = pDecodeBuffer->getMsgPtr();
   mMsgBufLen = pDecodeBuffer->mMsgBufLen;
   int stat = pDecodeBuffer->setBufPtr (pMsgBuf, mMsgBufLen);
   if (0 != stat)
   {
      LOG_RTERR(pctxt, stat);
      pDecodeBuffer->setStatus(stat);
   }
   stat = rtxCtxtSetBufPtr(pctxt, (OSOCTET*)pMsgBuf, mMsgBufLen);
   if (0 != stat)
   {
      LOG_RTERR(pctxt, stat);
      pDecodeBuffer->setStatus(stat);
   }
}

int ASN1BERTS32297CDRFileHeader::decode()
{
   OSCTXT* pctxt = 0;
   pctxt = mpBufferOrStream->getCtxtPtr();
   if (0 == pctxt) return RTERR_NOTINIT;
   int stat = rtDecTS32297FileHdr(pctxt, &mFileHeaderData);
   if (stat > 0) mOffset = mFileHeaderData.headerLength;
   return stat;
}

void ASN1BERTS32297CDRFileHeader::print()
{
   OSCTXT* pctxt = 0;
   pctxt = mpBufferOrStream->getCtxtPtr();
   if (0 == pctxt) return;
   rtPrtToStrmTS32297FileHdr(pctxt, &mFileHeaderData);
}

void ASN1BERTS32297CDRFileHeader::printErrorInfo()
{
   OSCTXT* pctxt = 0;
   pctxt = mpBufferOrStream->getCtxtPtr();
   if (0 == pctxt) return;
   rtxErrPrint(pctxt);
}

void ASN1BERTS32297CDRHeader::init()
{
   rtInitTS32297Hdr(&mHeaderData);
}

int ASN1BERTS32297CDRHeader::decode()
{
   OSCTXT* pctxt = 0;
   pctxt = mFileHeader.mpBufferOrStream->getCtxtPtr();
   if (0 == pctxt) return RTERR_NOTINIT;
   int stat = rtDecTS32297Hdr(pctxt, &mHeaderData);
   if (stat > 0) mFileHeader.mOffset += stat;
   return stat;
}

void ASN1BERTS32297CDRHeader::print()
{
   OSCTXT* pctxt = 0;
   pctxt = mFileHeader.mpBufferOrStream->getCtxtPtr();
   if (0 == pctxt) return;
   rtPrtToStrmTS32297Hdr(pctxt, &mHeaderData);
}

void ASN1BERTS32297CDRHeader::printErrorInfo()
{
   OSCTXT* pctxt = 0;
   pctxt = mFileHeader.mpBufferOrStream->getCtxtPtr();
   if (0 == pctxt) return;
   rtxErrPrint(pctxt);
}

int ASN1BERTS32297CDRHeader::nextCdrHeader()
{
   // If mFileHeader.mpBufferOrStream is a stream, isA() returns true for both Stream
   // and BERDecode.  So we'll check to see if it's a stream first, and then check to
   // see if it's anything other than BERDecode.
   if (mFileHeader.mpBufferOrStream->isA(OSRTMessageBufferIF::Stream)) return 0;
   if (!mFileHeader.mpBufferOrStream->isA(OSRTMessageBufferIF::BERDecode)) return 0;
   ASN1BERDecodeBuffer* pDecodeBuffer = static_cast<ASN1BERDecodeBuffer*>(mFileHeader.mpBufferOrStream);
   if (0 == pDecodeBuffer) return 0;
   OSCTXT* pctxt = pDecodeBuffer->getCtxtPtr();
   OSSIZE offset = mFileHeader.mOffset + mHeaderData.cdrLength;
   const OSOCTET* pMsgBuf = pDecodeBuffer->getMsgPtr();
   size_t msgBufLen = mFileHeader.mMsgBufLen;
   int stat = pDecodeBuffer->setBufPtr(pMsgBuf + offset, msgBufLen - offset);
   if (stat != 0) return stat;
   stat = rtxCtxtSetBufPtr(pctxt, (OSOCTET*)pMsgBuf + offset, msgBufLen - offset);
   if (stat != 0) return stat;
   mFileHeader.mOffset = offset;
   return 0;
}
