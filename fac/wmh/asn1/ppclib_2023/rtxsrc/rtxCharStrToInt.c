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

static const OSINT32 maxInt32 = 0x7FFFFFFF;
static const OSINT32 maxInt32_10 = 0xCCCCCCC; /* 0x80000000/10 */

EXTRTMETHOD int rtxCharStrnToInt
(const char* cstr, OSSIZE ndigits, OSINT32* pvalue)
{
   size_t i = 0, j;
   OSBOOL minus = FALSE;
   register OSINT32 value = 0;
   OSINT32 maxval = maxInt32, digitcnt = 0;

   *pvalue = 0;

   /* Consume leading white space */

   while (OS_ISSPACE (cstr[i])) i++;

   /* Check for '+' or '-' first character */

   if (cstr[i] == '+') {
      i++;
   }
   else if (cstr[i] == '-') {
      minus = TRUE;
      i++;
      maxval++; /* support for 0x80000000 */
   }

   /* Convert characters to an integer value */

   for (j = 0; j < ndigits; j++) {
      if (OS_ISDIGIT (cstr[i])) {
         OSINT32 tm = cstr[i] - '0';

         if (value > maxInt32_10) return RTERR_TOOBIG;

         value *= 10;

         if ((OSUINT32)value > (OSUINT32)(maxval - tm))
            return RTERR_TOOBIG;

         value += tm; i++; digitcnt++;
      }
      else break;
   }

   /* If there were no numeric digits in string , signal error */

   if (0 == digitcnt) return RTERR_INVCHAR;

   /* If negative, negate number */

   if (minus) *pvalue = 0 - value;
   else *pvalue = value;

   return (0);
}

EXTRTMETHOD int rtxCharStrToInt8 (const char* cstr, OSINT8* pvalue)
{
   OSINT32 itmp;
   int stat;

   stat = rtxCharStrToInt (cstr, &itmp);
   if (stat == 0) {
      if (itmp < -128 || itmp > 127)
         return RTERR_TOOBIG;

      *pvalue = (OSINT8)itmp;
   }

   return stat;
}

EXTRTMETHOD int rtxCharStrToInt16 (const char* cstr, OSINT16* pvalue)
{
   OSINT32 itmp;
   int stat;

   stat = rtxCharStrToInt (cstr, &itmp);
   if (stat == 0) {
      if (itmp < -32768 || itmp > 32767)
         return RTERR_TOOBIG;

      *pvalue = (OSINT8)itmp;
   }

   return stat;
}

EXTRTMETHOD int rtxCharStrToInt (const char* cstr, OSINT32* pvalue)
{
   return rtxCharStrnToInt (cstr, OSCRTLSTRLEN(cstr), pvalue);
}
