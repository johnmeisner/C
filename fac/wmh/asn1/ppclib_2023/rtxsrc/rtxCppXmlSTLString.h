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
 * @file rtxCppXmlSTLString.h
 * C++ XML STL string class definition.
 */
#ifndef _RTXCPPXMLSTLSTRING_H_
#define _RTXCPPXMLSTLSTRING_H_

#ifdef HAS_STL
#ifdef _MSC_VER
// Set warning level to 3 to suppress warnings in STL string header file..
#pragma warning( push, 3 )
#endif
#include <string>
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "rtxsrc/OSRTBaseType.h"
#include "rtxsrc/OSRTMemBuf.h"
#include "rtxsrc/rtxPrint.h"
#include "rtxsrc/rtxUTF8.h"
#include "rtxsrc/rtxXmlStr.h"

class OSRTMessageBufferIF;
struct OSXMLNamespace;

/**
 * XML STL string. This is the base class for generated C++ data type
 * classes for XSD string types (string, token, NMTOKEN, etc.)
 * when -usestl is used in command line.
 */
class EXTRTCLASS OSXMLSTLStringClass : public OSRTBaseType {
 protected:
   std::string value;
   OSBOOL cdata;

 public:
   /**
    * The default constructor creates an empty string.
    */
   OSXMLSTLStringClass();

   /**
    * This constructor initializes the string to contain the value.
    * A deep-copy of the given value is done.
    *
    * @param strval       - String value
    * @param cdata_       - Should string be encoded as a CDATA section?
    */
   OSXMLSTLStringClass (const OSUTF8CHAR* strval, OSBOOL cdata_ = FALSE);

   /**
    * This constructor initializes the string to contain the value.
    * It copies up to the given number of bytes from the source string.
    * A deep-copy of the given value is done.
    *
    * @param strval       - String value
    * @param nbytes       - Number of bytes to copy from source string
    * @param cdata_       - Should string be encoded as a CDATA section?
    */
   OSXMLSTLStringClass (const OSUTF8CHAR* strval, size_t nbytes,
                        OSBOOL cdata_ = FALSE);

   /**
    * This constructor initializes the string to contain the value.
    * A deep-copy of the given value is done.
    *
    * @param strval       - String value
    * @param cdata_       - Should string be encoded as a CDATA section?
    */
   OSXMLSTLStringClass (const char* strval, OSBOOL cdata_ = FALSE);

   /**
    * Copy constructor.
    *
    * @param str          - C XML string structure.
    */
   OSXMLSTLStringClass (const OSXMLSTRING& str);

   /**
    * Copy constructor.
    *
    * @param str          - C++ XML string class.
    */
   OSXMLSTLStringClass (const OSXMLSTLStringClass& str);

   /**
    * This method copies the given string value to the end of internal
    * string storage variable.  A deep-copy of the given value is done;
    * the class will delete this memory when the object is deleted.
    *
    * @param utf8str - C null-terminated string.
    *
    * @param nbytes  - length of utf8str in bytes.
    */
   void appendValue (const OSUTF8CHAR* utf8str, size_t nbytes = 0);

   /**
    * Clone method. Creates a copied instance and returns pointer to
    * OSRTBaseType.
    */
   OSRTBaseType* clone () const { return new OSXMLSTLStringClass (*this); }

   /**
    * This method does a standard string comparison operation (strcmp)
    * with the UTF-8 null terminated string.
    *
    * @return Zero (0) if the compared characters sequences are equal, -1 if
    *   the internal string value is less than the argument value, and +1
    *   if the internal string value is greater than the argument value.
    */
   inline int compare (const OSUTF8CHAR* value2) const {
      return value.compare ((const char*)value2);
   }

   /**
    * This method copies the given string value to the internal
    * string storage variable.  A deep-copy of the given value is done;
    * the class will delete this memory when the object is deleted.
    *
    * @param utf8str - C null-terminated string.
    *
    * @param nbytes  - length of utf8str in bytes.
    */
   void copyValue (const OSUTF8CHAR* utf8str, size_t nbytes = 0);

   /**
    * This method copies the given string value to the internal
    * string storage variable.  A deep-copy of the given value is done;
    * the class will delete this memory when the object is deleted.
    *
    * @param cstring - C null-terminated string.
    *
    * @param nbytes  - length of cstring in bytes.
    */
   inline void copyValue (const char* cstring, size_t nbytes = 0) {
      copyValue ((const OSUTF8CHAR*) cstring, nbytes);
   }

