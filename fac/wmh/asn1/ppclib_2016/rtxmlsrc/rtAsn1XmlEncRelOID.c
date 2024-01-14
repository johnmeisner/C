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

int rtAsn1XmlEncRelOID
(OSCTXT *pctxt, const ASN1OBJID *pvalue, const OSUTF8CHAR* elemName,
 const OSUTF8CHAR* nsPrefix)
{
   OSUTF8CHAR* qname = 0;
   char lbuf[40], *p;
   int  stat;
   OSUINT32 i;

   if (0 == elemName) elemName = OSUTF8("RELATIVE_OID");

   if (!OS_ISEMPTY (nsPrefix)) {
      qname = rtXmlNewQName (pctxt, elemName, nsPrefix);
      stat = rtXmlEncStartElement (pctxt, qname, 0, 0, TRUE);
   }
   else
      stat = rtXmlEncStartElement (pctxt, elemName, 0, 0, TRUE);

   if (stat != 0) return LOG_RTERR (pctxt, stat);

   pctxt->state = OSXMLDATA;

   for (i = 0; i < pvalue->numids; i++) {
      p = (char*) lbuf;
      if (i != 0)
         *p++ = '.';

      stat = rtxIntToCharStr(pvalue->subid[i], p, sizeof(lbuf), 0);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      stat = rtxCopyAsciiText (pctxt, lbuf);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   if (0 != qname) {
      stat = rtXmlEncEndElement (pctxt, qname, 0);
      rtxMemFreePtr (pctxt, qname);
   }
   else
      stat = rtXmlEncEndElement (pctxt, elemName, 0);

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}
