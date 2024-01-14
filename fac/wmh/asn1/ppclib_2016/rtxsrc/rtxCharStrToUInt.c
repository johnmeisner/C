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

#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxErrCodes.h"

static const OSUINT32 maxUInt32 = 0xFFFFFFFFu;
static const OSUINT32 maxUInt32_10 = 0x19999999u; /* 0x100000000/10 */

EXTRTMETHOD int rtxCharStrToUInt (const char* cstr, OSUINT32* pvalue)
{
   size_t i = 0;
   register OSUINT32 value = 0, ndigits = 0;

   *pvalue = 0;

   /* Consume leading white space */

   while (OS_ISSPACE (cstr[i])) i++;

   /* Check for '+' first character */

   if (cstr[i] == '+') {
      i++;
   }

   /* Convert characters to an integer value */

   while (OS_ISDIGIT (cstr[i])) {
      OSUINT32 tm = cstr[i] - '0';

      if (value > maxUInt32_10) return RTERR_TOOBIG;

      value *= 10;

      if (value > maxUInt32 - tm)
         return RTERR_TOOBIG;

      value += tm; i++; ndigits++;
   }

   /* If there were no numeric digits in string, signal error */

   if (0 == ndigits) return RTERR_INVCHAR;

   *pvalue = value;

   return (0);
}

EXTRTMETHOD int rtxCharStrToUInt8 (const char* cstr, OSOCTET* pvalue)
{
   OSUINT32 itmp;
   int stat = rtxCharStrToUInt (cstr, &itmp);
   if (stat == 0) {
      if (itmp > 255)
         return RTERR_TOOBIG;
      *pvalue = (OSOCTET)itmp;
   }
   return stat;
}

EXTRTMETHOD int rtxCharStrToUInt16 (const char* cstr, OSUINT16* pvalue)
{
   OSUINT32 itmp;
   int stat = rtxCharStrToUInt (cstr, &itmp);
   if (stat == 0) {
      if (itmp > 65535)
         return RTERR_TOOBIG;
      *pvalue = (OSOCTET)itmp;
   }
   return stat;
}
