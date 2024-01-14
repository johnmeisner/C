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

#include "rtsrc/asn1CppTypes.h"
#include "rtsrc/rt_common.hh"
#include "rtxsrc/rtxCharStr.h"

// ASN1Context methods

EXTRTMETHOD ASN1Context::ASN1Context () : OSRTContext()
{
   /* Add ASN.1 error codes to global table */
   rtErrASN1Init ();
   if (mbInitialized) {
      /* Init ASN.1 info block */
      mStatus = rtCtxtInitASN1Info (&mCtxt);
   }
   mbInitialized = (OSBOOL)(mStatus == 0);
   RTDIAG2 (&mCtxt, "context %x created, ref count = 0\n", &mCtxt);
}

EXTRTMETHOD int ASN1Context::setRunTimeKey (const OSOCTET* key, size_t keylen)
{
   return OSRTContext::setRunTimeKey (key, keylen);
}

// ASN1CType

EXTRTMETHOD ASN1CType::ASN1CType () :
   mpContext (new ASN1Context()), mpMsgBuf (0)
{
}

EXTRTMETHOD ASN1CType::ASN1CType (OSRTContext& ctxt) :
   mpContext (&ctxt), mpMsgBuf (0)
{
}

EXTRTMETHOD ASN1CType::ASN1CType (OSRTMessageBufferIF& msgBuf) :
   mpContext (msgBuf.getContext()), mpMsgBuf(&msgBuf)
{
}

EXTRTMETHOD ASN1CType::ASN1CType (const ASN1CType& orig) :
   mpContext (orig.mpContext), mpMsgBuf (orig.mpMsgBuf)
{
}

EXTRTMETHOD int ASN1CType::Decode (OSBOOL free)
{
   if (mpContext.isNull ()) return RTERR_NOTINIT;

   int stat = getStatus ();
   if (stat != 0) return stat;

   LCHECKCPP (getCtxtPtr());

   stat = DecodeFrom (*mpMsgBuf, free);

   return (stat < 0) ? LOG_RTERR (getCtxtPtr(), stat) : stat;
}

EXTRTMETHOD int ASN1CType::Encode ()
{
   if (mpContext.isNull ()) return RTERR_NOTINIT;

   int stat = getStatus ();
   if (stat != 0) return stat;

   LCHECKCPP (getCtxtPtr());

   stat = EncodeTo (*mpMsgBuf);

   return (stat < 0) ? LOG_RTERR (getCtxtPtr(), stat) : stat;
}

EXTRTMETHOD char* ASN1CType::getErrorText (char* textbuf, OSSIZE bufsize)
{
   char* textbuf2 = rtxErrGetText (getCtxtPtr(), textbuf, &bufsize);
   if (0 != textbuf2) {
      if (0 == textbuf || 0 == bufsize) {
         OSSIZE newBufSize = OSCRTLSTRLEN(textbuf2) + 1;
         textbuf = new char [newBufSize];
         if (0 != textbuf) {
            rtxStrcpy (textbuf, newBufSize, textbuf2);
         }
         rtxMemFreePtr (getCtxtPtr(), textbuf2);
      }
      return textbuf;
   }
   return 0;
}

EXTRTMETHOD int ASN1CType::setMsgBuf
(OSRTMessageBufferIF& msgBuf, OSBOOL initBuf)
{
   if (mpContext.isNull ()) return RTERR_NOTINIT;
   if (getStatus() != 0) return getStatus ();

   mpMsgBuf = &msgBuf;
   OSCTXT* pctxt = mpContext->getPtr();

   if (pctxt->key != 0 && msgBuf.getCtxtPtr()->key == 0) {
      msgBuf.getCtxtPtr()->key = pctxt->key;
      msgBuf.getCtxtPtr()->keylen = pctxt->keylen;
   }

   if (initBuf && getStatus() == 0) {
      int stat = msgBuf.init();
      mpContext->setStatus (stat);
   }

   return getStatus();
}

EXTRTMETHOD int ASN1CType::setRunTimeKey (const OSOCTET* key, size_t keylen)
{
   if (mpContext.isNull ()) return RTERR_NOTINIT;
   int stat = mpContext->setRunTimeKey (key, keylen);
   return mpContext->setStatus (stat);
}

// ASN1MessageBuffer methods

EXTRTMETHOD ASN1MessageBuffer::ASN1MessageBuffer (Type bufferType) :
   OSRTMessageBuffer (bufferType, new ASN1Context)
{
}

EXTRTMETHOD
ASN1MessageBuffer::ASN1MessageBuffer (Type bufferType, OSRTContext* pContext) :
   OSRTMessageBuffer (bufferType, pContext)
{
   rtErrASN1Init ();
}

EXTRTMETHOD int ASN1MessageBuffer::initBuffer (OSRTMEMBUF& membuf)
{
   int len;
   OSOCTET* data = rtxMemBufGetData (&membuf, &len);
   return rtxInitContextBuffer (getCtxtPtr(), data, len);
}

EXTRTMETHOD int ASN1MessageBuffer::initBuffer (OSUNICHAR* unistr)
{
   // Currently only supported for XER decode
   return setStatus (RTERR_NOTSUPP);
}

EXTRTMETHOD int
ASN1MessageBuffer::setRunTimeKey (const OSOCTET* key, size_t keylen)
{
   if (getContext().isNull()) return RTERR_NOTINIT;
   return getContext()->setStatus (getContext()->setRunTimeKey (key, keylen));
}

#if !defined(__SYMBIAN32__) && !defined(_V2X)
ASN1BMPString* ASN1MessageBuffer::CStringToBMPString
(const char* cstring, ASN1BMPString* pBMPString,
 Asn116BitCharSet* pCharSet)
{
   return rtCToBMPString (getCtxtPtr(), cstring, pBMPString, pCharSet);
}
#endif
