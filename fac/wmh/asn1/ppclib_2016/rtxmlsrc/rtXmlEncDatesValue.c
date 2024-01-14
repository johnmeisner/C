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
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxDateTime.hh"

EXTXMLMETHOD int rtXmlEncGYearValue (OSCTXT* pctxt, const OSXSDDateTime* pvalue)
{
   int stat;
   char lbuf[80];

   stat = rtxGYearToString (pvalue, (OSUTF8CHAR*)lbuf, sizeof (lbuf));
   if (stat < 0) return LOG_RTERRNEW (pctxt, stat);

   stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)lbuf, OSCRTLSTRLEN(lbuf));
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}

EXTXMLMETHOD int rtXmlEncGYearMonthValue (OSCTXT* pctxt, const OSXSDDateTime* pvalue)
{
   int stat;
   char lbuf[80];

   stat = rtxGYearMonthToString (pvalue, (OSUTF8CHAR*)lbuf, sizeof (lbuf));
   if (stat < 0) return LOG_RTERRNEW (pctxt, stat);

   stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)lbuf, OSCRTLSTRLEN(lbuf));
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}

EXTXMLMETHOD int rtXmlEncGMonthValue (OSCTXT* pctxt, const OSXSDDateTime* pvalue)
{
   int stat;
   char lbuf[80];

   stat = rtxGMonthToString (pvalue, (OSUTF8CHAR*)lbuf, sizeof (lbuf));
   if (stat < 0) return LOG_RTERRNEW (pctxt, stat);

   stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)lbuf, OSCRTLSTRLEN(lbuf));
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}

EXTXMLMETHOD int rtXmlEncGMonthDayValue (OSCTXT* pctxt, const OSXSDDateTime* pvalue)
{
   int stat;
   char lbuf[80];

   stat = rtxGMonthDayToString (pvalue, (OSUTF8CHAR*)lbuf, sizeof (lbuf));
   if (stat < 0) return LOG_RTERRNEW (pctxt, stat);

   stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)lbuf, OSCRTLSTRLEN(lbuf));
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}

EXTXMLMETHOD int rtXmlEncGDayValue (OSCTXT* pctxt, const OSXSDDateTime* pvalue)
{
   int stat;
   char lbuf[80];

   stat = rtxGDayToString (pvalue, (OSUTF8CHAR*)lbuf, sizeof (lbuf));
   if (stat < 0) return LOG_RTERRNEW (pctxt, stat);

   stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)lbuf, OSCRTLSTRLEN(lbuf));
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}


