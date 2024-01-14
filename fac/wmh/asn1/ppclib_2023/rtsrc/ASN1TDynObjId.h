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
/**
 * @file rtsrc/ASN1TDynObjId.h
 * ASN.1 dynamic object identifier class definition.
 */
#ifndef _ASN1TDYNOBJID_H_
#define _ASN1TDYNOBJID_H_

#include "rtsrc/asn1type.h"

/**
 * @ingroup cppruntime
 */
/**
 * @addtogroup asn1data
 * @{
 */
/**
 * Object identifier. This is the base class for generated C++ data type
 * classes for dynamic object identifier values. The difference between
 * this and the ASN1TObjId class is that this uses a dynamic array for
 * subidentifiers instead of a static array and is thus more memory
 * efficient.
 */
struct EXTRTCLASS ASN1TDynObjId : public ASN1DynOBJID {
   /**
    * Standard memory flag. This only has meaning if the memAllocated flag
    * is set in the base class. It indicates standard memory was allocated
    * using the new operator. This type of memory is freed in the destructor.
    * Non-standard memory must be freed externally.
    */
   bool mbStdMem;

   /**
    * The default constructor creates an empty object identifier value.
    */
   ASN1TDynObjId () {
      memAllocated = FALSE; numids = 0; pSubIds = NULL, mbStdMem = false;
   }

   /**
    * The Virtual Destructor
    */
   virtual EXTRTMETHOD ~ASN1TDynObjId ();

   /**
    * This constructor initializes the object identifer to contain the given
    * data values.
    *
    * @param _numids      - Number of subidentifiers in the OID.
    * @param _subids      - Array of subidentifier values.
    */
   EXTRTMETHOD ASN1TDynObjId (OSOCTET _numids, const OSUINT32* _subids);

   /**
    * This constructor initializes the object identifier to contain the given
    * data values. This can be used to set the value to a compiler-generated
    * OID value.
    *
    * @param oid          - C object identifier value.
    */
   EXTRTMETHOD ASN1TDynObjId (const ASN1OBJID& oid);

   /**
    * The copy constructor.
    *
    * @param oid          - C++ object identifier value.
    */
   EXTRTMETHOD ASN1TDynObjId (const ASN1TDynObjId& oid);

   /**
    * Construct an OID from a dotted string.
    *
    * @param dotted_oid_string - for example "1.3.1.6.1.10"
    */
   EXTRTMETHOD ASN1TDynObjId (const char *dotted_oid_string);

   /**
    * Assignment from a string.
    *
    * @param dotted_oid_string - New value (for example "1.3.6.1.6.0");
    */
   EXTRTMETHOD ASN1TDynObjId& operator=(const char *dotted_oid_string);

   /**
    * This assignment operator sets the object identifier to contain the OID in
    * the given C structure. This can be used to set the value to a
    * compiler-generated OID value.
    *
    * @param rhs          - C object identifier value.
    */
   EXTRTMETHOD void operator= (const ASN1OBJID& rhs);

   /**
    * This assignment operator sets the object identifier to contain the OID in
    * the given C++ structure.
    *
    * @param rhs          - C++ object identifier value.
    */
   EXTRTMETHOD void operator= (const ASN1TDynObjId& rhs);

   /**
    * Overloaded  += operator.
    * This operator allows subidentifiers in the form of a dotted OID
    * string ("n.n.n") to be appended to an existing OID object.
    *
    * @param dotted_oid_string - C++ object identifier value.
    * @return             - True if values are equal.
    */
   EXTRTMETHOD ASN1TDynObjId& operator+=(const char *dotted_oid_string);

   /**
    * Overloaded  += operator.
    * This operator allows a single subidentifier in the form of an
    * integer value to be appended to an existing OID object.
    *
    * @param i            - Subidentifier to append.
    * @return             - True if values are equal.
    */
   EXTRTMETHOD ASN1TDynObjId& operator+=(const OSUINT32 i);

