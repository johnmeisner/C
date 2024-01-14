/*
 * Copyright (c) 2020 Objective Systems, Inc.
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

#include "rtsrc/ASN1RTType.h"

ASN1RTElement::ASN1RTElement
(const char* name, const ASN1RTType& typeInfo,  bool optional, bool extended) :
   mName(name), mTypeInfo(typeInfo), mbOptional(optional), mbExtended(extended)
{
}

const std::vector<ASN1RTElement>& ASN1RTType::getElements() const
{
   static std::vector<ASN1RTElement> elements;
   return elements;
}

#define INSTANCE(T) \
const ASN1RTType& T::instance() \
{ \
   static T sInstance; \
   return sInstance; \
}

INSTANCE(ASN1RTBoolean)
INSTANCE(ASN1RTInteger)
INSTANCE(ASN1RTBitString)
INSTANCE(ASN1RTOctetString)
INSTANCE(ASN1RTNull)
INSTANCE(ASN1RTObjectId)
INSTANCE(ASN1RTIA5String)
INSTANCE(ASN1RTVisibleString)
INSTANCE(ASN1RTUTF8String)
INSTANCE(ASN1RTExternal)
INSTANCE(ASN1RTReal)
INSTANCE(ASN1RTEnumerated)
INSTANCE(ASN1RTSequence)
INSTANCE(ASN1RTSet)
INSTANCE(ASN1RTSeqOf)
INSTANCE(ASN1RTSetOf)
INSTANCE(ASN1RTChoice)
INSTANCE(ASN1RTOpenType)
