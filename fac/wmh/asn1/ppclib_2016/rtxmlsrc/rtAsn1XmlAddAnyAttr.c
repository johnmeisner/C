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
#include "rtxsrc/rtxDList.h"
#include "rtxmlsrc/asn1xml.h"


int rtAsn1XmlAddAnyAttr
 (OSCTXT *pctxt, const OSUTF8CHAR *name, const OSUTF8CHAR *value,
  OSRTDList *plist)
{
  int stat;
  OSRTDListNode *node;

  /* Format the string. */
  OSUTF8CHAR *attrStr;
  stat = rtAsn1XmlFmtAttrStr (pctxt, name, value, &attrStr);
  if (stat != 0) return LOG_RTERR (pctxt, stat);

  /* Add the node, make sure it's okay. */
  node = rtxDListAppend (pctxt, plist, attrStr);

  if (node == NULL) {
    return LOG_RTERR (pctxt, RTERR_NOMEM);
  }

  /* Success! */
  return 0;
}
