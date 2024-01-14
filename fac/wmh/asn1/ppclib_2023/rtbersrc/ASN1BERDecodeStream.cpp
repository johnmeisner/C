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

#include "rtbersrc/ASN1BERDecodeStream.h"

////////////////////////////////////////
//
// ASN1BERDecodeStream

ASN1BERDecodeStream::ASN1BERDecodeStream (OSRTInputStreamIF& is) :
   ASN1MessageBuffer (OSRTMessageBuffer::BEREncode, is.getContext()),
   mpStream (&is), mbOwnStream (FALSE)
{
   OSRTASSERT (0 != mpStream);
   rtCtxtInitASN1Info (getCtxtPtr());
}

ASN1BERDecodeStream::ASN1BERDecodeStream
(OSRTInputStreamIF* pis, OSBOOL bOwnStream) :
   ASN1MessageBuffer (OSRTMessageBuffer::BEREncode, pis->getContext()),
   mpStream (pis), mbOwnStream (bOwnStream)
{
   OSRTASSERT (0 != mpStream);
   rtCtxtInitASN1Info (getCtxtPtr());
}

ASN1BERDecodeStream::~ASN1BERDecodeStream ()
{
   if (mbOwnStream) {
      mpStream->close ();
      delete mpStream;
   }
}

ASN1BERDecodeStream& ASN1BERDecodeStream::operator >> (ASN1CType& val)
{
   if (getStatus () < 0) return *this;

   int stat = val.DecodeFrom (*this);
   if (stat != 0) LOG_RTERR (getCtxtPtr (), stat);
   return *this;
}

OSBOOL ASN1BERDecodeStream::chkend (ASN1CCB& ccb)
{
   return BS_CHKEND (getCtxtPtr (), &ccb);
}

size_t ASN1BERDecodeStream::byteIndex ()
{
   return OSRTSTREAM_BYTEINDEX (getCtxtPtr ());
}

