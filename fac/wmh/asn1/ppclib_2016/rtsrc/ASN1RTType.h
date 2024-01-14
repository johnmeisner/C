/*
 * Copyright (c) 2018 Objective Systems, Inc.
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

#ifndef ASN1RTTYPE_H
#define ASN1RTTYPE_H

#include <vector>

class ASN1RTType;

class ASN1RTElement {
 protected:
   const char* mName;
   const ASN1RTType& mTypeInfo;
   bool mbOptional;
   bool mbExtended;

 public:
   ASN1RTElement
      (const char* name, const ASN1RTType& typeInfo,
       bool optional=false, bool extended=false);

   const ASN1RTType& getTypeInfo() const { return mTypeInfo; }
   inline bool isOptional() const { return mbOptional; }
} ;

class ASN1RTType {
 public:
   enum TypeCode {
      EOC, Boolean, Integer, BitString, OctetString, Null, ObjectId,
      IA5String, VisibleString, UTF8String, External, Real, Enumerated,
      EmbeddedPDV, RelativeOID, Sequence, Set, SeqOf, SetOf, Choice,
      OpenType, TypeRef
   } ;
   virtual const char* getTypeAssnName() const { return 0; }
   virtual const char* getTypeName() const { return getTypeAssnName(); }
   virtual TypeCode getTypeCode() const = 0;
   virtual const std::vector<ASN1RTElement>& getElements() const;
} ;

class ASN1RTBoolean : public ASN1RTType {
 public:
   virtual TypeCode getTypeCode() const { return Boolean; }
   virtual const char* getTypeName() const { return "BOOLEAN"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTInteger : public ASN1RTType {
 public:
   virtual TypeCode getTypeCode() const { return Integer; }
   virtual const char* getTypeName() const { return "INTEGER"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTBitString : public ASN1RTType {
 public:
   virtual TypeCode getTypeCode() const { return BitString; }
   virtual const char* getTypeName() const { return "BIT STRING"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTOctetString : public ASN1RTType {
 public:
   virtual TypeCode getTypeCode() const { return OctetString; }
   virtual const char* getTypeName() const { return "OCTET STRING"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTNull : public ASN1RTType {
 public:
   virtual TypeCode getTypeCode() const { return Null; }
   virtual const char* getTypeName() const { return "NULL"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTObjectId : public ASN1RTType {
 public:
   virtual TypeCode getTypeCode() const { return ObjectId; }
   virtual const char* getTypeName() const { return "OBJECT IDENTIFIER"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTIA5String : public ASN1RTType {
 public:
   virtual TypeCode getTypeCode() const { return IA5String; }
   virtual const char* getTypeName() const { return "IA5String"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTVisibleString : public ASN1RTType {
 public:
   virtual TypeCode getTypeCode() const { return VisibleString; }
   virtual const char* getTypeName() const { return "VisibleString"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTUTF8String : public ASN1RTType {
 public:
   virtual TypeCode getTypeCode() const { return UTF8String; }
   virtual const char* getTypeName() const { return "UTF8String"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTExternal : public ASN1RTType {
 public:
   virtual TypeCode getTypeCode() const { return External; }
   virtual const char* getTypeName() const { return "EXTERNAL"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTReal : public ASN1RTType {
 public:
   virtual TypeCode getTypeCode() const { return Real; }
   virtual const char* getTypeName() const { return "REAL"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTEnumerated : public ASN1RTType {
 public:
   virtual TypeCode getTypeCode() const { return Enumerated; }
   virtual const char* getTypeName() const { return "ENUMERATED"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTRelativeOID : public ASN1RTType {
 public:
   virtual TypeCode getTypeCode() const { return RelativeOID; }
   virtual const char* getTypeName() const { return "RELATIVE-OID"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTCons : public ASN1RTType {
 public:
} ;

class ASN1RTSeqOrSet : public ASN1RTCons {
} ;

class ASN1RTSequence : public ASN1RTSeqOrSet {
 public:
   virtual TypeCode getTypeCode() const { return Sequence; }
   virtual const char* getTypeName() const { return "SEQUENCE"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTSet : public ASN1RTSeqOrSet {
 public:
   virtual TypeCode getTypeCode() const { return Set; }
   virtual const char* getTypeName() const { return "SET"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTSeqOf : public ASN1RTCons {
 public:
   virtual TypeCode getTypeCode() const { return SeqOf; }
   virtual const char* getTypeName() const { return "SEQUENCE OF"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTSetOf : public ASN1RTCons {
 public:
   virtual TypeCode getTypeCode() const { return SetOf; }
   virtual const char* getTypeName() const { return "SET OF"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTChoice : public ASN1RTCons {
 public:
   virtual TypeCode getTypeCode() const { return Choice; }
   virtual const char* getTypeName() const { return "CHOICE"; }
   static const ASN1RTType& instance();
} ;

class ASN1RTOpenType : public ASN1RTType {
 public:
   virtual TypeCode getTypeCode() const { return OpenType; }
   virtual const char* getTypeName() const { return "ANY"; }
   static const ASN1RTType& instance();
} ;

#endif
