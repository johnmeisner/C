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

#include "rtbersrc/ASN1BEREncodeStream.h"

////////////////////////////////////////
//
// ASN1BEREncodeStream

ASN1BEREncodeStream::ASN1BEREncodeStream (OSRTOutputStreamIF& os) :
   ASN1MessageBuffer (OSRTMessageBuffer::BEREncode, os.getContext ()),
   mpStream (&os), mbOwnStream (FALSE)
{
   OSRTASSERT (0 != mpStream);
}

ASN1BEREncodeStream::ASN1BEREncodeStream
(OSRTOutputStreamIF* pos, OSBOOL bOwnStream) :
   ASN1MessageBuffer (OSRTMessageBuffer::BEREncode, pos->getContext ()),
   mpStream (pos), mbOwnStream (bOwnStream)
{
   OSRTASSERT (0 != mpStream);
}

ASN1BEREncodeStream::~ASN1BEREncodeStream ()
{
   if (mbOwnStream) {
      mpStream->close ();
      delete mpStream;
   }
}

ASN1BEREncodeStream& ASN1BEREncodeStream::operator << (ASN1CType& val)
{
   if (getStatus () != 0) return *this;

   int stat = val.EncodeTo (*this);
   if (stat != 0)
      LOG_RTERR (getCtxtPtr(), stat);

   return *this;
}

int ASN1BEREncodeStream::encodeTag (ASN1TAG tag)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmTag (getCtxtPtr (), tag);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeLen (size_t len)
{
   int stat = getStatus();
   if (0 == stat) {
      stat = berEncStrmLength (getCtxtPtr(), (int)len);
   }
   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeIndefLen ()
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmLength (getCtxtPtr (), ASN_K_INDEFLEN);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeTagAndIndefLen (ASN1TAG tag)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmTagAndIndefLen (getCtxtPtr (), tag);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeTagAndLen (ASN1TAG tag, OSINT32 len)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmTagAndLen (getCtxtPtr (), tag, len);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeEoc ()
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmEOC (getCtxtPtr ());

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeInt (OSINT32 val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmInt (getCtxtPtr (), val, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeInt8 (OSINT8 val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmInt8 (getCtxtPtr (), val, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeInt16 (OSINT16 val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmInt16 (getCtxtPtr (), val, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeInt64 (OSINT64 val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmInt64 (getCtxtPtr (), val, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeUInt (OSUINT32 val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmUInt (getCtxtPtr (), val, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeUInt8 (OSUINT8 val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmUInt8 (getCtxtPtr (), val, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeUInt16 (OSUINT16 val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmUInt16 (getCtxtPtr (), val, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeUInt64 (OSUINT64 val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmUInt64 (getCtxtPtr (), val, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeBigInt (const char *pval, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmBigInt (getCtxtPtr(), pval, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeBigIntNchars
(const char *pval, size_t nchars, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmBigIntNchars
      (getCtxtPtr(), pval, nchars, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeEnum (OSINT32 val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmEnum (getCtxtPtr (), val, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeBool (OSBOOL val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmBool (getCtxtPtr (), val, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeReal (OSREAL val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmReal (getCtxtPtr (), val, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeReal10(const char* val, ASN1TagType tagging)
{
   if (getStatus() != 0) return getStatus();

   int stat = berEncStrmReal10(getCtxtPtr(), val, tagging);

   return (stat != 0) ? LOG_RTERR(getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeBitStr (const OSOCTET* pbits, size_t numbits,
   ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmBitStr (getCtxtPtr (), pbits, numbits, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeBitStr
(const ASN1DynBitStr& val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmBitStr
      (getCtxtPtr (), val.data, val.numbits, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeOctStr
(const OSOCTET* pocts, OSSIZE numocts, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmOctStr (getCtxtPtr(), pocts, numocts, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeOctStr
(const ASN1DynOctStr& val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmOctStr
      (getCtxtPtr(), val.data, val.numocts, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeCharStr
(const char* pval, ASN1TagType tagging, ASN1TAG tag)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmCharStr (getCtxtPtr(), pval, tagging, tag);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeObjId
(const ASN1OBJID& val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmObjId (getCtxtPtr(), &val, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeObjId64
(const ASN1OID64& val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmObjId64 (getCtxtPtr(), &val, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeRelativeOID
(const ASN1OBJID& val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmRelativeOID (getCtxtPtr (), &val, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeNull (ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmNull (getCtxtPtr (), tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeBMPStr
(const Asn116BitCharString& val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmBMPStr (getCtxtPtr (), &val, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeUnivStr
(const Asn132BitCharString& val, ASN1TagType tagging)
{
   if (getStatus () != 0) return getStatus ();

   int stat = berEncStrmUnivStr (getCtxtPtr (), &val, tagging);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BEREncodeStream::encodeObj (ASN1CType& val)
{
   if (getStatus () != 0) return getStatus ();

   int stat = val.EncodeTo (*this);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

OSBOOL ASN1BEREncodeStream::isA (Type bufferType)
{
   return (bufferType == BEREncode || bufferType == Stream);
}