   /**
    * Overloaded  += operator.
    * This operator allows one object identifier to be appended to
    * another object identifier.
    *
    * @param o            - C++ object identifier value.
    * @return             - True if values are equal.
    */
   EXTRTMETHOD ASN1TDynObjId& operator+=(const ASN1TDynObjId &o);

   /**
    * Append sub-identifiers to this OID.
    *
    * @param nsubids Number of subids to append.
    * @param pSubIdArray Pointer to array containing sub-identifiers.
    * @return Zero if operation successful or negative error code.
    */
   EXTRTMETHOD int appendSubIds(OSSIZE nsubids, const OSUINT32* pSubIdArray);

   /**
    * Get a printable ASCII string of a part of the value.
    *
    * @param pctxt        - Pointer to a context structure.
    * @return             - Dotted OID string (for example "3.6.1.6")
    */
   EXTRTMETHOD const char *toString (OSCTXT* pctxt) const;

   /**
    * Sets the data of an object identifier using a pointer and a length.
    *
    * @param raw_oid      - Pointer to an array of subidentifier values.
    * @param oid_len      - Number of subids in the array,
    */
   EXTRTMETHOD void set_data (const OSUINT32 *raw_oid, OSUINT32 oid_len);

   /**
    * Compare the first n sub-ids(left to right) of two object identifiers.
    *
    * @param n            - Number of subid values to compare.
    * @param o            - OID to compare this OID with.
    * @return             - 0 if OID's are equal, -1 if this OID less than
    *                       given OID, +1 if this OID > given OID.
    */
   EXTRTMETHOD int nCompare(const OSUINT32 n, const ASN1TDynObjId &o) const;

   /**
    * Compare the last n sub-ids(right to left) of two object identifiers.
    *
    * @param n            - Number of subid values to compare.
    * @param o            - OID to compare this OID with.
    * @return             - 0 if OID's are equal, -1 if this OID less than
    *                       given OID, +1 if this OID > given OID.
    */
   EXTRTMETHOD int RnCompare(const OSUINT32 n, const ASN1TDynObjId &o) const;

   /**
    * Trim the given number of rightmost sub elements from this OID.
    *
    * @param n - number of subids to trim from OID
    */
   EXTRTMETHOD void trim(const OSUINT32 n);

   /**
    * Copy an OID value into this OID object. Memory is allocated to hold
    * the subidentifiers using the new operator.
    *
    * @param _numids Number of sub-identifiers
    * @param _subids Array of sub-identifiers to be copied.
    */
   EXTRTMETHOD void stdMemCopy(OSSIZE _numids, const OSUINT32* _subids);

   /**
    * Read a dotted OID string into this OID object.
    *
    * @param str Dotted OID string to parse (ex. 1.2.3).
    * @return Number of sub-identifiers in string or negative error code.
    */
   EXTRTMETHOD int strToOid (const char *str);

};

/**
 * This comparison operator allows for comparison of equality of two C-based
 * dynamic object identifier structures.
 *
 * @param lhs          - C object identifier value.
 * @param rhs          - C object identifier value.
 * @return             - True if values are equal.
 */
EXTERNRT int operator== (const ASN1TDynObjId& lhs, const ASN1TDynObjId& rhs);

/**
 * This comparison operator allows for comparison of equality of a C-based
 * dynamic object identifier structure and a dotted string.
 *
 * @param lhs          - C object identifier value.
 * @param dotted_oid_string - String containing OID value to compare.
 * @return             - True if values are equal.
 */
EXTERNRT int operator==(const ASN1TDynObjId &lhs, const char *dotted_oid_string);

/**
 * Overloaded not equal operator.
 * This comparison operator allows for comparison of not equality of  C++ based
 * dynamic object identifier structure and a dotted string.
 *
 * @param lhs          - C++ object identifier value.
 * @param rhs          - C++ object identifier value
 * @return             - True if values are equal.
 */
EXTERNRT int operator!=(const ASN1TDynObjId &lhs, const ASN1TDynObjId &rhs);

