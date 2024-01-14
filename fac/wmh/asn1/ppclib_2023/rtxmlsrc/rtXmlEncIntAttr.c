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
/* These defs allow this function to be used as a template for other
   XML integer encode functions (int64, unsigned int, etc.) */
#ifndef RTXMLENCINTFUNC
#define RTXMLENCINTFUNC rtXmlEncIntAttr
#define RTXINTTOCHARSTR rtxIntToCharStr
#define OSINTTYPE OSINT32
#endif

#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCharStr.h"

EXTXMLMETHOD int RTXMLENCINTFUNC (OSCTXT* pctxt, OSINTTYPE value,
                     const OSUTF8CHAR* attrName, size_t attrNameLen)
{
   int stat, len;
   char lbuf[40];

   OSRTASSERT (0 != attrName);

   len = RTXINTTOCHARSTR (value, lbuf, sizeof(lbuf), 0);
   if (len < 0) return LOG_RTERR (pctxt, len);

   /* Verify element will fit in encode buffer */

   /* 4 extra chars = space + equal sign + two double quotes + final '>' */
   stat = rtXmlCheckBuffer (pctxt, attrNameLen + len + 5);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* Copy data to buffer */

   stat = rtXmlPutChar (pctxt, ' ');
   stat = rtXmlWriteChars (pctxt, attrName, attrNameLen);
   stat = rtXmlPutChar (pctxt, '=');
   stat = rtXmlPutChar (pctxt, '\"');
   stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)lbuf, len);
   stat = rtXmlPutChar (pctxt, '\"');

   if (!rtxCtxtTestFlag(pctxt, OSASN1XER ) &&
      rtxCtxtTestFlag (pctxt, OSXMLC14N))
      rtXmlEncAttrC14N (pctxt);

   return 0;
}

