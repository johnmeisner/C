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
#include "rtxsrc/rtxContext.hh"
#include "rtxmlsrc/osrtxml.hh"

/* Encode CDATA value.  Check for "]]>" in string wrapped in CDATA, which
   is illegal. */
EXTXMLMETHOD int rtXmlEncCDATAStringValue
   (OSCTXT* pctxt, const OSUTF8CHAR* value, size_t valueLen)
{
   size_t i;
   enum {
      OSXMLCDATA_CHARS,
      OSXMLCDATA_ONE_BRACKET,
      OSXMLCDATA_TWO_BRACKETS
   } state = OSXMLCDATA_CHARS;

   for (i = 0; i < valueLen; i++) {
      /* Caller must ensure buffer is big enough. */
      rtXmlPutChar (pctxt, value[i]);
      switch (state) {
      case OSXMLCDATA_CHARS:
         if (value[i] == ']')
            state = OSXMLCDATA_ONE_BRACKET;
         break;
      case OSXMLCDATA_ONE_BRACKET:
         if (value[i] == ']')
            state = OSXMLCDATA_TWO_BRACKETS;
         else
            state = OSXMLCDATA_CHARS;
         break;
      case OSXMLCDATA_TWO_BRACKETS:
         if (value[i] == '>')
            return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);
         else if (value[i] != ']')
            state = OSXMLCDATA_CHARS;
         break;
      }
   }
   return 0;
}