   /**
    * This method returns the pointer to C null terminated string.
    */
   inline const char* c_str () const { return value.c_str(); }

   /**
    * This method compares this string with the UTF-8 null terminated
    * string for equality.
    */
   inline OSBOOL equals (const OSUTF8CHAR* value2) const {
      return value.compare ((const char*)value2) == 0;
   }

   /**
    * This method returns a reference to the underlying std::string object
    * to allow direct manipulation of the value.
    */
   inline std::string& getString () { return value; }

   /**
    * This method returns a pointer to the UTF-8 null terminated string.
    */
   inline const OSUTF8CHAR* getValue () const {
      return (const OSUTF8CHAR*) value.c_str();
   }

   /**
    * This method returns the value of the cdata member variable.
    * This indicates if this string should be encoded as a CDATA
    * section in an XML document.
    *
    * @return   - True if string is to be encoded as CDATA section
    */
   inline OSBOOL isCDATA () const { return cdata; }

   /**
    * This method sets the value of the cdata member variable.
    * This indicates if this string should be encoded as a CDATA
    * section in an XML document.
    *
    * @param bvalue     - Boolean value.
    */
   inline void setCDATA (OSBOOL bvalue) { cdata = bvalue; }

   /**
    * This method prints the string value to standard output.
    *
    * @param name - Name of generated string variable.
    */
   inline void print (const char* name) {
      rtxPrintUTF8CharStr (name, (const OSUTF8CHAR*) value.c_str());
   }

   /**
    * This method sets the string value to the given string.
    * In the case of an STL string, this is equivalent to copyValue
    * since the underlying STL string object holds a copy of the data.
    *
    * @param utf8str - UTF8 null-terminated string.
    * @param nbytes  - Number of bytes to copy from the source string.
    *                  If zero, bytes are copied up to the null-terminator.
    */
   inline void setValue (const OSUTF8CHAR* utf8str, size_t nbytes = 0) {
      copyValue (utf8str, nbytes);
   }

   /**
    * This method sets the string value to the given string.
    * In the case of an STL string, this is equivalent to copyValue
    * since the underlying STL string object holds a copy of the data.
    *
    * @param cstring - C null-terminated string.
    * @param nbytes  - Number of bytes to copy from the source string.
    *                  If zero, bytes are copied up to the null-terminator.
    */
   inline void setValue (const char* cstring, size_t nbytes = 0) {
      setValue ((const OSUTF8CHAR*) cstring, nbytes);
   }

   /**
    * This method sets the string value to the value of the data
    * in the given memory buffer object.
    *
    * @param membuf  - Reference to a memory buffer object.
    */
   inline void setValue (OSRTMemBuf& membuf) {
      setValue (membuf.getData(), membuf.getDataLen());
   }

   /**
    * Assignment operator.
    */
   OSXMLSTLStringClass& operator= (const OSXMLSTLStringClass& original);

   /**
    * Assignment operator for C strings.
    */
   OSXMLSTLStringClass& operator= (const char* original);

   /**
    * Assignment operator for C UTF-8 strings.
    */
   OSXMLSTLStringClass& operator= (const OSUTF8CHAR* original);
   /**
    * String to C const char* type conversion operator.
    */
   inline operator const char* () const { return c_str(); }

   /**
    * String to C const OSUTF8CHAR* type conversion operator.
    */
   inline operator const OSUTF8CHAR* () const { return getValue(); }

   /**
    * This method returns the number of characters.
    */
   inline size_t length () const { return value.length(); }

   /**
    * This method returns the number of bytes.
    */
   inline size_t size () const { return value.size(); }

   /**
    * This method decodes XML content at the current stream/buffer position
    * into this string object.  This method is normally overridden by a
    * decodeXML method in a generated class.
    *
    * @param pctxt        Pointer to context block structure.
    * @return             Completion status of operation:
    *                       - 0 = success,
    *                       - negative return value is error.
    */
   virtual int decodeXML (OSCTXT* pctxt);

   /**
    * This method encodes the data in this string object into XML
    * content in the encode data stream.  This method is normally
    * overridden by an encodeXML method in a generated class.
    *
    * @param msgbuf       Message buffer or stream object reference.
    * @param elemName     XML element name that should be added to encoded
    *                       fragment.
    * @param pNS          Pointer to namespace structure.
    * @return             Completion status of operation:
    *                       - 0 = success,
    *                       - negative return value is error.
    */
   virtual int encodeXML (OSRTMessageBufferIF& msgbuf,
      const OSUTF8CHAR* elemName, OSXMLNamespace* pNS);

} ;
#endif
#endif
