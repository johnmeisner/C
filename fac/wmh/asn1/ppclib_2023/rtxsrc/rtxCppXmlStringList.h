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
 * @file rtxCppXmlStringList.h
 * C++ XML string list class definition.
 */
#ifndef _RTXCPPXMLSTRINGLIST_H_
#define _RTXCPPXMLSTRINGLIST_H_

#include "rtxsrc/rtxCppDList.h"
#include "rtxsrc/rtxCppXmlString.h"

/**
 * XML list string. This is the base class for generated C++ data type
 * classes for repeating occurences of XSD string types (string, token,
 * NMTOKEN, etc.).
 */
class EXTRTCLASS OSRTXMLStringList : public OSRTBaseType {
 public:
   /**
    * List of OSRTXMLString objects
    */
   OSRTObjListClass mElemList;

   /**
    * The default constructor creates an empty list.
    */
   OSRTXMLStringList () {}

   /**
    * The copy constructor creates a deep-copy of the original list.
    *
    * @param orig Object to be copied.
    */
   OSRTXMLStringList (const OSRTXMLStringList& orig);

   /**
    * The assignment operator frees the existing list and then makes a
    * deep-copy of the original list.
    *
    * @param orig Object to be assigned.
    */
   OSRTXMLStringList& operator= (const OSRTXMLStringList& orig);

   /**
    * The append method adds the given object to the end of the list.
    * The pointer is assigned directly (i.e. a deep-copy is not made).
    *
    * @param pdata Pointer to object to be appended.
    */
   void append (OSRTXMLString* pdata);

   /**
    * The appendCopy method adds a copy of the given object to the end of
    * the list.  In this case, a deep-copy of the given object is made
    * before appending it to the list.
    *
    * @param pdata Pointer to object to be appended.
    */
   void appendCopy (OSRTXMLString* pdata);

   /**
    * The clone method makes a cloned copy of this object.  It may be
    * used to create a copy of any object derived from this base class.
    */
   virtual OSRTBaseType* clone () const;

   /**
    * The getItem method returns a pointer to the indexed item in the
    * list or NULL if the index in out-of-range.
    */
   OSRTXMLString* getItem (int idx);

} ;

/* This is required for backward compatibility */
#define OSXMLStringList OSRTXMLStringList

#endif