int ASN1BERDecodeStream::decodeTag (ASN1TAG& tag)
{
   if (getStatus () < 0) return getStatus ();

   int stat = berDecStrmTag (getCtxtPtr (), &tag);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeLength (OSINT32& length)
{
   if (getStatus () < 0) return getStatus ();

   int stat = berDecStrmLength (getCtxtPtr (), &length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeTagAndLen (ASN1TAG& tag, OSINT32& len)
{
   if (getStatus () < 0) return getStatus ();

   int stat = berDecStrmTagAndLen (getCtxtPtr(), &tag, &len);
   if (stat != 0) return LOG_RTERR (getCtxtPtr(), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeTagAndLen (ASN1TAG& tag, ASN1BERLength& len)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmTagAndLen2
      (getCtxtPtr(), &tag, &len.mLength, &len.mbIndef);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BERDecodeStream::decodeEoc ()
{
   if (getStatus () < 0) return getStatus ();

   int stat = berDecStrmMatchEOC (getCtxtPtr ());
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeInt (OSINT32& val, ASN1TagType tagging,
                                    int length)
{
   if (getStatus () < 0) return getStatus ();

   int stat = berDecStrmInt (getCtxtPtr (), &val, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeInt8 (OSINT8& val, ASN1TagType tagging,
                                     int length)
{
   if (getStatus () < 0) return getStatus ();

   int stat = berDecStrmInt8 (getCtxtPtr (), &val, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeInt16 (OSINT16& val, ASN1TagType tagging,
                                      int length)
{
   if (getStatus () < 0) return getStatus ();

   int stat = berDecStrmInt16 (getCtxtPtr (), &val, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeInt64 (OSINT64& val, ASN1TagType tagging,
                                      int length)
{
   if (getStatus () < 0) return getStatus ();

   int stat = berDecStrmInt64 (getCtxtPtr (), &val, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeUInt (OSUINT32& val, ASN1TagType tagging,
                                     int length)
{
   if (getStatus () < 0) return getStatus ();

   int stat = berDecStrmUInt (getCtxtPtr (), &val, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeUInt8 (OSUINT8& val, ASN1TagType tagging,
                                      int length)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmUInt8 (getCtxtPtr (), &val, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeUInt16 (OSUINT16& val, ASN1TagType tagging,
                                       int length)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmUInt16 (getCtxtPtr (), &val, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeUInt64 (OSUINT64& val, ASN1TagType tagging,
                                       int length)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmUInt64 (getCtxtPtr (), &val, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeBigInt (const char*& pval, ASN1TagType tagging,
                                       int length)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmBigInt (getCtxtPtr (), &pval, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeEnum (OSINT32& val, ASN1TagType tagging,
                                     int length)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmEnum (getCtxtPtr (), &val, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeBool (OSBOOL& val, ASN1TagType tagging,
                                     int length)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmBool (getCtxtPtr (), &val, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeReal (OSREAL& val, ASN1TagType tagging,
                                     int length)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmReal (getCtxtPtr (), &val, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeBitStr (OSOCTET* pbits, OSUINT32& numbits,
   ASN1TagType tagging, int length)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmBitStr (getCtxtPtr (), pbits, &numbits,
                                tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeBitStr (ASN1DynBitStr& val, ASN1TagType tagging,
                                       int length)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmDynBitStr (getCtxtPtr (), &val.data,
                                   &val.numbits, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeOctStr (OSOCTET* pocts, OSUINT32& numocts,
   ASN1TagType tagging, int length)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmOctStr (getCtxtPtr (), pocts, &numocts, tagging,
                                length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeOctStr (ASN1DynOctStr& val, ASN1TagType tagging,
                                       int length)
{
   if (getStatus() < 0) return getStatus ();

   OSUINT32 numocts;
   int stat = berDecStrmDynOctStr (getCtxtPtr(), &val.data,
                                   &numocts, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr(), stat);

   val.numocts = numocts;

   return 0;
}

int ASN1BERDecodeStream::decodeOctStr
(OSDynOctStr64& val, ASN1TagType tagging, OSSIZE length, OSBOOL indefLen)
{
   if (getStatus() < 0) return getStatus ();

   OSOCTET* pdata = 0;
   int stat = berDecStrmDynOctStr64 (getCtxtPtr(), &pdata,
                                     &val.numocts, tagging, length, indefLen);
   if (stat != 0) return LOG_RTERR (getCtxtPtr(), stat);
   val.data = pdata;

   return 0;
}

int ASN1BERDecodeStream::decodeCharStr (const char*& pval, ASN1TagType tagging,
                                        ASN1TAG tag, int length)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmCharStr (getCtxtPtr (), &pval, tagging, tag,
                                 length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeObjId (ASN1OBJID& val, ASN1TagType tagging,
                                      int length)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmObjId (getCtxtPtr (), &val, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeObjId64 (ASN1OID64& val, ASN1TagType tagging,
                                        int length)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmObjId64 (getCtxtPtr (), &val, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeRelativeOID
(ASN1OBJID& val, ASN1TagType tagging, int length)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmRelativeOID
      (getCtxtPtr(), &val, tagging, length);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BERDecodeStream::decodeNull (ASN1TagType tagging)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmNull (getCtxtPtr (), tagging);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeBMPStr (Asn116BitCharString& val,
                                       ASN1TagType tagging, int length)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmBMPStr (getCtxtPtr (), &val, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeUnivStr (Asn132BitCharString& val,
                                        ASN1TagType tagging, int length)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmUnivStr (getCtxtPtr (), &val, tagging, length);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeObj (ASN1CType& val)
{
   if (getStatus() < 0) return getStatus ();

   int stat = val.DecodeFrom (*this);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);
   return 0;
}

int ASN1BERDecodeStream::decodeOpenType (ASN1OpenType& val)
{
   if (getStatus() < 0) return getStatus ();

   int stat = berDecStrmOpenType
      (getCtxtPtr(), &val.data, &val.numocts);

   return (stat != 0) ? LOG_RTERR (getCtxtPtr(), stat) : 0;
}

int ASN1BERDecodeStream::getTLVLength ()
{
   if (getStatus () < 0) return getStatus ();

   int stat = berDecStrmGetTLVLength (getCtxtPtr());
   if (stat < 0) return LOG_RTERR (getCtxtPtr(), stat);

   return stat;
}

OSBOOL ASN1BERDecodeStream::isA (Type bufferType)
{
   return (bufferType == BERDecode || bufferType == Stream);
}

int ASN1BERDecodeStream::peekTagAndLen (ASN1TAG& tag, int& len)
{
   if (getStatus () < 0) return getStatus ();

   int stat = berDecStrmPeekTagAndLen (getCtxtPtr (), &tag, &len);
   if (stat != 0) return LOG_RTERR (getCtxtPtr (), stat);

   return 0;
}

int ASN1BERDecodeStream::readTLV (OSOCTET* pDestBuf, size_t bufsiz)
{
   if (getStatus () < 0) return getStatus ();

   int stat = berDecStrmReadTLV (getCtxtPtr(), pDestBuf, bufsiz);
   if (stat < 0) return LOG_RTERR (getCtxtPtr(), stat);
   else return stat;
}

