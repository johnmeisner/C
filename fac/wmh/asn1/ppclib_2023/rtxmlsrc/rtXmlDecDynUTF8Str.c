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
#include "rtxmlsrc/rtSaxCParser.h"

/* This function decodes an XML string */

EXTXMLMETHOD int rtXmlDecXmlStr
   (OSCTXT* pctxt, OSXMLSTRING* outdata)
{
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   outdata->cdata = ctxtInfo->mbCdataProcessed;
   return rtXmlDecDynUTF8Str (pctxt, &outdata->value);
}

EXTXMLMETHOD int rtXmlDecUTF8Str (OSCTXT* pctxt, OSUTF8CHAR* outdata, size_t max_len)
{
   const OSUTF8CHAR* const inpdata = (const OSUTF8CHAR*) OSRTBUFPTR (pctxt);
   size_t i;
   const size_t nbytes =
      (OSRTBUFSIZE(pctxt) + 1 > max_len) ? max_len : OSRTBUFSIZE(pctxt) + 1;

   LCHECKX (pctxt);
   OSRT_CHECK_EVAL_DATE1(pctxt);

   for(i = 0; i < nbytes - 1; i++) {
      outdata[i] = inpdata[i];
   }
   outdata[i] = (OSUTF8CHAR)'\0';

   return 0;
}

/* This function decodes a dynamic UTF-8 string.  Conditional logic     */
/* is present to handle Unicode strings..                               */

EXTXMLMETHOD int rtXmlDecDynUTF8Str (OSCTXT* pctxt, const OSUTF8CHAR** outdata)
{
   const OSUTF8CHAR* const inpdata = (const OSUTF8CHAR*) OSRTBUFPTR (pctxt);
   size_t i;
   const size_t nbytes = OSRTBUFSIZE(pctxt);
   OSOCTET* str;

   LCHECKX (pctxt);
   OSRT_CHECK_EVAL_DATE1(pctxt);

   /* Allocate memory for output data */

   str = (OSOCTET*) rtxMemAlloc (pctxt, nbytes + 1);
   if (str != 0) {

      /* Copy UTF-8 source chars to destination */

      for (i = 0; i < nbytes; i++) {
         str[i] = inpdata[i];
      }
      str [i] = 0;
   }
   else
      return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

   *outdata = (const OSUTF8CHAR*) str;

   return (0);
}