/**
 * Overloaded not equal operator.
 * This comparison operator allows for comparison of not equality of  C based
 * dynamic object identifier structure and a dotted string.
 *
 * @param lhs          - C object identifier value.
 * @param rhs          - C object identifier value
 * @return             - True if values are equal.
 */
EXTERNRT int operator!=(const ASN1DynOBJID &lhs, const ASN1OBJID &rhs);

/**
 * Overloaded not equal operator.
 * This comparison operator allows for comparison of not equality of  C based
 * dynamic object identifier structure and a dotted string.
 *
 * @param lhs          - C object identifier value.
 * @param dotted_oid_string - String containing OID value to compare.
 * @return             - True if values are equal.
 */
EXTERNRT int operator!=(const ASN1DynOBJID &lhs, const char *dotted_oid_string);

/**
 * Overloaded not equal operator.
 * This comparison operator allows for comparison of not equality of  C++ based
 * dynamic object identifier structure and a dotted string.
 *
 * @param lhs          - C++ object identifier value.
 * @param dotted_oid_string - String containing OID value to compare.
 * @return             - True if values are equal.
 */
EXTERNRT int operator!=(const ASN1TDynObjId &lhs, const char *dotted_oid_string);

/**
 * Overloaded less than < operator.
 * This comparison operator allows for comparison of less than of  C++ based
 * dynamic object identifier structure and a dotted string.
 *
 * @param lhs          - C++ object identifier value.
 * @param rhs          - C++ object identifier value.
 * @return             - True if values are equal.
 */
EXTERNRT int operator<(const ASN1TDynObjId &lhs, const ASN1TDynObjId &rhs);

/**
 * Overloaded less than < operator.
 * This comparison operator allows for comparison of less than of  C based
 * dynamic object identifier structure and a dotted string.
 *
 * @param lhs          - C object identifier value.
 * @param rhs          - C object identifier value.
 * @return             - True if values are equal.
 */
EXTERNRT int operator<(const ASN1DynOBJID &lhs, const ASN1DynOBJID &rhs);

/**
 * Overloaded less than < operator.
 * This comparison operator allows for comparison of less than of  C based
 * dynamic object identifier structure and a dotted string.
 *
 * @param lhs          - C object identifier value.
 * @param dotted_oid_string - String containing OID value to compare.
 * @return             - True if values are equal.
 */
EXTERNRT int operator<(const ASN1DynOBJID &lhs, const char *dotted_oid_string);

/**
 * Overloaded less than < operator.
 * This comparison operator allows for comparison of less than of  C++ based
 * dynamic object identifier structure and a dotted string.
 *
 * @param lhs          - C++ object identifier value.
 * @param dotted_oid_string - String containing OID value to compare.
 * @return             - True if values are equal.
 */
EXTERNRT int operator<(const ASN1TDynObjId &lhs, const char *dotted_oid_string);

/**
 * Overloaded less than <= operator.
 * This comparison operator allows for comparison of less than of  C++ based
 * dynamic object identifier structure and a dotted string.
 *
 * @param lhs          - C++ object identifier value.
 * @param rhs          - C++ object identifier value
 * @return             - True if values are equal.
 */
EXTERNRT int operator<=(const ASN1TDynObjId &lhs, const ASN1TDynObjId &rhs);

/**
 * Overloaded less than <= operator.
 * This comparison operator allows for comparison of less than of  C based
 * dynamic object identifier structure and a dotted string.
 *
 * @param lhs          - C object identifier value.
 * @param rhs          - C object identifier value
 * @return             - True if values are equal.
 */
EXTERNRT int operator<=(const ASN1DynOBJID &lhs, const ASN1DynOBJID &rhs);

/**
 * Overloaded less than <= operator.
 * This comparison operator allows for comparison of less than of a C++ based
 * dynamic object identifier structure and a dotted string.
 *
 * @param lhs          - C++ object identifier value.
 * @param dotted_oid_string - String containing OID value to compare.
 * @return             - True if values are equal.
 */

EXTERNRT int operator<=(const ASN1TDynObjId &lhs, const char *dotted_oid_string);

