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
#include "rtxsrc/rtxUTF16.h"

static const OSUINT32 endian = 0x01020304;
static const OSOCTET *pEndian = (OSOCTET*)&endian;

/* This function is used to encode an ASCII character */
EXTXMLMETHOD int rtXmlPutChar (OSCTXT* pctxt, const OSUTF8CHAR value)
{
   OSXMLCtxtInfo* pXmlInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;
   if (pXmlInfo == 0) {
      OSRTSAFEPUTCHAR (pctxt, value);
      return 0;
   }

   if (pXmlInfo->encoding == OSXMLUTF8 ||
       pXmlInfo->encoding == OSXMLLATIN1) {
      OSRTSAFEPUTCHAR (pctxt, value);
   }
   else if (pXmlInfo->encoding == OSXMLUTF16) {
      if (pEndian[0] == 4) { /* on a little endian system */
         OSRTSAFEPUTCHAR (pctxt, value);
         OSRTSAFEPUTCHAR (pctxt, '\0');
      }
      else { /* big endian */
         OSRTSAFEPUTCHAR (pctxt, '\0');
         OSRTSAFEPUTCHAR (pctxt, value);
      }
   }
   else if (pXmlInfo->encoding == OSXMLUTF16BE) {
      OSRTSAFEPUTCHAR (pctxt, '\0');
      OSRTSAFEPUTCHAR (pctxt, value);
   }
   else if (pXmlInfo->encoding == OSXMLUTF16LE) {
      OSRTSAFEPUTCHAR (pctxt, value);
      OSRTSAFEPUTCHAR (pctxt, '\0');
   }

   return 0;
}

