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
 * @file rtxCppAnyElement.h
 * C++ any element class definition.
 */
#ifndef _RTXCPPANYELEMENT_H_
#define _RTXCPPANYELEMENT_H_

#include "rtxsrc/rtxCommon.h"
#include "rtxsrc/OSRTBaseType.h"
#include "rtxsrc/rtxPrint.h"

typedef OSUTF8NVP OSAnyElement;

/**
 * Any element. This is the base class for generated C++ data type
 * classes for any element declarations (xsd:any).
 */
class EXTRTCLASS OSAnyElementClass :
public OSAnyElement, public OSRTBaseType
{
 public:
   /**
    * The default constructor creates an empty element.
    */
   OSAnyElementClass();

   /**
    * This constructor initializes the element to contain the
    * given data values.
    *
    * @param pname       - element name
    * @param pvalue      - element contents
    */
   OSAnyElementClass (const OSUTF8CHAR* pname, const OSUTF8CHAR* pvalue);

   /**
    * This constructor initializes the element to contain the
    * given data values.  This version allows the name/value arguments
    * to be passed as standard C character string literal values.
    *
    * @param pname       - element name
    * @param pvalue      - element contents
    */
   OSAnyElementClass (const char* pname, const char* pvalue);

   /**
    * This copy constructor initializes the element to contain the given
    * data values from the C data structure.
    *
    * A deep copy is performed.
    *
    * @param os          - C binary string structure.
    */
   OSAnyElementClass (OSAnyElement& os);

   /**
    * This copy constructor initializes the element to contain the
    * given data values from the C++ data object.
    *
    * A deep copy is performed.
    *
    * @param os          - C++ binary string object reference.
    */
   OSAnyElementClass (const OSAnyElementClass& os);

   /**
    * The destructor frees string memory.
    */
   virtual ~OSAnyElementClass();

   /**
    * This method copies the given element value to the internal
    * string storage variable.  A deep-copy of the given value is done;
    * the class will delete this memory when the object is deleted.
    *
    * @param pname        - Element name.
    * @param pvalue       - Element value.
    */
   void copyValue (const OSUTF8CHAR* pname, const OSUTF8CHAR* pvalue);

   /**
    * This method prints the given element value to standard output.
    *
    * @param pname - Name of generated string variable.
    */
   inline void print (const char* pname) { rtxPrintNVP (pname, this); }

   /**
    * This method copies the given element value to the internal
    * string storage variable.  A deep-copy of the given value is done;
    * the class will delete this memory when the object is deleted.
    *
    * @param pname        - Element name.
    * @param pvalue       - Element value.
    */
   void setValue (const OSUTF8CHAR* pname, const OSUTF8CHAR* pvalue);

   OSRTBaseType* clone () const { return new OSAnyElementClass (*this); }
} ;

#endif
