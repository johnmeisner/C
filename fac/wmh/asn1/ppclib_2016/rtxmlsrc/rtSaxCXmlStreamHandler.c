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
#include "rtxmlsrc/rtSaxCXmlStreamHandler.h"

EXTXMLMETHOD int SAX2XMLStreamStartElement
(void *userData, const OSUTF8CHAR* localname,
 const OSUTF8CHAR* qname, const OSUTF8CHAR* const* attrs)
{
   OSCTXT* pctxt = (OSCTXT*) userData;

   if (pctxt->state == OSXMLINIT) {
      rtXmlEncStartDocument (pctxt);
   }
   else if (pctxt->state == OSXMLSTART) {
      rtXmlPutChar (pctxt, '>');
   }
   if (pctxt->state != OSXMLDATA) {
      rtXmlEncIndent (pctxt);
   }
   pctxt->level++;
   pctxt->state = OSXMLSTART;

   /* Verify element will fit in encode buffer */

   if (0 == rtXmlCheckBuffer (pctxt, OSUTF8LEN(qname) + 1)) {
      /* Copy element QName to buffer */
      rtXmlPutChar (pctxt, '<');
      rtXmlWriteChars (pctxt, qname, OSUTF8LEN(qname));

      if (0 != attrs && 0 != attrs[0]) {
         OSUINT32 i = 0;
         while (0 != attrs[i]) {
            rtXmlEncUTF8Attr (pctxt, attrs[i], attrs[i+1]);
            i += 2;
         }
      }
   }
   if (0 != localname) {} /* to suppress level 4 warning */

   return 0;
}

EXTXMLMETHOD int SAX2XMLStreamCharacters
(void *userData, const OSUTF8CHAR* chars, int length)
{
   OSCTXT* pctxt = (OSCTXT*) userData;
   if (pctxt->state == OSXMLSTART) {
      rtXmlPutChar (pctxt, '>');
   }
   rtXmlEncStringValue2 (pctxt, chars, length);
   pctxt->state = OSXMLDATA;
   return 0;
}

EXTXMLMETHOD int SAX2XMLStreamEndElement
(void *userData, const OSUTF8CHAR* localname, const OSUTF8CHAR* qname)
{
   OSCTXT* pctxt = (OSCTXT*) userData;
   pctxt->level--;

   if (pctxt->state == OSXMLSTART) {
      rtXmlWriteChars (pctxt, OSUTF8("/>"), 2);
   }
   else {
      if (pctxt->state == OSXMLEND) {
         rtXmlEncIndent (pctxt);
      }
      if (0 == rtXmlCheckBuffer (pctxt, OSUTF8LEN(qname) + 3)) {
         rtXmlPutChar (pctxt, '<');
         rtXmlPutChar (pctxt, '/');
         rtXmlWriteChars (pctxt, qname, OSUTF8LEN(qname));
         rtXmlPutChar (pctxt, '>');
      }
   }
   pctxt->state = OSXMLEND;
   if (0 != localname) {} /* to suppress level 4 warning */
   return 0;
}

