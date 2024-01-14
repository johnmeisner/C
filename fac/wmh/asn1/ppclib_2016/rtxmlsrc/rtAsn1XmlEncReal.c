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
#include "rtxmlsrc/asn1xml.h"
#include "rtxmlsrc/osrtxml.hh"

EXTXMLMETHOD int rtAsn1XmlEncReal
(OSCTXT *pctxt, OSREAL value, const OSUTF8CHAR* elemName,
 const OSUTF8CHAR* nsPrefix)
{
   int stat = 0;
   OSUTF8CHAR* qname = 0;

   if ( elemName != 0 ) {
      if (!OS_ISEMPTY (nsPrefix)) {
         qname = rtXmlNewQName (pctxt, elemName, nsPrefix);
         stat = rtXmlEncStartElement (pctxt, qname, 0, 0, TRUE);
      }
      else
         stat = rtXmlEncStartElement (pctxt, elemName, 0, 0, TRUE);
   }

   if (0 == stat) {
      if (pctxt->state != OSXMLATTR)
         pctxt->state = OSXMLDATA;

      if (rtxIsPlusInfinity (value)) {
         stat = rtXmlEncEmptyElement(pctxt, OSUTF8("PLUS-INFINITY"), 0, 0,
            TRUE);
      }
      else if (rtxIsMinusInfinity (value)) {
         stat = rtXmlEncEmptyElement(pctxt, OSUTF8("MINUS-INFINITY"), 0, 0,
            TRUE);
      }
      else if (rtxIsNaN (value)) {
         stat = rtXmlEncEmptyElement(pctxt, OSUTF8("NOT-A-NUMBER"), 0, 0,
            TRUE);
      }
      else
         stat = rtXmlEncDoubleNormalValue(pctxt, value, 0,
            DEFAULT_DOUBLE_PRECISION);
   }

   if ( stat == 0 && elemName != 0) {
      if (0 != qname)
         stat = rtXmlEncEndElement (pctxt, qname, 0);
      else
         stat = rtXmlEncEndElement (pctxt, elemName, 0);
   }

   if (qname != 0) rtxMemFreePtr (pctxt, qname);

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;

}
