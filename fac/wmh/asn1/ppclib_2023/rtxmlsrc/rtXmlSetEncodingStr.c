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

#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/rtxCtype.h"

EXTXMLMETHOD int rtXmlSetEncodingStr (OSCTXT* pctxt, const OSUTF8CHAR* encodingStr)
{
   if (0 != pctxt->pXMLInfo) {
      OSXMLEncoding encoding;
      char s[16];
      size_t i, len;

      ((OSXMLCtxtInfo*)pctxt->pXMLInfo)->encodingStr = encodingStr;

      if (encodingStr == 0)
         return rtXmlSetEncoding (pctxt, OSXMLUTF8);

      len = strlen((const char*)encodingStr);
      if (len > 16)
         return rtXmlSetEncoding (pctxt, OSXMLUTF8);

      for (i = 0; i < len; i++) s[i] = (char) OS_TOUPPER(encodingStr[i]);
      s[i] = '\0';

      if (OSCRTLSTRCMP (s, OSXMLHDRUTF16) == 0)
         encoding = OSXMLUTF16;
      else if (OSCRTLSTRCMP (s, OSXMLHDRUTF16BE) == 0)
         encoding = OSXMLUTF16BE;
      else if (OSCRTLSTRCMP (s, OSXMLHDRUTF16LE) == 0)
         encoding = OSXMLUTF16LE;
      else if (OSCRTLSTRCMP (s, OSXMLHDRLATIN1) == 0)
         encoding = OSXMLLATIN1;
      else
         encoding = OSXMLUTF8;

      return rtXmlSetEncoding (pctxt, encoding);
   }
   return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
}