EXTXMLMETHOD int rtXmlEncStringValue2
   (OSCTXT* pctxt, const OSUTF8CHAR* value, size_t valueLen)
{
   if (pctxt->state != OSXMLATTR)
      pctxt->state = OSXMLDATA;

   if (0 != value && 0 != *value) {
      size_t i;
      size_t byteCount = 0;
      OSUTF8CHAR* tmpBuf = rtxMemAllocArray (pctxt, valueLen*5+1, OSUTF8CHAR);
      size_t tail = valueLen*5 + 1; /* how much of tmpBuf remains to write */
      OSUTF8CHAR* pDst = tmpBuf;
      if (tmpBuf == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);

      if (rtxCtxtTestFlag (pctxt, OSXMLC14N)) {
         int XER = rtxCtxtTestFlag(pctxt, OSASN1XER);

         /* C14N text node
         XML:
            attribute nodes: use entity and character refs for '&', '>', '<',
               CR, LF, TAB.
            text nodes: use entity refs for '&', '>', '<' and CR (0xD).
         XER:
            entity refs used for '&', '>', and '<'.
            canonical XER does not encode any values as attributes and so the
            handling for attributes below is XML-specific.
         */
         if (pctxt->state == OSXMLDATA) {
            for (i = 0; i < valueLen; i++) {
               switch (value[i]) {
               case '&':
                  OSCRTLSAFEMEMCPY (pDst, tail, "&amp;", 5); pDst += 5;
                  byteCount += 5; tail -= 5;
                  break;
               case '<':
                  OSCRTLSAFEMEMCPY (pDst, tail, "&lt;", 4); pDst += 4;
                  byteCount += 4; tail -= 4;
                  break;
               case '>':
                  OSCRTLSAFEMEMCPY (pDst, tail, "&gt;", 4); pDst += 4;
                  byteCount += 4; tail -= 4;
                  break;
               case 0xD:
                  if ( XER ) {
                     OSCRTLSAFEMEMCPY (pDst, tail, &value[i], 1); pDst += 1;
                     byteCount += 1; tail -= 1;
                  }
                  else {
                     OSCRTLSAFEMEMCPY (pDst, tail, "&#xD;", 5); pDst += 5;
                     byteCount += 5; tail -= 5;
                  }
                  break;
               default:
                  OSCRTLSAFEMEMCPY (pDst, tail, &value[i], 1); pDst += 1;
                  byteCount += 1; tail -= 1;
               }
            }
         }
         else { /* C14N attribute node */
            for (i = 0; i < valueLen; i++) {
               switch (value[i]) {
               case '&':
                  OSCRTLSAFEMEMCPY (pDst, tail, "&amp;", 5); pDst += 5;
                  byteCount += 5; tail -= 5;
                  break;
               case '"':
                  OSCRTLSAFEMEMCPY (pDst, tail, "&quot;", 6); pDst += 6;
                  byteCount += 6; tail -= 5;
                  break;
               case 0x9:
                  OSCRTLSAFEMEMCPY (pDst, tail, "&#x9;", 5); pDst += 5;
                  byteCount += 5; tail -= 5;
                  break;
               case 0xA:
                  OSCRTLSAFEMEMCPY (pDst, tail, "&#xA;", 5); pDst += 5;
                  byteCount += 5; tail -= 5;
                  break;
               case 0xD:
                  OSCRTLSAFEMEMCPY (pDst, tail, "&#xD;", 5); pDst += 5;
                  byteCount += 5; tail -= 5;
                  break;
               case '<':
                  OSCRTLSAFEMEMCPY (pDst, tail, "&lt;", 4); pDst += 4;
                  byteCount += 4; tail -= 4;
                  break;
               default:
                  OSCRTLSAFEMEMCPY (pDst, tail, &value[i], 1); pDst += 1;
                  byteCount += 1; tail--;
               }
            }
         }
      }
      else { /* not C14N */
         /* X.693 requires that 0x9, 0xA, 0xD chars be escaped in
         attribute text.  The XSD and XML specs don't require this, but I
         think it should not hurt.
         */
         if ( pctxt->state == OSXMLDATA ) {
            for (i = 0; i < valueLen; i++) {
               switch (value[i]) {
               case '<':
                  OSCRTLSAFEMEMCPY (pDst, tail, "&lt;", 4); pDst += 4;
                  byteCount += 4; tail -= 4;
                  break;
               case '>':
                  OSCRTLSAFEMEMCPY (pDst, tail, "&gt;", 4); pDst += 4;
                  byteCount += 4; tail -= 4;
                  break;
               case '&':
                  OSCRTLSAFEMEMCPY (pDst, tail, "&amp;", 5); pDst += 5;
                  byteCount += 5; tail -= 5;
                  break;
               case '\'':
                  OSCRTLSAFEMEMCPY (pDst, tail, "&apos;", 6); pDst += 6;
                  byteCount += 6; tail -= 6;
                  break;
               case '"':
                  OSCRTLSAFEMEMCPY (pDst, tail, "&quot;", 6); pDst += 6;
                  byteCount += 6; tail -= 6;
                  break;
               default:
                  OSCRTLSAFEMEMCPY (pDst, tail, &value[i], 1); pDst += 1;
                  byteCount += 1; tail --;
               }
            }
         }
         else {   /* attribute */
            for (i = 0; i < valueLen; i++) {
               switch (value[i]) {
               case '<':
                  OSCRTLSAFEMEMCPY (pDst, tail, "&lt;", 4); pDst += 4;
                  byteCount += 4; tail -= 4;
                  break;
               case '>':
                  OSCRTLSAFEMEMCPY (pDst, tail, "&gt;", 4); pDst += 4;
                  byteCount += 4; tail -= 4;
                  break;
               case '&':
                  OSCRTLSAFEMEMCPY (pDst, tail, "&amp;", 5); pDst += 5;
                  byteCount += 5; tail -= 5;
                  break;
               case '\'':
                  OSCRTLSAFEMEMCPY (pDst, tail, "&apos;", 6); pDst += 6;
                  byteCount += 6; tail -= 6;
                  break;
               case '"':
                  OSCRTLSAFEMEMCPY (pDst, tail, "&quot;", 6); pDst += 6;
                  byteCount += 6; tail -= 6;
                  break;
               case 0x9:
                  OSCRTLSAFEMEMCPY (pDst, tail, "&#x9;", 5); pDst += 5;
                  byteCount += 5; tail -= 5;
                  break;
               case 0xA:
                  OSCRTLSAFEMEMCPY (pDst, tail, "&#xA;", 5); pDst += 5;
                  byteCount += 5; tail -= 5;
                  break;
               case 0xD:
                  OSCRTLSAFEMEMCPY (pDst, tail, "&#xD;", 5); pDst += 5;
                  byteCount += 5; tail -= 5;
                  break;
               default:
                  OSCRTLSAFEMEMCPY (pDst, tail, &value[i], 1); pDst += 1;
                  byteCount += 1; tail --;
               }
            }
         }
      }
      rtXmlWriteChars (pctxt, tmpBuf, byteCount);
      rtxMemFreePtr (pctxt, tmpBuf);
   }
   LCHECKX (pctxt);
   OSRT_CHECK_EVAL_DATE1(pctxt);

   return 0;
}

