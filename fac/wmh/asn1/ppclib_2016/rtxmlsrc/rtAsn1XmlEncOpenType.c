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
#include "rtxmlsrc/asn1xml.h"

static OSBOOL isBinaryData (const OSOCTET *data, OSSIZE nocts);

EXTXMLMETHOD int rtAsn1XmlEncOpenType
(OSCTXT *pctxt, const OSOCTET* data, OSSIZE nocts,
 const OSUTF8CHAR* elemName, const OSUTF8CHAR* nsPrefix)
{
   OSUTF8CHAR* qname = 0;
   int stat;

   if (isBinaryData (data, nocts)) {
      const OSUTF8CHAR *ename = (OS_ISEMPTY(elemName)) ?
         OSUTF8("binext") : elemName;

      qname = rtXmlNewQName (pctxt, ename, nsPrefix);

      if (0 != qname) {
         stat = rtXmlEncStartElement (pctxt, qname, 0, 0, TRUE);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      else return RTERR_NOMEM;

      if (0 != data && nocts > 0) {
         stat = rtXmlEncHexStrValue (pctxt, nocts, data);
         pctxt->state = OSXMLDATA;

         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }

      if (0 != qname) {
         stat = rtXmlEncEndElement (pctxt, qname, 0);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         rtxMemFreePtr (pctxt, qname);
      }
   }

   else {
      if (! OS_ISEMPTY(elemName) ) {
         qname = rtXmlNewQName (pctxt, elemName, nsPrefix);

         if (0 != qname) {
            stat = rtXmlEncStartElement (pctxt, qname, 0, 0, TRUE);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }
         else return RTERR_NOMEM;
      }
      else {
         stat = rtXmlEncIndent(pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }

      if (0 != data && nocts > 0) {
         OSRTSAFEMEMCPY (pctxt, data, nocts);
         pctxt->state = OSXMLDATA;
      }

      if (0 != qname) {
         stat = rtXmlEncEndElement (pctxt, qname, 0);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         rtxMemFreePtr (pctxt, qname);
      }
      else {
         pctxt->state = OSXMLEND;
      }
   }

   return 0;
}

static OSBOOL isBinaryData (const OSOCTET *data, OSSIZE nocts)
{
   OSSIZE i = 0;
   int wc, wclen;

   /* The minimal length for valid XML markup is 4 bytes
      (something like <a/>) */
   if (nocts < 4) return TRUE;

   /* First char must be '<' and last char must be '>' */
   if (!(data[0] == '<' && data[nocts-1] == '>')) return TRUE;

   /* All characters in between must be valid UTF-8 chars */
   for (i = 1; i < (nocts - 1); i += wclen) {
      wc = rtxUTF8DecodeChar (0, &data[i], &wclen);
      if (wc < 0) /* invalid char */ return TRUE;
   }

   return FALSE;
}

