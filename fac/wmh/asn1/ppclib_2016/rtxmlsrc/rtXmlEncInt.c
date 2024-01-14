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
/* These defs allow this function to be used as a template for other
   XML integer encode functions (int64, unsigned int, etc.) */
#ifndef RTXMLENCINTFUNC
#define RTXMLENCINTFUNC rtXmlEncInt
#define RTXMLENCINTVALUEFUNC rtXmlEncIntValue
#define RTXINTTOCHARSTR rtxIntToCharStr
#define OSINTTYPE OSINT32
#endif

#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCharStr.h"

EXTXMLMETHOD int RTXMLENCINTVALUEFUNC (OSCTXT* pctxt, OSINTTYPE value)
{
   int stat;
   char lbuf[40];

   if (pctxt->state != OSXMLATTR)
      pctxt->state = OSXMLDATA;

   stat = RTXINTTOCHARSTR (value, lbuf, sizeof(lbuf), 0);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)lbuf,
                           OSCRTLSTRLEN((const char*)lbuf));
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}

EXTXMLMETHOD int RTXMLENCINTFUNC
(OSCTXT* pctxt, OSINTTYPE value, const OSUTF8CHAR* elemName,
 OSXMLNamespace* pNS)
{
   OSRTDList namespaces;
   int stat;

   rtxDListInit(&namespaces);

   stat = rtXmlEncStartElement (pctxt, elemName, pNS, &namespaces, FALSE);

   /* if rtXmlEncStartElement added any namespaces, encode them */
   if ( stat == 0 )
      stat = rtXmlEncNSAttrs(pctxt, &namespaces);

   /* encode close of start tag */
   if ( stat == 0 )
      stat = rtXmlEncTermStartElement(pctxt);

   if (0 == stat)
      stat = RTXMLENCINTVALUEFUNC (pctxt, value);

   if (0 != elemName && 0 == stat) {
      stat = rtXmlEncEndElement (pctxt, elemName, pNS);
   }

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}


