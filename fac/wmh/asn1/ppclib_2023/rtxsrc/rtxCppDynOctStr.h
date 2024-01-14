/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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
/**
 * @file rtxCppDynOctStr.h
 * C++ dynamic binary string class definition.
 */
#ifndef _RTXCPPDYNOCTSTR_H_
#define _RTXCPPDYNOCTSTR_H_

#include "rtxsrc/rtxCommon.h"
#include "rtxsrc/OSRTBaseType.h"

/**
 * Dynamic binary string. This is the base class for generated C++ data type
 * classes for XSD binary types (hexBinary and base64Binary).
 */
class EXTRTCLASS OSDynOctStrClass : public OSRTBaseType {
 protected:
   OSSIZE numocts;
   OSOCTET* data;

 public:
   /**
    * The default constructor creates an empty binary string.
    */
   OSDynOctStrClass();

   /**
    * This constructor initializes the binary string to contain the
    * given data values.
    *
    * @param numocts_    - Number of bytes in the binary string.
    * @param data_       - The binary string data values.
    */
   OSDynOctStrClass (OSSIZE numocts_, const OSOCTET* data_);

   /**
    * The copy constructor initializes the binary string to contain the
    * given data values from the C data structure.
    *
    * @param os          - C binary string structure.
    */
   OSDynOctStrClass (OSDynOctStr& os);

   /**
    * The copy constructor initializes the binary string to contain the
    * given data values from the C data structure.
    *
    * @param os          - C binary string structure.
    */
   OSDynOctStrClass (OSDynOctStr64& os);

   /**
    * This copy constructor initializes the binary string to contain the
    * given data values from the C++ data object.
    *
    * @param os          - C++ binary string object reference.
    */
   OSDynOctStrClass (const OSDynOctStrClass& os);

   /**
    * The destructor frees string memory.
    */
   virtual ~OSDynOctStrClass();

   /**
    * Clone method. Creates a copied instance and returns pointer to
    * OSRTBaseType.
    */
   OSRTBaseType* clone () const { return new OSDynOctStrClass (*this); }

   /**
    * This method copies the given binary string value to the internal
    * string storage variable.  A deep-copy of the given value is done;
    * the class will delete this memory when the object is deleted.
    *
    * @param numocts_    - Number of bytes in the binary string.
    * @param data_       - The binary string data values.
    */
   void copyValue (OSSIZE numocts_, const OSOCTET* data_);

   /**
    * This method returns a pointer to the binary data field.
    */
   inline const OSOCTET* getValue () const {
      return data;
   }

   /**
    * This method returns the length in octets of the binary data field.
    */
   inline size_t getLength () const {
      return numocts;
   }

   /**
    * This method returns the length in octets of the binary data field.
    */
   inline size_t length () const {
      return numocts;
   }

   /**
    * This method copies the given binary string value to the internal
    * string storage variable.  A deep-copy of the given value is done;
    * the class will delete this memory when the object is deleted.
    *
    * @param numocts_    - Number of bytes in the binary string.
    * @param data_       - The binary string data values.
    */
   void setValue (OSSIZE numocts_, const OSOCTET* data_);

   /**
    * This method converts hex characters into binary form and sets
    * the value.
    *
    * @param hexstr      - Hex char string value.
    * @param nchars      - Number of characters in string.  If zero,
    *                      characters are read up to null-terminator.
    * @return            - Status of operation: zero if success or a
    *                      negative status code on error.
    */
   int setValue (const char* hexstr, size_t nchars = 0);

   /**
    * This method converts base64-encoded characters into binary form
    * and sets the value.
    *
    * @param base64str   - Base64 char string value.
    * @param nchars      - Number of characters in string.  If zero,
    *                      characters are read up to null-terminator.
    * @return            - Status of operation: zero if success or a
    *                      negative status code on error.
    */
   int setValueFromBase64 (const char* base64str, size_t nchars = 0);

   /**
    * Assignment operator.
    */
   OSDynOctStrClass& operator= (const OSDynOctStrClass& original);
} ;

#endif
