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

EXTXMLMETHOD int rtAsn1XmlpDecGenTime
(OSCTXT* pctxt, const char** outdata)
{
   const OSUTF8CHAR* strxmlvalue = 0;
   OSNumDateTime dateTime;
   int stat;

   stat = rtXmlpDecDynUTF8Str(pctxt, &strxmlvalue);

   if ( stat == 0 )
      stat = rtxParseDateTimeString(strxmlvalue, OSUTF8LEN(strxmlvalue),
         &dateTime);

   if ( stat == 0 ) {
      /* before casting outdata to non-const, assign it null pointer */
      /* just to guarantee that rtMakeGeneralizedTime does not modify the */
      /* contents of whatever string *outdata may point at. */
      *outdata = 0;
      stat = rtMakeGeneralizedTime(pctxt, &dateTime, (char**) outdata, 0);
   }

   if ( strxmlvalue != 0 ) rtxMemFreePtr(pctxt, strxmlvalue);

   if ( stat != 0 )
      return LOG_RTERR(pctxt, stat);

   return 0;
}
