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

#include "rtxersrc/ASN1XEREncodeStream.h"

////////////////////////////////////////
//
// ASN1XEREncodeStream

ASN1XEREncodeStream::ASN1XEREncodeStream (OSRTOutputStreamIF& os,
      OSBOOL canonical) :
   ASN1XEREncodeBuffer (canonical, os.getContext ()),
   mpStream (&os), mbOwnStream (FALSE)
{
   OSRTASSERT (0 != mpStream);
}

ASN1XEREncodeStream::ASN1XEREncodeStream (OSRTOutputStreamIF* pos,
      OSBOOL bOwnStream, OSBOOL canonical) :
   ASN1XEREncodeBuffer (canonical, pos->getContext ()),
   mpStream (pos), mbOwnStream (bOwnStream)
{
   OSRTASSERT (0 != mpStream);
}

ASN1XEREncodeStream::~ASN1XEREncodeStream ()
{
   if (mbOwnStream) {
      mpStream->close ();
      delete mpStream;
   }
}

ASN1XEREncodeStream& ASN1XEREncodeStream::operator << (ASN1CType& val)
{
   if (getStatus () != 0) return *this;

   int stat = val.EncodeTo (*this);
   if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);
   return *this;
}

int ASN1XEREncodeStream::encodeObj (ASN1CType& val)
{
   if (getStatus () != 0) return getStatus ();

   int stat = val.EncodeTo (*this);
   if (stat != 0)
      return LOG_RTERR (getCtxtPtr(), stat);
   return 0;
}

OSBOOL ASN1XEREncodeStream::isA (Type bufferType)
{
   return (bufferType == XEREncode || bufferType == Stream);
}

