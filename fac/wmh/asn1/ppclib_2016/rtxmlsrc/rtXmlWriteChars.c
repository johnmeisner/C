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
#include "rtxsrc/rtxLatin1.h"

EXTXMLMETHOD int rtXmlWriteChars
(OSCTXT* pctxt, const OSUTF8CHAR* value, size_t len)
{
   int            stat, outlen;
   OSXMLCtxtInfo* pXmlInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (pXmlInfo == 0) {
      stat = rtxWriteBytes (pctxt, value, len);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      else return 0;
   }

   if (pXmlInfo->encoding == OSXMLUTF8) {
      stat = rtxWriteBytes (pctxt, value, len);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else if (pXmlInfo->encoding == OSXMLUTF16) {
      outlen = rtxStreamUTF8ToUTF16 (pctxt, value, len);
      if (outlen < 0) {
         return LOG_RTERR (pctxt, outlen);
      }
   }
   else if (pXmlInfo->encoding == OSXMLUTF16BE) {
      outlen = rtxStreamUTF8ToUTF16BE (pctxt, value, len);
      if (outlen < 0) {
         return LOG_RTERR (pctxt, outlen);
      }
   }
   else if (pXmlInfo->encoding == OSXMLUTF16LE) {
      outlen = rtxStreamUTF8ToUTF16LE (pctxt, value, len);
      if (outlen < 0) {
         return LOG_RTERR (pctxt, outlen);
      }
   }
   else if (pXmlInfo->encoding == OSXMLLATIN1) {
      outlen = rtxStreamUTF8ToLatin1 (pctxt, value, len);
      if (outlen < 0) {
         return LOG_RTERR (pctxt, outlen);
      }
   }

   return 0;
}

