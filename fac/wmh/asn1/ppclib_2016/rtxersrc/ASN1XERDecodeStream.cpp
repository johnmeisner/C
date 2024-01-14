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

#include "rtxersrc/ASN1XERDecodeStream.h"

////////////////////////////////////////
//
// ASN1XERDecodeStream

ASN1XERDecodeStream::ASN1XERDecodeStream (OSRTInputStreamIF& is,
      OSBOOL openType) :
   ASN1XERDecodeBuffer (is, openType),
   mpStream (&is), mbOwnStream (FALSE)
{
   OSRTASSERT (0 != mpStream);
}

ASN1XERDecodeStream::ASN1XERDecodeStream (OSRTInputStreamIF* pis,
   OSBOOL bOwnStream, OSBOOL openType) :
   ASN1XERDecodeBuffer (*pis, openType),
   mpStream (pis), mbOwnStream (bOwnStream)
{
   OSRTASSERT (0 != mpStream);
}

ASN1XERDecodeStream::~ASN1XERDecodeStream ()
{
   if (mbOwnStream) {
      mpStream->close ();
      delete mpStream;
   }
}

ASN1XERDecodeStream& ASN1XERDecodeStream::operator >> (ASN1CType& val)
{
   if (getStatus () != 0) return *this;

   int stat = val.DecodeFrom (*this);
   if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);
   return *this;
}

int ASN1XERDecodeStream::decodeObj (ASN1CType& val)
{
   if (getStatus () != 0) return getStatus ();

   int stat = val.DecodeFrom (*this);
   if (stat != 0) return LOG_RTERR (getCtxtPtr(), stat);
   return 0;
}

OSBOOL ASN1XERDecodeStream::isA (Type bufferType)
{
   return (bufferType == XERDecode || bufferType == Stream);
}

