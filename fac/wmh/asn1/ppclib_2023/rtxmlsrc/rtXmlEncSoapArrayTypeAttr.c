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

#include "rtxsrc/rtxErrCodes.h"
#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCharStr.h"

/* Encode SOAP arrayType attribute */

EXTXMLMETHOD int rtXmlEncSoapArrayTypeAttr (OSCTXT* pctxt,
                               const OSUTF8CHAR* name,
                               const OSUTF8CHAR* value,
                               size_t itemCount)
{
   size_t nameLen  = rtxUTF8LenBytes (name);
   size_t valueLen = rtxUTF8LenBytes (value);

   return rtXmlEncSoapArrayTypeAttr2
      (pctxt, name, nameLen, value, valueLen, itemCount);
}

/* This version of the function is more efficient because it does       */
/* not need to calculate the length of the attribute name and value.    */
/* The compiler can invoke it directly in places where it can calculate */
/* the lengths at compile time..                                        */

EXTXMLMETHOD int rtXmlEncSoapArrayTypeAttr2 (OSCTXT* pctxt,
                                const OSUTF8CHAR* name, size_t nameLen,
                                const OSUTF8CHAR* value, size_t valueLen,
                                size_t itemCount)
{
   char numbuf[16];
   int  nchars, stat;

   nchars = rtxUIntToCharStr ((OSUINT32)itemCount, numbuf, sizeof(numbuf), 0);
   if (nchars < 0) return LOG_RTERR (pctxt, nchars);

   /* Verify NVP and special chars will fit in encode buffer */

   stat = rtXmlCheckBuffer (pctxt, nameLen + valueLen + nchars + 5);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* Copy data to buffer */

   stat = rtXmlPutChar (pctxt, ' ');
   stat = rtXmlWriteChars (pctxt, name, nameLen);
   stat = rtXmlPutChar (pctxt, '=');
   stat = rtXmlPutChar (pctxt, '\"');

   stat = rtXmlWriteChars (pctxt, value, valueLen);
   stat = rtXmlPutChar (pctxt, '[');
   stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)numbuf, nchars);
   stat = rtXmlPutChar (pctxt, ']');

   stat = rtXmlPutChar (pctxt, '\"');

   if (!rtxCtxtTestFlag(pctxt, OSASN1XER ) &&
      rtxCtxtTestFlag (pctxt, OSXMLC14N)) {
      stat = rtXmlEncAttrC14N (pctxt);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}

