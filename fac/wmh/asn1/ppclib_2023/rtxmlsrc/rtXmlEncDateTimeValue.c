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
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxDateTime.hh"
#include "rtxsrc/rtxContext.hh"

/*
 * rtXmlEncDateValue:
 * This function encodes a variable of the XSD DateTime type as an
 * string. This version of the function is used
 * to encode OSXSDDateTime value into CCYY-MM-DD format
 */
EXTXMLMETHOD int rtXmlEncDatePartValue (OSCTXT* pctxt, const OSXSDDateTime* pvalue)
{
   int stat;
   char lbuf[80];

   if (pvalue->day < 1 || pvalue->day > 31 ||
       pvalue->mon < 1 || pvalue->mon > 12)
       return LOG_RTERRNEW (pctxt, RTERR_BADVALUE);

   stat = rtxDatePartToString (pvalue, (OSUTF8CHAR*)lbuf, sizeof (lbuf));
   if (stat < 0) return LOG_RTERRNEW (pctxt, stat);

   stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)lbuf, OSCRTLSTRLEN(lbuf));
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}

EXTXMLMETHOD int rtXmlEncDateValue (OSCTXT* pctxt, const OSXSDDateTime* pvalue)
{
   int stat;
   char lbuf[80];

   if (pvalue->day < 1 || pvalue->day > 31 ||
       pvalue->mon < 1 || pvalue->mon > 12)
       return LOG_RTERRNEW (pctxt, RTERR_BADVALUE);

   stat = rtxDateToString (pvalue, (OSUTF8CHAR*)lbuf, sizeof (lbuf));
   if (stat < 0) return LOG_RTERRNEW (pctxt, stat);

   stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)lbuf, OSCRTLSTRLEN(lbuf));
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   LCHECKX (pctxt);

   return 0;
}

/*
 * rtXmlEncTimeValue:
 * This function encodes a variable of the XSD DateTime type as an
 * string. This version of the function is used
 * to encode OSXSDDateTime value into different format.
 */
EXTXMLMETHOD int rtXmlEncTimeValue (OSCTXT* pctxt, const OSXSDDateTime* pvalue)
{
   int stat;
   char lbuf[80] = "";

   if (pvalue->hour > 23 || pvalue->min  > 59 ||
       pvalue->sec < 0.0 || pvalue->sec >= 60.0)
      return LOG_RTERRNEW (pctxt, RTERR_BADVALUE);

   stat = rtxTimeToString (pvalue, (OSUTF8CHAR*)lbuf, sizeof (lbuf));
   if (stat < 0) return LOG_RTERRNEW (pctxt, stat);

   stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)lbuf, OSCRTLSTRLEN(lbuf));
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   LCHECKX (pctxt);

   return 0;
}

/*
 * rtXmlEncDateTimeValue:
 * This function encodes a variable of the XSD DateTime type as an
 * string. This version of the function is used to encode OSXSDDateTime
 * value into <date>T<time> where <date> would be replaced by a
 * formatted date and <time> by a formatted time
 */
EXTXMLMETHOD int rtXmlEncDateTimeValue (OSCTXT* pctxt, const OSXSDDateTime* pvalue)
{
   int stat;

   stat = rtXmlEncDatePartValue (pctxt, pvalue);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   stat = rtXmlPutChar (pctxt, 'T');
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   stat = rtXmlEncTimeValue (pctxt, pvalue);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   LCHECKX (pctxt);

   return 0;
}



