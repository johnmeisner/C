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
#include <string.h>
#include "rtxmlsrc/osrtxml.h"
#include "rtxsrc/rtxErrCodes.h"

/**
 * rtXmlDecDate:This function decodes a date value from a supplied context
  and set the pointed OSXSDDateTime to the decoded date value.
 */
EXTXMLMETHOD int rtXmlDecDate (OSCTXT* pctxt, OSXSDDateTime* pvalue)
{
   const OSUTF8CHAR* const inpdata = (const OSUTF8CHAR*) OSRTBUFPTR (pctxt);
   const size_t nbytes = OSRTBUFSIZE(pctxt);
   int stat;

   stat = rtxParseDateString (inpdata, nbytes, pvalue);
   return (stat < 0) ? LOG_RTERRNEW (pctxt, stat) : 0;
}

/**
 * rtXmlDecTime:
 * This function decodes a time value from a supplied context and set the
 * pointed OSXSDDateTime structure to the decoded time value.
 */
EXTXMLMETHOD int rtXmlDecTime (OSCTXT* pctxt, OSXSDDateTime* pvalue)
{
   const OSUTF8CHAR* const inpdata = (const OSUTF8CHAR*) OSRTBUFPTR (pctxt);
   const size_t nbytes = OSRTBUFSIZE(pctxt);
   int stat;

   stat = rtxParseTimeString (inpdata, nbytes, pvalue);
   return (stat < 0) ? LOG_RTERRNEW (pctxt, stat) : 0;
}

/**
 * rtXmlDecDateTime:
 * This function decodes a datetime value from a supplied context and set the
 * pointed OSXSDDateTime to the decoded date and time value.
 */
EXTXMLMETHOD int rtXmlDecDateTime (OSCTXT* pctxt, OSXSDDateTime* pvalue)
{
   const OSUTF8CHAR* const inpdata = (const OSUTF8CHAR*) OSRTBUFPTR (pctxt);
   const size_t nbytes = OSRTBUFSIZE(pctxt);
   int stat;

   stat = rtxParseDateTimeString (inpdata, nbytes, pvalue);
   return (stat < 0) ? LOG_RTERRNEW (pctxt, stat): 0;
}


