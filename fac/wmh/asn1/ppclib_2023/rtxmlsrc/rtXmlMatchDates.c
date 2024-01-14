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
#include <string.h>
#include "rtxmlsrc/osrtxml.h"
#include "rtxsrc/rtxErrCodes.h"

/**
 * rtXmlMatchGYear:This function matches a date value from a supplied context
 */
EXTXMLMETHOD int rtXmlMatchGYear (OSCTXT* pctxt)
{
   const OSUTF8CHAR* const inpdata = (const OSUTF8CHAR*) OSRTBUFPTR (pctxt);
   const size_t nbytes = OSRTBUFSIZE(pctxt);
   OSXSDDateTime value;
   int stat;

   stat = rtxParseGYearString (inpdata, nbytes, &value);
   return (stat < 0) ? stat: 0;
}

/**
 * rtXmlMatchGYearMonth:
 * This function matches a time value from a supplied context.
 */
EXTXMLMETHOD int rtXmlMatchGYearMonth (OSCTXT* pctxt)
{
   const OSUTF8CHAR* const inpdata = (const OSUTF8CHAR*) OSRTBUFPTR (pctxt);
   const size_t nbytes = OSRTBUFSIZE(pctxt);
   OSXSDDateTime value;
   int stat;

   stat = rtxParseGYearMonthString (inpdata, nbytes, &value);
   return (stat < 0) ? stat: 0;
}

/**
 * rtXmlMatchGMonth:
 * This function matches a datetime value from a supplied context.
 */
EXTXMLMETHOD int rtXmlMatchGMonth (OSCTXT* pctxt)
{
   const OSUTF8CHAR* const inpdata = (const OSUTF8CHAR*) OSRTBUFPTR (pctxt);
   const size_t nbytes = OSRTBUFSIZE(pctxt);
   OSXSDDateTime value;
   int stat;

   stat = rtxParseGMonthString (inpdata, nbytes, &value);
   return (stat < 0) ? stat: 0;
}

/**
 * rtXmlMatchGMonthDay:
 * This function matches a datetime value from a supplied context.
 */
EXTXMLMETHOD int rtXmlMatchGMonthDay (OSCTXT* pctxt)
{
   const OSUTF8CHAR* const inpdata = (const OSUTF8CHAR*) OSRTBUFPTR (pctxt);
   const size_t nbytes = OSRTBUFSIZE(pctxt);
   OSXSDDateTime value;
   int stat;

   stat = rtxParseGMonthDayString (inpdata, nbytes, &value);
   return (stat < 0) ? stat: 0;
}

/**
 * rtXmlMatchGDay:
 * This function matches a datetime value from a supplied context.
 */
EXTXMLMETHOD int rtXmlMatchGDay (OSCTXT* pctxt)
{
   const OSUTF8CHAR* const inpdata = (const OSUTF8CHAR*) OSRTBUFPTR (pctxt);
   const size_t nbytes = OSRTBUFSIZE(pctxt);
   OSXSDDateTime value;
   int stat;

   stat = rtxParseGDayString (inpdata, nbytes, &value);
   return (stat < 0) ? stat: 0;
}


