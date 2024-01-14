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
#include "rtxmlsrc/osrtxml.h"

/* Decode XSI namespace attributes */

EXTXMLMETHOD int rtXmlDecXSIAttrs (OSCTXT* pctxt,
   const OSUTF8CHAR* const* attrs,
   const char* typeName)
{
   int stat = 0, i = 0, rstat = 0;

   if (attrs != 0) {
      while (0 != attrs[i]) {
         stat = rtXmlDecXSIAttr (pctxt, attrs[i], attrs[i + 1]);
         if (stat < 0) {
            rstat = RTERR_INVATTR;

            if (LOG_SAXERRNEW2(pctxt, rstat,
               rtxErrAddStrParm (pctxt, typeName),
               rtxErrAddStrParm (pctxt, (const char*)attrs[i])) != stat)
               break;
         }

         i += 2;
      }
   }
   return rstat;
}

