/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
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

#include "rtxmlsrc/osrtxml.hh"

EXTXMLMETHOD int rtXmlEncDecimalAttr (OSCTXT* pctxt, OSREAL value,
                         const OSUTF8CHAR* attrName, size_t attrNameLen,
                         const OSDecimalFmt* pFmtSpec)
{
   int stat;

   OSRTASSERT (0 != attrName);

   /* Verify element will fit in encode buffer */

   /* 3 extra chars = space + equal sign + one double quote */
   stat = rtXmlCheckBuffer (pctxt, attrNameLen + 5);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* Copy data to buffer */

   stat = rtXmlPutChar (pctxt, ' ');
   stat = rtXmlWriteChars (pctxt, attrName, attrNameLen);
   stat = rtXmlPutChar (pctxt, '=');
   stat = rtXmlPutChar (pctxt, '\"');
   stat = rtXmlEncDecimalValue (pctxt, value, pFmtSpec, 0, 0);
   stat = rtXmlPutChar (pctxt, '\"');

   if (stat < 0) return LOG_RTERR (pctxt, stat);

   if (!rtxCtxtTestFlag(pctxt, OSASN1XER ) &&
      rtxCtxtTestFlag (pctxt, OSXMLC14N)) {
      stat = rtXmlEncAttrC14N (pctxt);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}

