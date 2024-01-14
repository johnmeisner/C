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
// ASN1PERMessageBuffer methods         //
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

void ASN1PERMessageBuffer::newBitField (const char* nameSuffix)
{
   rtxDiagNewBitField (getCtxtPtr()->pBitFldList, nameSuffix);
}

void ASN1PERMessageBuffer::setBitFieldCount()
{
   rtxDiagSetBitFldCount (getCtxtPtr()->pBitFldList);
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

EXTPERMETHOD int ASN1PEREncodeBuffer::encodeLength
(size_t value, Asn1SizeCnst* pSizeCnst)
{
   OSCTXT* pctxt = getCtxtPtr();
   int stat = pu_addSizeConstraint (pctxt, pSizeCnst);
   if (0 == stat) {
      stat = pe_Length(pctxt, value);
   }
   return (stat < 0) ? LOG_RTERR (pctxt, stat) : stat;
}

EXTPERMETHOD int ASN1PEREncodeBuffer::encodeBitOrOctStr
(size_t numItems, const OSOCTET* pdata, Asn1SizeCnst* pSizeCnst,
 OSBOOL bitStr, ASN1PEREncInfo* pPEREncInfo)
{
   OSCTXT* pctxt = getCtxtPtr();
   OSSIZE  segSizeBits;
   int enclen, octidx = 0, stat;

   for (int i = 0; ; i++) {
      if ((enclen = encodeLength(numItems, pSizeCnst)) < 0) {
         return LOG_RTERR (pctxt, enclen);
      }
      // Note: encoded message info is only returned for first segment
      // if fragmented..
      if (0 == i && pPEREncInfo) {
         pPEREncInfo->mLenNumBits = rtxCtxtGetBitOffset(pctxt);
      }
      if (0 == numItems) break;

      if (pctxt->buffer.aligned &&
          pu_BitAndOctetStringAlignmentTest(pSizeCnst, numItems, bitStr)) {
         stat = pe_byte_align (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }

      if (0 == i && pPEREncInfo) {
         pPEREncInfo->mDataBitOffset = rtxCtxtGetBitOffset(pctxt);
      }

      if (bitStr) {
         segSizeBits = (OSSIZE)enclen;
      }
      else {
         segSizeBits = (OSSIZE)enclen * 8;
         if (segSizeBits < (OSSIZE)enclen) {
            return LOG_RTERR(pctxt, RTERR_TOOBIG);
         }
      }
      stat = rtxEncBitsFromByteArray(pctxt, &pdata[octidx], segSizeBits);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      if (0 == i && pPEREncInfo) {
         pPEREncInfo->mDataNumBits =
            rtxCtxtGetBitOffset(pctxt) - pPEREncInfo->mDataBitOffset;
      }
      if ((size_t)enclen < numItems) {
         numItems -= enclen;
         if (bitStr)
            octidx += (enclen/8);
         else
            octidx += enclen;
      }
      else break;
   }

   /* if numItems % 16384 == 0, we must encode a byte of zeros for length */
   if (numItems % 16384 == 0 && numItems > 0) {
      pe_Length (pctxt, 0);
   }

   return 0;
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
   if (getStatus() != 0) return getStatus ();

   OSCTXT* pCtxt = getCtxtPtr();
#if !defined(_NO_CPP_STREAM)
   if (OSRTISSTREAM(pCtxt)) {
      rtxStreamFlush (pCtxt);
   }
#endif
   return pe_resetBuffer (pCtxt);
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

EXTPERMETHOD int ASN1PERDecodeBuffer::decodeBits (OSSIZE nbits, OSUINT32& value)
{
   return rtxDecBits (getCtxtPtr(), &value, nbits);
}

EXTPERMETHOD int ASN1PERDecodeBuffer::decodeBits
(OSSIZE nbits, OSOCTET* buffer, OSSIZE bufsize)
{
   return rtxDecBitsToByteArray (getCtxtPtr(),buffer, bufsize,  nbits);
}

EXTPERMETHOD int ASN1PERDecodeBuffer::decodeBytes
(OSSIZE nbytes, OSOCTET* buffer, OSSIZE bufsize)
{
   return rtxReadBytesSafe (getCtxtPtr(), buffer, bufsize, nbytes);
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

