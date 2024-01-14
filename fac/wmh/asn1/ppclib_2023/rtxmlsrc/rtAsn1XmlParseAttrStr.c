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

int rtAsn1XmlParseAttrStr
 (OSCTXT* pctxt, const OSUTF8CHAR* pAttrStr, OSUTF8NVP* pNVPair)
{
  OSSIZE len, ct;

  /* Passed in attribute string should not be NULL. */
  if (pAttrStr == NULL) {
    return LOG_RTERR (pctxt, RTERR_NULLPTR);
  }

  /* Prevent strings of the form ="value". */
  else if ((char)pAttrStr[0] == '=') {
    return LOG_RTERR (pctxt, RTERR_INVFORMAT);
  }

  len = rtxUTF8Len (pAttrStr);
  ct  = 1;

  pNVPair = (OSUTF8NVP *) rtxMemAlloc(pctxt, sizeof(OSUTF8NVP));
  if (pNVPair == NULL) {
    return LOG_RTERR (pctxt, RTERR_NOMEM);
  }

  do {
    if ((char)pAttrStr[ct] == '=') {
      int stat = 0;

      /* Parse the name from the string; terminate with NULL. */
      pNVPair->name = rtxUTF8Strndup (pctxt, pAttrStr, ct);
      if (pNVPair->name == NULL) {
        return LOG_RTERR (pctxt, RTERR_NOMEM);
      }

      /* Now parse the value, making sure to remove the quotes and equals
         sign. */
      pNVPair->value = rtxUTF8Strndup (pctxt, &pAttrStr[ct+2], len-ct-3);
      if (pNVPair->value == NULL) {
        return LOG_RTERR (pctxt, RTERR_NOMEM);
      }

      return stat;  /* If the rtxUTF8Strndup functions worked, we're clear! */
    } /* if ((char) pAttrStr[ct] == '=') */

    ct ++;
  } while (ct != len);

  /* if we get here, we didn't find an equals sign, so we have a malformed
     string again. */
  return LOG_RTERR(pctxt, RTERR_INVFORMAT);
}
