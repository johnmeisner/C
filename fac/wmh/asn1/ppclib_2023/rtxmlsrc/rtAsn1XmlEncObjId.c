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

#include "rtxmlsrc/asn1xml.h"

int rtAsn1XmlEncObjId2
(OSCTXT *pctxt, OSUINT32 numids, const OSUINT32* pSubIds,
 const OSUTF8CHAR* elemName, const OSUTF8CHAR* nsPrefix)
{
   /* Validate given object ID by applying ASN.1 rules */

   if (numids < 2) return LOG_RTERR (pctxt, ASN_E_INVOBJID);
   if (pSubIds[0] > 2) return LOG_RTERR (pctxt, ASN_E_INVOBJID);
   if (pSubIds[0] != 2 && pSubIds[1] > 39)
      return LOG_RTERR (pctxt, ASN_E_INVOBJID);

   if (0 == elemName) elemName = OSUTF8("OBJECT_IDENTIFIER");

   return rtAsn1XmlEncRelOID2 (pctxt, numids, pSubIds, elemName, nsPrefix);
}

int rtAsn1XmlEncObjId
(OSCTXT *pctxt, const ASN1OBJID *pvalue, const OSUTF8CHAR* elemName,
 const OSUTF8CHAR* nsPrefix)
{
   if (0 == pvalue) return LOG_RTERR (pctxt, ASN_E_INVOBJID);
   return rtAsn1XmlEncObjId2
      (pctxt, pvalue->numids, pvalue->subid, elemName, nsPrefix);
}
