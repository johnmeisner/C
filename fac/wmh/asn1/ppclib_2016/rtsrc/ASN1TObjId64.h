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
/**
 * @file rtsrc/ASN1TObjId64.h
 * ASN.1 object identifier with 64-bit arcs class definition.
 */
#ifndef _ASN1TOBJID64_H_
#define _ASN1TOBJID64_H_

#include "rtsrc/asn1type.h"

/**
 * @ingroup cppruntime
 */
/**
 * @addtogroup asn1data
 * @{
 */
/**
 * Object identifier with 64-bit arcs. This class is identical to the
 * ASN1TObjId class except it allows 64-bit integers to be used for the arc
 * (i.e. subidentifier) values.
 */
struct EXTRTCLASS ASN1TObjId64 : public ASN1OID64 {
   /**
    * Constructs an empty 64-bit object identifier, i.e., one with no ids.
    */
   inline ASN1TObjId64 () { numids = 0; }

   /**
    * Constructs a 64-bit object identifier with the given ids and number of
    * ids.
    *
    * @param   _numids     The number of ids (up to 128).
    * @param   _subids     The actual subids.
    */
   EXTRTMETHOD ASN1TObjId64 (OSOCTET _numids, const OSINT64* _subids);

   /**
    * The copy constructor.
    *
    * @param   oid      A reference to an ::ASN1OID64 structure.
    */
   EXTRTMETHOD ASN1TObjId64 (const ASN1OID64& oid);

   /**
    * The copy constructor.
    *
    * @param   oid      A reference to an ::ASN1TObjId64 structure.
    */
   EXTRTMETHOD ASN1TObjId64 (const ASN1TObjId64& oid);

   /**
    * The assignment operator.
    *
    * @param   rhs      A reference to an ::ASN1OID64 structure.
    */
   EXTRTMETHOD void operator= (const ASN1OID64& rhs);

   /**
    * The assignment operator.
    *
    * @param   rhs      A reference to an ::ASN1TObjId64 structure.
    */
   EXTRTMETHOD void operator= (const ASN1TObjId64& rhs);
} ;

#endif

/**
 * @}
 */
