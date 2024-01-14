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

#include "rtxmlsrc/osrtxml.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxErrCodes.h"

static const OSUINT64 maxUInt64 = OSUI64CONST(0xFFFFFFFFFFFFFFFF);
static const OSUINT64 maxUInt64_10 = OSUI64CONST(0x1999999999999999);

EXTXMLMETHOD int rtXmlDecUInt64 (OSCTXT* pctxt, OSUINT64* pvalue)
{
   const OSUTF8CHAR* const inpdata = (const OSUTF8CHAR*) OSRTBUFPTR (pctxt);
   size_t i;
   size_t nbytes = OSRTBUFSIZE(pctxt);
   register OSUINT64 value = 0;

   *pvalue = 0;

   /* Consume leading white space */

   for (i = 0; i < nbytes; i++) {
      if (!OS_ISSPACE (inpdata[i])) break;
   }
   if (i == nbytes) return LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);

   /* Consume trailing white space */

   for (; nbytes > 0; nbytes--) {
      if (!OS_ISSPACE (inpdata[nbytes - 1])) break;
   }

   /* Check for '+' or '-' first character */

   if (inpdata[i] == '+') {
      i++;
      if (i >= nbytes) return LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
   }

   /* Convert Unicode characters to an integer value */

   for ( ; i < nbytes; i++) {
      if (OS_ISDIGIT (inpdata[i])) {
         OSUINT32 tm = inpdata[i] - '0';

         if (value > maxUInt64_10)
            return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);

         value *= 10;

         if (value > maxUInt64 - tm)
            return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);

         value += tm;
      }
      else
         return LOG_RTERRNEW (pctxt, RTERR_INVCHAR);
   }

   *pvalue = value;

   return (0);
}


