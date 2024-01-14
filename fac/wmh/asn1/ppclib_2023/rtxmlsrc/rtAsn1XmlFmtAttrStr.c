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

#include "rtsrc/asn1type.h"
#include "rtxmlsrc/asn1xml.h"

int rtAsn1XmlFmtAttrStr
(OSCTXT* pctxt, const OSUTF8CHAR* name, const OSUTF8CHAR* value,
 OSUTF8CHAR** ppAttrStr)
{
   /* Provided name, value pair should not be null. */
   if (name == NULL) {
      return LOG_RTERR (pctxt, RTERR_NULLPTR);
   }
   else if (value == NULL) {
      return LOG_RTERR (pctxt, RTERR_NULLPTR);
   }

   /* The name should not be empty. */
   if (*name == 0) {
      return LOG_RTERR (pctxt, RTERR_INVFORMAT);
   }

   /* Join string parts */
   *ppAttrStr = (OSUTF8CHAR*)
      rtxUTF8StrJoin (pctxt, name, OSUTF8("=\""), value, OSUTF8("\""), 0);

   if (0 == *ppAttrStr) {
      return LOG_RTERR (pctxt, RTERR_NOMEM);
   }

   return 0;
}