EXTXMLMETHOD int rtXmlEncStringValue (OSCTXT* pctxt, const OSUTF8CHAR* value)
{
   if (0 != value && 0 != *value) {
      return rtXmlEncStringValue2 (pctxt, value, rtxUTF8LenBytes (value));
   }
   return 0;
}

EXTXMLMETHOD int rtXmlEncString (OSCTXT* pctxt,
                    OSXMLSTRING* pxmlstr,
                    const OSUTF8CHAR* elemName,
                    OSXMLNamespace* pNS)
{
   int stat = 0;
   if ( !OS_ISEMPTY(elemName) ) {
      OSRTDList namespaces;
      rtxDListInit(&namespaces);

      stat = rtXmlEncStartElement (pctxt, elemName, pNS, &namespaces, FALSE);

      /* if rtXmlEncStartElement added any namespaces, encode them */
      if ( stat == 0 )
         stat = rtXmlEncNSAttrs(pctxt, &namespaces);

      /* encode close of start tag */
      if ( stat == 0 )
         stat = rtXmlEncTermStartElement(pctxt);

      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else if (pctxt->state != OSXMLATTR) {
      stat = rtXmlEncTermStartElement (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   /* Encode string content */

   if (0 != pxmlstr) {

      OSBOOL encAttr = pctxt->state == OSXMLATTR && OS_ISEMPTY(elemName);
      OSBOOL emptyValue = OS_ISEMPTY(pxmlstr->value);

      /* C14N specifies CDATA sections are to be replaced with their
         character content.  Disregard CDATA in C14N mode. */
      OSBOOL writeCData = pxmlstr->cdata && !rtxCtxtTestFlag (pctxt, OSXMLC14N);

      /* Do not change state if encoding attr or if the value is empty */
      if (!(encAttr || emptyValue))
         pctxt->state = OSXMLDATA;

      /* C14N specifies CDATA sections are to be replaced with their
         character content.  Disregard CDATA in C14N mode. */
      if ( !emptyValue && writeCData ) {
         /* Encode CDATA section */
         size_t len = rtxUTF8LenBytes (pxmlstr->value);

         /* Verify string will fit in encode buffer */
         stat = rtXmlCheckBuffer (pctxt, len + 12);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         /* Copy data to buffer or stream */
         stat = rtXmlWriteChars (pctxt, OSUTF8("<![CDATA["), 9);
         stat = rtXmlEncCDATAStringValue (pctxt, pxmlstr->value, len);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
         stat = rtXmlWriteChars (pctxt, OSUTF8("]]>"), 3);
      }
      else {
         stat = rtXmlEncStringValue (pctxt, pxmlstr->value);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
   }

   if ( !OS_ISEMPTY(elemName) ) {
      stat = rtXmlEncEndElement (pctxt, elemName, pNS);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}

EXTXMLMETHOD int rtXmlEncUTF8Str (OSCTXT* pctxt,
                     const OSUTF8CHAR* pvalue,
                     const OSUTF8CHAR* elemName,
                     OSXMLNamespace* pNS)
{
   OSXMLSTRING xmlstr;
   xmlstr.cdata = FALSE;
   xmlstr.value = pvalue;
   return rtXmlEncString (pctxt, &xmlstr, elemName, pNS);
}

