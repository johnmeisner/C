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

#include "rtsrc/ASN1TObjId64.h"

// 64-bit object identifier class methods

EXTRTMETHOD
ASN1TObjId64::ASN1TObjId64 (OSOCTET _numids, const OSINT64* _subids)
{
   numids = _numids;
   for (OSUINT32 ui = 0; ui < numids; ui++) {
      subid[ui] = _subids[ui];
   }
}

EXTRTMETHOD ASN1TObjId64::ASN1TObjId64 (const ASN1OID64& oid) {
   numids = oid.numids;
   for (OSUINT32 ui = 0; ui < numids; ui++) {
      subid[ui] = oid.subid[ui];
   }
}

EXTRTMETHOD ASN1TObjId64::ASN1TObjId64 (const ASN1TObjId64& oid) {
   numids = oid.numids;
   for (OSUINT32 ui = 0; ui < numids; ui++) {
      subid[ui] = oid.subid[ui];
   }
}

  void ASN1TObjId64::operator= (const ASN1OID64& rhs) {
   numids = rhs.numids;
   for (OSUINT32 ui = 0; ui < numids; ui++) {
      subid[ui] = rhs.subid[ui];
   }
}

void ASN1TObjId64::operator= (const ASN1TObjId64& rhs) {
   numids = rhs.numids;
   for (OSUINT32 ui = 0; ui < numids; ui++) {
      subid[ui] = rhs.subid[ui];
   }
}
