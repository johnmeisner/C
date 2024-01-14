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
 * @file rtxCppAnyAttr.h
 * C++ any element class definition.
 */
#ifndef _RTXCPPANYATTR_H_
#define _RTXCPPANYATTR_H_

#include "rtxsrc/rtxCommon.h"
#include "rtxsrc/OSRTBaseType.h"

typedef OSUTF8NVP OSAnyAttr;

/**
 * Any attribute. This is the base class for generated C++ data type
 * classes for any attribute declarations (xsd:anyAttr).
 */
class EXTRTCLASS OSAnyAttrClass :
public OSAnyAttr, public OSRTBaseType
{
 public:
   /**
    * The default constructor creates an empty attribute.
    */
   OSAnyAttrClass();

   /**
    * This constructor initializes the attribute to contain the
    * given data values.
    *
    * @param pname       - attribute name
    * @param pvalue      - attribute contents
    */
   OSAnyAttrClass (const OSUTF8CHAR* pname, const OSUTF8CHAR* pvalue);

   /**
    * This constructor initializes the attribute to contain the
    * given data values.  This version allows the name/value arguments
    * to be passed as standard C character string literal values.
    *
    * @param pname       - attribute name
    * @param pvalue      - attribute contents
    */
   OSAnyAttrClass (const char* pname, const char* pvalue);

   /**
    * This constructor initializes the attribute to contain the
    * given data values.
    *
    * @param pname        - Attribute name.
    * @param pvalue       - Attribute value.
    */
   OSAnyAttrClass
      (OSUTF8CHAR* pname, OSUTF8CHAR* pvalue);

   /**
    * This copy constructor initializes the attribute to contain the given
    * data values from the C data structure.
    *
    * It performs a deep copy.
    *
    * @param os          - C binary string structure.
    */
   OSAnyAttrClass (OSAnyAttr& os);

   /**
    * This copy constructor initializes the attribute to contain the
    * given data values from the C++ data object.
    *
    * It performs a deep copy.
    *
    * @param os          - C++ binary string object reference.
    */
   OSAnyAttrClass (const OSAnyAttrClass& os);

   /**
    * The destructor frees string memory.
    */
   virtual ~OSAnyAttrClass();

   /**
    * Clone method. Creates a copied instance and returns pointer to
    * OSRTBaseType.
    */
   OSRTBaseType* clone () const { return new OSAnyAttrClass (*this); }

   /**
    * This method copies the given attribute value to the internal
    * string storage variable.  A deep-copy of the given value is done;
    * the class will delete this memory when the object is deleted.
    *
    * @param pname        - Attribute name.
    * @param pvalue       - Attribute value.
    */
   void copyValue (const OSUTF8CHAR* pname, const OSUTF8CHAR* pvalue);

   /**
    * This method sets the attribute value to the given name/value.
    * A deep-copy of the given value is not done; the pointer is
    * stored directly in the class member variable.
    *
    * @param pname        - Attribute name.
    * @param pvalue       - Attribute value.
    */
   void setValue (const OSUTF8CHAR* pname, const OSUTF8CHAR* pvalue);

   /**
    * This method sets the attribute value to the given name/value.
    * A deep-copy of the given value is not done; the pointer is
    * stored directly in the class member variable.
    *
    * @param pname        - Attribute name.
    * @param pvalue       - Attribute value.
    * @param namebytes    - Attribute name length.
    * @param valuebytes   - Attribute value length.
    */
   void setValue (const OSUTF8CHAR* pname, const OSUTF8CHAR* pvalue,
                  size_t namebytes, size_t valuebytes = 0);

   /**
    * Assignment operator.
    */
   OSAnyAttrClass& operator= (const OSAnyAttrClass& original);
} ;

#endif
