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
#include "asn1AvnCppTypes.h"
#include "rtsrc/rt_common.hh"
#include "rtxsrc/rtxFile.h"
#include "rtxsrc/OSRTFileInputStream.h"

#if !defined(_NO_CPP_STREAM)
#include "rtxsrc/OSRTInputStream.h"
#include "rtxsrc/OSRTOutputStream.h"
#include "rtxsrc/rtxStream.h"
#endif

//////////////////////////////////////////
//                                      //
// ASN1AVNEncodeBuffer methods          //
//                                      //
//////////////////////////////////////////

ASN1AVNEncodeBuffer::ASN1AVNEncodeBuffer(OSOCTET* pMsgBuf, size_t msgBufLen)
   : ASN1MessageBuffer(AVNEncode)
{
   OSCTXT* pctxt = getCtxtPtr();
   int stat = rtxInitContextBuffer(pctxt, pMsgBuf, msgBufLen);
   if (stat != 0) LOG_RTERR(pctxt, stat);
}

ASN1AVNEncodeBuffer::ASN1AVNEncodeBuffer(OSOCTET *pMsgBuf, size_t msgBufLen,
   OSRTContext *pContext) : ASN1MessageBuffer(AVNEncode, pContext)
{
   OSCTXT* pctxt = getCtxtPtr();
   int stat = rtxInitContextBuffer(pctxt, pMsgBuf, msgBufLen);
   if (stat != 0) LOG_RTERR(pctxt, stat);
}


#if !defined(_NO_CPP_STREAM)
EXTRTMETHOD ASN1AVNEncodeBuffer::ASN1AVNEncodeBuffer
(OSRTOutputStream& ostream) :
   ASN1MessageBuffer(AVNEncode, ostream.getContext())
{
   OSCTXT* pctxt = getCtxtPtr();
   if (0 == pctxt->pASN1Info) {
      int stat = rtCtxtInitASN1Info(pctxt);
      if (stat != 0) LOG_RTERR(getCtxtPtr(), stat);
   }
}
#endif

EXTRTMETHOD ASN1AVNEncodeBuffer::~ASN1AVNEncodeBuffer()
{
#if !defined(_NO_CPP_STREAM)
   OSCTXT* pCtxt = getCtxtPtr();
   if (OSRTISSTREAM(pCtxt)) {
      rtxStreamFlush(pCtxt);
   }
#endif
}

EXTRTMETHOD int ASN1AVNEncodeBuffer::init()
{
   if (getStatus() != 0) return getStatus();

   OSCTXT* pCtxt = getCtxtPtr();
#if !defined(_NO_CPP_STREAM)
   if (OSRTISSTREAM(pCtxt)) {
      rtxStreamFlush(pCtxt);
   }
#endif
   pCtxt->buffer.byteIndex = 0;
   pCtxt->buffer.bitOffset = 8;
   return 0;
}


EXTRTMETHOD long ASN1AVNEncodeBuffer::write(const char* filename)
{
   FILE* fp;
   long nbytes = rtxFileOpen(&fp, filename, "wb");
   if (0 == nbytes) {
      nbytes = write(fp);
      fclose(fp);
   }
   return nbytes;
}

// Write JSON document out to given file
// returns: number of bytes written or negative status code if error

EXTRTMETHOD long ASN1AVNEncodeBuffer::write(FILE* fp)
{
   return (long)fwrite(getMsgPtr(), 1, getMsgLen(), fp);
}


//////////////////////////////////////////
//                                      //
// ASN1AVNDecodeBuffer methods          //
//                                      //
//////////////////////////////////////////

ASN1AVNDecodeBuffer::ASN1AVNDecodeBuffer() :
   ASN1MessageBuffer(AVNDecode), mpInputStream(0)
{
   OSCTXT* pctxt = getCtxtPtr();
   int stat = rtxInitContextBuffer(pctxt, 0, 0);
   if (stat != 0) LOG_RTERR(pctxt, stat);
}


ASN1AVNDecodeBuffer::ASN1AVNDecodeBuffer(const OSOCTET* pMsgBuf,
                                          size_t msgBufLen) :
   ASN1MessageBuffer(PERDecode), mpInputStream(0)
{
   OSCTXT* pctxt = getCtxtPtr();
   int stat = rtxInitContextBuffer(pctxt, (OSOCTET*)pMsgBuf, msgBufLen);
   if (stat != 0) LOG_RTERR(pctxt, stat);
}


ASN1AVNDecodeBuffer::ASN1AVNDecodeBuffer(const OSOCTET* pMsgBuf,
   size_t msgBufLen, OSRTContext *pContext) :
   ASN1MessageBuffer(AVNDecode, pContext), mpInputStream(0)
{
   OSCTXT* pctxt = getCtxtPtr();
   int stat = rtxInitContextBuffer(pctxt, (OSOCTET*)pMsgBuf, msgBufLen);
   if (stat != 0) LOG_RTERR(pctxt, stat);
}

#if !defined(_NO_CPP_STREAM)
EXTRTMETHOD ASN1AVNDecodeBuffer::ASN1AVNDecodeBuffer(OSRTInputStream& istream):
               ASN1MessageBuffer(AVNDecode, istream.getContext()),
               mpInputStream(0)
{
   OSCTXT* pctxt = getCtxtPtr();
   if (0 == pctxt->pASN1Info) {
      int stat = rtCtxtInitASN1Info(pctxt);
      if (stat != 0) LOG_RTERR(getCtxtPtr(), stat);
   }
}
#endif


EXTRTMETHOD ASN1AVNDecodeBuffer::ASN1AVNDecodeBuffer
(const char* filePath) :
   ASN1MessageBuffer(AVNDecode), mpInputStream(0)
{
   if (0 != filePath) {
      mpInputStream = new OSRTFileInputStream(getContext(), filePath);
   }
}


ASN1AVNDecodeBuffer::~ASN1AVNDecodeBuffer()
{
   // Delete input stream object if we created one.
   if (mpInputStream)
      delete mpInputStream;
}
