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

#include "rtsrc/asn1type.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxmlsrc/asn1xml.h"


int rtAsn1XmlpDecRelOID (OSCTXT* pctxt, ASN1OBJID *pvalue)
{
   const OSUTF8CHAR* str;
   int stat;

   pvalue->numids = 0;

   /* Get OID string */

   stat = rtXmlpDecDynUTF8Str (pctxt, &str);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* Parse string */

   stat = rtRelOIDParseString
      ((const char*)str, OSUTF8LEN(str), pvalue);

   rtxMemFreeArray (pctxt, str);

   return (0 == stat) ? 0 : LOG_RTERR (pctxt, stat);
}


int rtAsn1XmlpDecObjId (OSCTXT* pctxt, ASN1OBJID *pvalue)
{
   const OSUTF8CHAR* str;
   int stat;

   pvalue->numids = 0;

   /* Get OID string */

   stat = rtXmlpDecDynUTF8Str (pctxt, &str);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* Parse string */

   stat = rtOIDParseString
      ((const char*)str, OSUTF8LEN(str), pvalue);

   rtxMemFreeArray (pctxt, str);

   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* Validate given object ID by applying ASN.1 rules */

   return (rtOIDIsValid (pvalue)) ? 0 : LOG_RTERR (pctxt, ASN_E_INVOBJID);
}
