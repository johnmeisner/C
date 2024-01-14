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

#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxErrCodes.h"

EXTRTMETHOD int rtxCharStrToUInt64 (const char* cstr, OSUINT64* pvalue)
{
   OSSIZE i = 0, len;
   OSUINT64 value = 0, prevValue;

   if (0 == cstr || 0 == pvalue) return RTERR_INVPARAM;

   *pvalue = 0;

   len = OSCRTLSTRLEN (cstr);
   if (0 == len) return RTERR_INVLEN;

   /* Consume leading white space */

   while (OS_ISSPACE (cstr[i])) i++;

   /* Check for '+' first character */

   if (cstr[i] == '+') {
      /* Advance index and check for string with '+' sign only */
      if (++i >= len) return RTERR_INVCHAR;
   }

   /* Convert characters to an integer value */

   for ( ; i < len; i++) {
      OSUINT32 tm = cstr[i] - '0';

      if (!OS_ISDIGIT (cstr[i])) return RTERR_INVCHAR;

      prevValue = value;

      value = (value * 10) + tm;

      if (value < prevValue) return RTERR_TOOBIG;
   }

   *pvalue = value;

   return (0);
}