/**
 * Overloaded less than <= operator.
 * This comparison operator allows for comparison of less than or equal of
 * a C based object identifier structure and a dotted string.
 *
 * @param lhs          - C object identifier value.
 * @param dotted_oid_string - String containing OID value to compare.
 * @return             - True if values are equal.
 */
EXTERNRT int operator<=(const ASN1DynOBJID &lhs, const char *dotted_oid_string);


/**
 * Overloaded greater than > operator.
 * This comparison operator allows for comparison of greater than of C++ based
 * dynamic object identifier structures
 *
 * @param lhs          - C++ object identifier value.
 * @param rhs          - C++ object identifier value.
 * @return             - True if values are equal.
 */
EXTERNRT int operator>(const ASN1TDynObjId &lhs, const ASN1TDynObjId &rhs);

/**
 * Overloaded greater than > operator.
 * This comparison operator allows for comparison of greater than of a C++
 * based object identifier structure and a dotted string.
 *
 * @param lhs          - C++ object identifier value.
 * @param dotted_oid_string - String containing OID value to compare.
 * @return             - True if values are equal.
 */
EXTERNRT int operator>(const ASN1TDynObjId &lhs, const char *dotted_oid_string);

/**
 * Overloaded greater than > operator.
 * This comparison operator allows for comparison of greater than of C based
 * dynamic object identifier structures.
 *
 * @param lhs          - C object identifier value.
 * @param rhs          - C object identifier value.
 * @return             - True if values are equal.
 */
EXTERNRT int operator>(const ASN1DynOBJID &lhs, const ASN1DynOBJID &rhs);

/**
 * Overloaded greater than > operator.
 * This comparison operator allows for comparison of greater than of a C based
 * dynamic object identifier structure and a dotted string.
 *
 * @param lhs          - C object identifier value.
 * @param dotted_oid_string - String containing OID value to compare.
 * @return             - True if values are equal.
 */
EXTERNRT int operator>(const ASN1DynOBJID &lhs, const char *dotted_oid_string);

/**
 * Overloaded greater than equal >= operator.
 * This comparison operator allows for comparison of greater than or equal
 * of C++ based object identifier structures.
 *
 * @param lhs          - C++ object identifier value.
 * @param rhs          - C++ object identifier value.
 * @return             - True if values are equal.
 */
EXTERNRT int operator>=(const ASN1TDynObjId &lhs, const ASN1TDynObjId &rhs);

/**
 * Overloaded greater than equal >= operator.
 * This comparison operator allows for comparison of greater than or equal
 * of a C++ based dynamic object identifier structure and a dotted string.
 *
 * @param lhs          - C++ object identifier value.
 * @param dotted_oid_string - String containing OID value to compare.
 * @return             - True if values are equal.
 */
EXTERNRT int operator>=(const ASN1TDynObjId &lhs, const char *dotted_oid_string);

/**
 * Overloaded greater than equal >= operator.
 * This comparison operator allows for comparison of greater than or equal
 * of C based dynamic object identifier structures.
 *
 * @param lhs          - C object identifier value.
 * @param rhs          - C object identifier value.
 * @return             - True if values are equal.
 */
EXTERNRT int operator>=(const ASN1DynOBJID &lhs, const ASN1DynOBJID &rhs);


/**
 * Overloaded greater than equal >= operator.
 * This comparison operator allows for comparison of greater than or equal
 * of a C based dynamic object identifier structure and a dotted string.
 *
 * @param lhs          - C object identifier value.
 * @param dotted_oid_string - String containing OID value to compare.
 * @return             - True if values are equal.
 */
EXTERNRT int operator>=(const ASN1DynOBJID &lhs, const char *dotted_oid_string);

/**
 * Overloaded append + operator.
 * This operator allows two dynamic Object Identifier values to be concatanated.
 *
 * @param lhs          - C++ object identifier value.
 * @param rhs          - C++ object identifier value.
 *
 */
EXTERNRT ASN1TDynObjId operator +(const ASN1TDynObjId &lhs,
                                  const ASN1TDynObjId &rhs);

#endif

/**
 * @}
 */
